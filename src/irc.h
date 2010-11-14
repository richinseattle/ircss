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

// counter for number of bots that have connected
extern int bot_fd;

/*
 * IRC user, used to keep track of registered users on the irc server.
 */
typedef struct user {char *nick; char *user; char *real; char *host; int reg; int sockfd;} user_t;

/*
 * SS bot, used to keep track of connected bots.
 */
typedef struct bot {int sockfd;} bot_t;

/*
 * Registers newly-connected users via NICK and USER commands as specified in
 * RFC 1459.
 */
void reg_conn(int cli_sockfd, user_t *user);

/*
 * Parses messages from an irc user.
 */
void cli_read(int cli_sockfd);

/*
 * Sends a message to an irc user.
 */
void cli_write(int cli_sockfd, char *msg);

/*
 * Starts a thread to listen for user input on given sockfd.
 */
void *run_irc_cli(void *ptr);

/*
 * Starts a listening irc server on the given port.
 */
void *run_irc_srv(void *ptr);

#endif

