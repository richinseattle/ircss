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

#ifndef _SS_H_
#define _SS_H_

// counter for the number of users that have connected
extern int user_fd;

/*
 * Parses messages from a bot.
 */
void ss_read(int cli_sockfd);

/*
 * Sends a message to a bot.
 */
void ss_write(int cli_sockfd, char *msg);

/*
 * Starts a thread to listen for bot input on given sockfd.
 */
void *run_ss_cli(void *ptr);

/*
 * Starts a listening bot server on the given port.
 */
void *run_ss_srv(void *ptr);

#endif

