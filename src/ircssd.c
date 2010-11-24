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
#include <pthread.h>
#include "core.h"
#include "irc.h"
#include "ss.h"

void print_version() {
    printf("ircssd v%s\n", VERSION);
    exit(EXIT_SUCCESS);
}

void print_help() {
    printf("Usage: ircssd -[hv] -p PORT -s PORT\n");
    printf("\n");
    printf("Options:\n");
    printf("  -p, --irc-port=PORT  irc server port to listen on\n");
    printf("  -s, --ss-port=PORT   ss (bot) server port to listen on\n");
    printf("  -h, --help           display this screen\n");
    printf("  -v, --version        display version\n");
    exit(EXIT_SUCCESS);
}

/* available command line arguments: short/long form and number of args */
static struct option long_options[] = {
    {"irc-port", 0, 0, 'p'},
    {"ss-port",  0, 0, 's'},
    {"help",     0, 0, 'h'},
    {"version",  0, 0, 'v'},
    {0, 0, 0, 0}
};

void run_ircssd(int irc_port, int ss_port) {
    pthread_t pt_irc, pt_ss;
    hcreate(MAX_HTAB);

    /* validate port numbers */
    if (irc_port < 1 || irc_port > 65535) error("invalid irc-port.");
    if (ss_port < 1 || ss_port > 65535) error("invalid ss-port.");

    /* launch threads for listening irc server and bot server */
    pthread_create(&pt_irc, NULL, run_irc_srv, (void *) &irc_port);
    pthread_create(&pt_ss, NULL, run_ss_srv, (void *) &ss_port);

    /* wait for all threads to complete  before continuing */
    pthread_join(pt_irc, NULL);
    pthread_join(pt_ss, NULL);
}

int main(int argc, char **argv) {
    int next_arg, irc_port, ss_port;
    extern char *optarg;
    extern int optind;
    
    /* parse all command line arguments */
    while (1) {
        int option_index = 0;

        /* parse the next argument based on specified possible arguments */
        next_arg = getopt_long_only(argc, argv, "p:s:hv", long_options, &option_index);

        /* done parsing arguments, move on */
        if (next_arg == -1) break;

        switch(next_arg) {
            /* set listening irc server port */
            case 'p': irc_port = atoi(optarg); break;
            /* set listening bot server port */
            case 's': ss_port = atoi(optarg); break;
            case 'v': print_version(); break;
            case 'h': default: print_help(); break;
        }
    }

    /* both irc server and bot server ports are required arguments */
    if (irc_port == 0 || ss_port == 0) print_help();

    run_ircssd(irc_port, ss_port);

    return 0;
}

