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

#ifndef _SOCK_H_
#define _SOCK_H_

#include <sys/socket.h>

/*
 * Returns the appropriate protocol-specific sockaddr (ipv4/ipv6) given a
 * generic sockaddr.
 */
void *get_in_addr(struct sockaddr *sa);

/*
 * Child process handler, used to avoid zombie child processes.
 */
void sigchld_handler(int s);

/*
 * Establishes a listening socket on the specified port and returns the sockfd.
 */
int get_srv_sock(int port);

/*
 * Creates a socket for an incoming client connection and returns the sockfd.
 */
int get_cli_sock(int srv_sockfd);

/*
 * Connects to a remote socket and returns the sockfd.
 */
int get_conn_sock(char *addr, int port);

#endif

