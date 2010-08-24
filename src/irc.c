/*
 * Copyright (c) 2010 John Gordon <jgor@indiecom.org> and Doug Farre <dougfarre@gmail.com>
 * * This program is free software: you can redistribute it and/or modify
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
#include <pthread.h>
#include "irc.h"
#include "ss.h"

/* 1 enables debug messages, 0 disables */
#define DEBUG 1

/* the channel users will join on connect (include the # or &) */
#define CHANNEL "#ircss"

/* max connections the irc daemon will accept */
#define MAX_CONNS 10

/* max message buffer size */
#define MAX_BUF 255

/* max lengths for client nick, username, real name, hostname */
#define MAX_NICK 9
#define MAX_USER 9
#define MAX_REAL 25
#define MAX_HOST 255

/* server response message codes, defined in RFC 1459 */
#define RPL_MOTDSTART 375
#define RPL_MOTD 372
#define RPL_ENDOFMOTD 376

/*
 * name: get_in_addr
 * return: ipv4 or ipv6 sockaddr
 * description: returns the appropriate protocol-specific sockaddr (ipv4/ipv6)
 *   given a generic sockaddr.
 */
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * name: sigchld_handler
 * return: none
 * description: child process handler, used to avoid zombie child processes.
 */
void sigchld_handler(int s) {
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*
 * name: error
 * return: none
 * description: displays the error message via perror then exits with failure
 *   code.
 */
void error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/*
 * name: reg_conn
 * return: none
 * description: registers newly-connected clients via NICK and USER commands
 *   as specified in RFC 1459.
 */
void reg_conn(int cli_sockfd, user_t *user) {
  if (DEBUG) fprintf(stderr, "DEBUG: reg_conn() entered.\n");
  char buf[MAX_BUF + 1], *line, *tok;
  int err, i;

  while (!(user->reg)) {
    memset(&buf, 0, sizeof(buf));
    err = read(cli_sockfd, buf, MAX_BUF);
    if (err == -1) error("ERROR on read");

    tok = strtok(buf, " ,");

    while (tok != NULL) {
      if (strcmp(tok, "NICK") == 0) {
        tok = strtok(NULL, "\n");
        strncpy(user->nick, tok, MAX_NICK);
        if (user->nick[strlen(user->nick) - 1] == '\r')
          user->nick[strlen(user->nick) - 1] = '\0';
        if (DEBUG) fprintf(stderr, "DEBUG: user->nick = %s\n", user->nick);
      }

      else if (strcmp(tok, "USER") == 0) {
        tok = strtok(NULL, " ,");
        strncpy(user->user, tok, MAX_USER);
        if (DEBUG) fprintf(stderr, "DEBUG: user->user = %s\n", user->user);
        tok = strtok(NULL, " ,");
        tok = strtok(NULL, " ,");
        tok = strtok(NULL, "\n");
        if (tok[0] == ':') tok++;
        strncpy(user->real, tok, MAX_REAL);
        if (user->real[strlen(user->real) - 1] == '\r')
          user->real[strlen(user->real) - 1] = '\0';
        if (DEBUG) fprintf(stderr, "DEBUG: user->real = %s\n", user->real);
      }

      tok = strtok(NULL, " ,");
    }

    if (strlen(user->nick) && strlen(user->user) && strlen(user->real))
      user->reg = 1;
  }

  if (DEBUG) fprintf(stderr, "DEBUG: registration successful.\n");

  char *server_name = "ircss";
  char preamble[4][MAX_BUF + 1];
  snprintf(preamble[0], MAX_BUF, ":%s %d %s :- %s Message of the Day -\n", server_name, RPL_MOTDSTART, user->nick, server_name);
  snprintf(preamble[1], MAX_BUF, ":%s %d %s :- Welcome to ircss. Enjoy!\n", server_name, RPL_MOTD, user->nick);
  snprintf(preamble[2], MAX_BUF, ":%s %d %s :End of /MOTD command.\n", server_name, RPL_ENDOFMOTD, user->nick);
  snprintf(preamble[3], MAX_BUF, ":%s!~%s@%s JOIN :%s\n", user->nick, user->user, user->host, CHANNEL);

  for (i = 0; i < sizeof(preamble) / MAX_BUF; i++) {
    err = write(cli_sockfd, preamble[i], strlen(preamble[i]));
    if (err == -1) error("ERROR on write");
  }
}

/*
 * name: init_srv
 * return: listening server socket file descriptor
 * description: establishes a listening socket on the specified port.
 */
int init_srv(int port) {
  int srv_sockfd, err;
  struct addrinfo hints, *res, *res0;
  struct sigaction sa;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  char port_str[6];
  if (port < 1 || port > 65535) fprintf(stderr, "Invalid port\n");
  sprintf(port_str, "%d", port);
  err = getaddrinfo(NULL, port_str, &hints, &res0);
  if (err) {
    fprintf(stderr, "ERROR on getaddrinfo: %s\n", gai_strerror(err));
    exit(EXIT_FAILURE);
  }

  for (res = res0; res != NULL; res = res->ai_next) {
    srv_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (srv_sockfd == -1) continue;

    int reuse = 1;
    err = setsockopt(srv_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if (err == -1) error("ERROR on setsockopt");

    err = bind(srv_sockfd, res->ai_addr, res->ai_addrlen);
    if (err == -1) continue;

    break;
  }

  if (res == NULL) {
    fprintf(stderr, "ERROR binding\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(res0);

  err = listen(srv_sockfd, MAX_CONNS);
  if (err == -1) error("ERROR on listen");

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  err = sigaction(SIGCHLD, &sa, NULL);
  if (err == -1) error("ERROR on sigaction");

  return srv_sockfd;
}

/*
 * name: init_cli
 * return: server<->client socket file descriptor
 * description: creates a socket for incoming client connections.
 */
int init_cli(int srv_sockfd) {
  int cli_sockfd, err;
  struct sockaddr_storage cli_addr;
  int cli_len = sizeof(cli_addr);

  cli_sockfd = accept(srv_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
  if (cli_sockfd == -1) error("ERROR on accept");

  return cli_sockfd;
}

/*
 * name: cli_read
 * return: void pointer (see pthread docs)
 * description: thread to handle reads from the client, parses all user input.
 *   cli_read and cli_write are asynchronous i/o via the client socket.
 */
void *cli_read(void *ptr) {
  int cli_sockfd = *((int *) ptr);
  int err;
  char buf[MAX_BUF + 1];

  while (1) {
    memset(&buf, 0, sizeof(buf));
    err = read(cli_sockfd, buf, MAX_BUF);
    if (err == -1) error("ERROR on read");
    else if (err == 0) break;
    char *tok;

    tok = strtok(buf, " ");

    if (strcmp(tok, "PRIVMSG") == 0) {
      tok = strtok(NULL, " ");
      if (strcmp(tok, CHANNEL) == 0) {
        tok = strtok(NULL, " ");
        if (tok[0] == ':') tok++;
        if (tok[0] == '.') {
          tok++;
          char cmd[strlen(tok)];
          sscanf(tok, "%s", &cmd);

          if (strcmp(cmd, "exit") == 0) {
            close(cli_sockfd);
            if (DEBUG) fprintf(stderr, "DEBUG: client exited.\n");
            exit(EXIT_SUCCESS);
          }
          else {
            if (DEBUG) fprintf(stderr, "DEBUG: unkown command: %s\n", cmd);  
            continue;
          }
        }
        //else ss_send(tok);
      }
    }
  }
}

/*
 * name: cli_write
 * return: void pointer (see pthread docs)
 * description: thread to handle writes to the client, sends messages to the
 *   user. cli_read and cli_write are asynchronous i/o via the client socket.
 */
void *cli_write(void *ptr) {
  int cli_sockfd = *((int *) ptr);

  /* TODO: messages to be sent to the client, perhaps from a socket in the
           spread-spectrum code returned by ss_recv() */
}

/*
 * name: run_cli
 * return: none
 * description: the main execution point for new client connections.
 */
void run_cli(int cli_sockfd) {
  pthread_t pt_read, pt_write;
  user_t user;
  char s[INET6_ADDRSTRLEN];
  char host[MAX_HOST + 1];
  struct sockaddr_storage cli_addr;
  int cli_len = sizeof(cli_addr);

  user.reg = 0;
  user.nick = calloc(MAX_NICK + 1, sizeof(char));
  user.user = calloc(MAX_USER + 1, sizeof(char));
  user.real = calloc(MAX_REAL + 1, sizeof(char));
  user.host = calloc(MAX_HOST + 1, sizeof(char));

  getpeername(cli_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
  inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *)&cli_addr), s, sizeof(s));
  getnameinfo((struct sockaddr *)&cli_addr, sizeof(cli_addr), host, sizeof(host), NULL, 0, 0);
  strncpy(user.host, host, MAX_HOST);
  if (DEBUG) fprintf(stderr, "DEBUG: user.host = %s\n", user.host);

  reg_conn(cli_sockfd, &user);

  pthread_create(&pt_read, NULL, cli_read, (void *)&cli_sockfd);
  pthread_create(&pt_write, NULL, cli_write, (void *)&cli_sockfd);

  pthread_join(pt_read, NULL);
  pthread_join(pt_write, NULL);
}

/*
 * name: irc
 * return: 0 on success
 * description: the entry point for the listening irc daemon.
 */
int irc(int port) {
  int srv_sockfd, cli_sockfd;

  srv_sockfd = init_srv(port);

  while (1) {
    cli_sockfd = init_cli(srv_sockfd);

    if (!fork()) {
      close(srv_sockfd);

      run_cli(cli_sockfd);

      close(cli_sockfd);
      exit(EXIT_SUCCESS);
    }

    close(cli_sockfd);
  }

  return 0;
}

