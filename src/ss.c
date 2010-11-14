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
#include <string.h>
#include <search.h>
#include "core.h"
#include "sock.h"
#include "irc.h"
#include "ss.h"

// counter for number of bots that have connected
int bot_fd = 0;

/*
 * Parses messages from a bot.
 */
void ss_read(int cli_sockfd) {
    int err, i;
    char buf[MAX_BUF], cmd[MAX_BUF], args[MAX_BUF], msg[MAX_BUF];

    /* listen indefinitely for incoming messages */
    while (1) {
        /* attempt to read the next incoming message */
        memset(&buf, 0, sizeof(buf));
        err = read(cli_sockfd, buf, MAX_BUF);
        if (err == -1) error("read failed.");
        /* bot has disconnected */
        else if (err == 0) break;

        /* parse message as a single command followed by arguments */
        sscanf(buf, "%s %[^\n]", cmd, args);

        debug("cmd = %s, args = %s\n", cmd, args);

        /* MSG, a string from the bot to be displayed to the irc user */
        if (strcmp(cmd, "MSG") == 0) {
            debug("got MSG\n");

            /* add a newline to the message */
            snprintf(msg, MAX_BUF, "%s\n", args);

            /* iterate through all connected irc users */
            for (i = 1; i <= user_fd; i++) {
                char search[MAX_BUF];
                ENTRY search_item;
                ENTRY *result_item;

                /* key for connection hashtable, format is 'userN' for N = i */
                snprintf(search, MAX_BUF, "user%d", i);

                search_item.key = search;

                debug("searching for %s\n", search_item.key);

                /* search the connection hashtable for the i'th irc user,
                   return user_t struct for the user containing its socket */ 
                result_item = hsearch(search_item, FIND);

                /* send the MSG string from the bot to the i'th irc user */
                if (result_item != NULL)
                    cli_write(((user_t *)result_item->data)->sockfd, msg);
                else debug("socket not found.\n");
            }
        }
    }
}

/*
 * Sends a message to a bot.
 */
void ss_write(int cli_sockfd, char *msg) {
    int err;

    debug("ss_write() entered, cli_sockfd = %d\n", cli_sockfd);
    debug("msg = %s\n", msg);

    /* send given string to specified socket */
    err = write(cli_sockfd, msg, strlen(msg));
    if (err == -1) error("write failed.");
}

/*
 * Starts a thread to listen for bot input on given sockfd.
 */
void *run_ss_cli(void *ptr) {
    int *cli_sockfd_ptr = (int *) ptr;
    int cli_sockfd = *cli_sockfd_ptr;

    /* start reading messages from the bot on the specified socket */
    ss_read(cli_sockfd);
}

/*
 * Starts a listening bot server on the given port.
 */
void *run_ss_srv(void *ptr) {
    int *port_ptr = (int *) ptr;
    int port = *port_ptr;
    int srv_sockfd, cli_sockfd;
    pthread_t pt_read;

    /* establish a listening socket  on given port */
    srv_sockfd = get_srv_sock(port);

    /* listen indefinitely for new bots to connect */
    while (1) {
        /* listen for and accept a new connection from a bot */
        cli_sockfd = get_cli_sock(srv_sockfd);
        bot_fd++;

        char key[MAX_BUF];
        bot_t data;
        ENTRY item;

        /* construct connection hashtable key for this bot, botN */
        snprintf(key, MAX_BUF, "bot%d", bot_fd);

        /* construct connection hashtable value, a bot_t containing the sock */
        data.sockfd = cli_sockfd;

        item.key = key;
        item.data = &data;

        debug("entering %s -> sockfd=%d\n", item.key, ((bot_t *)item.data)->sockfd);

        /* enter the bot's key/value into the connection hashtable */
        hsearch(item, ENTER);

        /* start a thread to interact with the bot */
        pthread_create(&pt_read, NULL, run_ss_cli, (void *) &cli_sockfd);
    }
}

