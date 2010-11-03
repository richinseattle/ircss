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

int bot_fd = 0;

/*
 * Parses messages from the client.
 */
void ss_read(int cli_sockfd) {
  int err, i;
  char buf[MAX_BUF + 1];

  while (1) {
    memset(&buf, 0, sizeof(buf));
    err = read(cli_sockfd, buf, MAX_BUF);
    if (err == -1) error("ERROR on read");
    else if (err == 0) break;
    char *tok;

    tok = strtok(buf, " ");

    if (strcmp(tok, "MSG") == 0) {
      tok = strtok(NULL, "\r\n");
      char *msg = calloc(strlen(tok) + 2, sizeof(char));
      strcpy(msg, tok);
      strcat(msg, "\n");

      for (i = 1; i <= user_fd; i++) {
        char search[10] = "user";
        search[4] = i + 48;
        search[5] = '\0';
        char *search_ptr = search;
        bot_t *result_ptr;
        ENTRY search_item;
        ENTRY *result_item;
        search_item.key = search_ptr;
        result_item = hsearch(search_item, FIND);
        if (result_item != NULL)
          cli_write(((user_t *)result_item->data)->sockfd, msg);
        else
          if (DEBUG)fprintf(stderr, "DEBUG: no results found\n");
      }
    }
  }
}

/*
 * Sends a message to all users.
 */
void ss_write(int cli_sockfd, char *msg) {
  int err;

  err = write(cli_sockfd, msg, strlen(msg));
  if (err == -1) error("ERROR on write");
}

/*
 * Starts a thread to listen for bot input on given sockfd.
 */
void *run_ss_cli(void *ptr) {
  int *cli_sockfd_ptr = (int *) ptr;
  int cli_sockfd = *cli_sockfd_ptr;

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

  srv_sockfd = get_srv_sock(port);

  while (1) {
    cli_sockfd = get_cli_sock(srv_sockfd);
    bot_fd++;

    char *key_ptr = calloc(10, sizeof(char));
    sprintf(key_ptr, "bot%d", bot_fd);
    bot_t *data_ptr = calloc(1, sizeof(bot_t));
    data_ptr->sockfd = cli_sockfd;
    ENTRY item;
    item.key = key_ptr;
    item.data = data_ptr;

    hsearch(item, ENTER);

    pthread_create(&pt_read, NULL, run_ss_cli, (void *) &cli_sockfd);
  }
}

