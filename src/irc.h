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

#ifndef _IRC_H_
#define _IRC_H_

#include <sys/socket.h>
#include "ss.h"

//typedef struct user {char *nick; char *user; char *real; char *host; int reg;} user_t;

void *get_in_addr(struct sockaddr *sa);

void sigchld_handler(int s);

void error(char *msg);

void reg_conn(int cli_sockfd, user_t *user);

int init_srv(int port);

int init_cli(int srv_sockfd);

void *cli_read(void *ptr);

int cli_write(settings_t *cli_sett, char *msg);

void run_cli(int cli_sockfd);

int irc(int);

#endif

