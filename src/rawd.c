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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "sock.h"

/* 1 enables debug messages, 0 disables */
#define DEBUG 1

/* max connections the irc daemon will accept */
#define MAX_CONNS 10

/* max message buffer size */
#define MAX_BUF 255

/*
 * name: raw
 * return: 0 on success
 * description: the entry point for the listening raw daemon.
 */
int raw(int port) {
  int srv_sockfd, cli_sockfd, err;
  char buf[MAX_BUF];
  fd_set master;
  fd_set read_fds;
  int fdmax;
  char remoteIP[INET6_ADDRSTRLEN];
  int i, j, nbytes;

  srv_sockfd = get_srv_sock(port);

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  FD_SET(srv_sockfd, &master);

  fdmax = srv_sockfd;

  while(1) {
    read_fds = master;
    err = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
    if (err == -1) error("ERROR on select");

    for (i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
        if (i == srv_sockfd) {
          cli_sockfd = get_cli_sock(srv_sockfd);
          FD_SET(cli_sockfd, &master);
          if (cli_sockfd > fdmax) fdmax = cli_sockfd;
        }
        else {
          nbytes = read(i, buf, sizeof(buf));
          if (nbytes == -1) error("ERROR on read");
          else if (nbytes == 0) {
            close(i);
            FD_CLR(i, &master);
          }
          else {
            for (j = 0; j <= fdmax; j++) {
              if (FD_ISSET(j, &master)) {
                if (j != srv_sockfd && j != i) {
                  err = write(j, buf, nbytes);
                  if (err == -1) error("ERROR on write");
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}

int main() {

  raw(6601);

  return EXIT_SUCCESS;
}

