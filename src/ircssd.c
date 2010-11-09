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
#include "core.h"
#include "irc.h"
#include "ss.h"

void print_version() {
  printf("ircssd v%s\n", VERSION);
  exit(EXIT_SUCCESS);
}

void print_help() {
  printf("Usage: ircssd -[hv] [-c FILE]\n");
  printf("\n");
  printf("Options:\n");
  printf("  -c, --config=FILE config file to use\n");
  printf("  -h, --help        display this screen\n");
  printf("  -v, --version     display version\n");
  exit(EXIT_SUCCESS);
}

static struct option long_options[] = {
  {"config",  1, 0, 'c'},
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {0, 0, 0, 0}
};

int main(int argc, char **argv) {
  int c;
  FILE *fp = NULL;
  char *filename, str[LINE_MAX] = "", *ret;
  extern char *optarg;
  extern int optind;
  pthread_t pt_irc, pt_ss;
  int irc_port, ss_port;
  
  while (1) {
    int option_index = 0;

    c = getopt_long_only(argc, argv, "c:hv", long_options, &option_index);

    if (c == -1)
      break;

    switch(c) {
      case 'c':
        filename = optarg;
        fp = fopen(filename, "r");
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

  hcreate(MAX_HTAB);

  irc_port = 6666;
  ss_port = 7666;

  pthread_create(&pt_irc, NULL, run_irc_srv, (void *) &irc_port);
  pthread_create(&pt_ss, NULL, run_ss_srv, (void *) &ss_port);

  pthread_join(pt_irc, NULL);
  pthread_join(pt_ss, NULL);

  exit(EXIT_SUCCESS);
}
