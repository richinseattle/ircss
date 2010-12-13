/*
 * Copyright (c) 2010 John Gordon <jgor@indiecom.org>
 *   and Doug Farre <dougfarre@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/in.h>
#include "core.h"
#include "sock.h"

/*
 * Returns the appropriate protocol-specific sockaddr (ipv4/ipv6) given a
 * generic sockaddr.
 */
void *get_in_addr(struct sockaddr *sa) {
    /* IPv4 */
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

    /* IPv6 */
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * Establishes a listening socket on the specified port and returns the sockfd.
 */
int get_srv_sock(int port, int family) {
    int srv_sockfd, err;
    struct addrinfo hints, *res, *res0;
    struct sigaction sa;

    /* populate hints as a family-inspecific (ipv4/ipv6) stream socket (tcp)
       suitable for binding on a listening port (AI_PASSIVE is set) */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* validate port then store as string */
    char port_str[MAX_BUF];
    if (port < 1 || port > 65535) error("invalid port");
    snprintf(port_str, MAX_BUF, "%d", port);

    /* populate the addrinfo struct res0 based on hints and port */
    err = getaddrinfo(NULL, port_str, &hints, &res0);
    if (err) error("getaddrinfo failed: %s\n", gai_strerror(err));

    /* iterate through all available addresses to listen on */
    for (res = res0; res != NULL; res = res->ai_next) {
        /* create the socket, on failure go to the next available address */
        srv_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (srv_sockfd == -1) continue;

        /* set socket as reusable to allow multiple simultaneous connections */
        int reuse = 1;
        err = setsockopt(srv_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        if (err == -1) error("setsockopt failed");

#ifdef IPV6_V6ONLY
        /* if socket is ipv6, disallow ipv4 connections */
        if (res->ai_family == AF_INET6) {
            int v6only = 1;
            err = setsockopt(srv_sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
            if (err == -1) error("setsockopt failed");
        }
#endif

        /* bind the socket to its address/port, on failure go to next addr */
        err = bind(srv_sockfd, res->ai_addr, res->ai_addrlen);
        if (err == -1) continue;

        /* assume the socket is valid at this point, so break and move on */
        break;
    }

    /* if all available addresses have been exhausted, fail */
    if (res == NULL) error("bind failed");

    /* done with the list of available addresses, free the memory */
    freeaddrinfo(res0);

    /* start listening for incoming connections on the newly built socket */
    err = listen(srv_sockfd, MAX_CONNS);
    if (err == -1) error("listen failed");

    /* return the listening socket */
    return srv_sockfd;
}

/*
 * Creates a socket for an incoming client connection and returns the sockfd.
 */
int get_cli_sock(int srv_sockfd) {
    int cli_sockfd, err;
    struct sockaddr_storage cli_addr;
    int cli_len = sizeof(cli_addr);

    /* accept a new incoming connection on the listening srv_sockfd */
    cli_sockfd = accept(srv_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (cli_sockfd == -1) error("accept failed");

    /* return the socket between the listening server and new client */
    return cli_sockfd;
}

/*
 * Connects to a remote socket and returns the sockfd.
 */
int get_conn_sock(char *addr, int port) {
    int conn_sockfd, err;
    struct addrinfo hints, *res, *res0;

    /* populate hints as a family-inspecific (ipv4/ipv6) stream socket (tcp)
       suitable for connecting to a given address  (AI_PASSIVE not set) */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* validate port then store as string */
    char port_str[MAX_BUF];
    if (port < 1 || port > 65535) error("invalid port");
    snprintf(port_str, MAX_BUF, "%d", port);

    /* populate the addrinfo struct res0 based on hints, addr and port */
    err = getaddrinfo(addr, port_str, &hints, &res0);
    if (err) error("getaddrinfo failed: %s\n", gai_strerror(err));

    /* iterate through all available addresses to connect to */
    for (res = res0; res != NULL; res = res->ai_next) {
        /* create the socket, on failure go to the next available address */
        conn_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (conn_sockfd == -1) continue;

        /* establish a connection to the remote address */
        err = connect(conn_sockfd, res->ai_addr, res->ai_addrlen);
        if (err == -1) continue;

        /* assume the socket is valid at this point, so break and move on */
        break;
    }

    /* if all available addresses have been exhausted, fail */
    if (res == NULL) error("connect failed");

    /* done with the list of available addresses, free the memory */
    freeaddrinfo(res0);

    /* return the connected socket */
    return conn_sockfd;
}

