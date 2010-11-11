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
#include "core.h"
#include "sock.h"

/*
 * Returns the appropriate protocol-specific sockaddr (ipv4/ipv6) given a
 * generic sockaddr.
 */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * Child process handler, used to avoid zombie child processes.
 */
void sigchld_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*
 * Establishes a listening socket on the specified port and returns the sockfd.
 */
int get_srv_sock(int port) {
    int srv_sockfd, err;
    struct addrinfo hints, *res, *res0;
    struct sigaction sa;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char port_str[6];
    if (port < 1 || port > 65535) error("invalid port.");
    sprintf(port_str, "%d", port);
    err = getaddrinfo(NULL, port_str, &hints, &res0);
    if (err) error("getaddrinfo failed: %s\n", gai_strerror(err));

    for (res = res0; res != NULL; res = res->ai_next) {
        srv_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (srv_sockfd == -1) continue;

        int reuse = 1;
        err = setsockopt(srv_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
        if (err == -1) error("setsockopt failed.");

        err = bind(srv_sockfd, res->ai_addr, res->ai_addrlen);
        if (err == -1) continue;

        break;
    }

    if (res == NULL) error("bind failed");

    freeaddrinfo(res0);

    err = listen(srv_sockfd, MAX_CONNS);
    if (err == -1) error("listen failed.");

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    err = sigaction(SIGCHLD, &sa, NULL);
    if (err == -1) error("sigaction failed");

    return srv_sockfd;
}

/*
 * Creates a socket for an incoming client connection and returns the sockfd.
 */
int get_cli_sock(int srv_sockfd) {
    int cli_sockfd, err;
    struct sockaddr_storage cli_addr;
    int cli_len = sizeof(cli_addr);

    cli_sockfd = accept(srv_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (cli_sockfd == -1) error("accept failed.");

    return cli_sockfd;
}

/*
 * Connects to a remote socket and returns the sockfd.
 */
int get_conn_sock(char *addr, int port) {
    int conn_sockfd, err;
    struct addrinfo hints, *res, *res0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char port_str[6];
    if (port < 1 || port > 65535) error("invalid port.");
    sprintf(port_str, "%d", port);
    err = getaddrinfo(NULL, port_str, &hints, &res0);
    if (err) error("getaddrinfo failed: %s\n", gai_strerror(err));

    for (res = res0; res != NULL; res = res->ai_next) {
        conn_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (conn_sockfd == -1) continue;

        err = connect(conn_sockfd, res->ai_addr, res->ai_addrlen);
        if (err == -1) continue;

        break;
    }

    if (res == NULL) error("connect failed.");

    freeaddrinfo(res0);

    return conn_sockfd;
}

