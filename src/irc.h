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

/* 1 enables debug messages, 0 disables */
#define DEBUG 1

/* Bot info */
#define BOT_NICK "ircss"
#define BOT_USER "ircss"
#define BOT_HOST "localhost"

/* Channel info */
#define SERVER "ircss"
#define CHANNEL "#ircss"
#define TOPIC "IRCSS"

/* Maximum message buffer size in bytes */
#define MAX_BUF 255

/* Maximum lengths for client nick, username, real name, hostname */
#define MAX_NICK 9
#define MAX_USER 9
#define MAX_REAL 25
#define MAX_HOST 255

/* Server response message codes, as defined in RFC 1459 */
#define RPL_TOPIC 332
#define RPL_NAMREPLY 353
#define RPL_MOTD 372
#define RPL_MOTDSTART 375
#define RPL_ENDOFMOTD 376

/*
 * IRC user, used to keep track of registered users on the irc server.
 */
typedef struct user {char *nick; char *user; char *real; char *host; int reg;} user_t;

/*
 * Registers newly-connected clients via NICK and USER commands as specified
 * in RFC 1459.
 */
void reg_conn(int cli_sockfd, user_t *user);

/*
 * Pthread to handle reading messages from the client, parses all user input.
 */
void *cli_read(void *ptr);

/*
 * Sends a message to the client.
 */
void cli_write(int cli_sockfd, char *msg);

/*
 * Starts a thread to listen for user input on given sockfd.
 */
void run_irc_cli(int cli_sockfd);

/*
 * Starts a listening irc server on the given port.
 */
int run_irc_srv(int port);

#endif

