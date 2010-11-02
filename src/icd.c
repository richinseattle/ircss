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

  exit(EXIT_SUCCESS);
}

