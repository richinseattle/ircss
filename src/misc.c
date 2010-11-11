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
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "core.h"

/*
 * Displays the (printf-style) given message via perror then exits as failure.
 */
void error(char *fmt, ...) {
    char buf[MAX_BUF], msg[MAX_BUF];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, MAX_BUF, fmt, args);
    va_end(args);

    snprintf(msg, MAX_BUF, "ERROR: %s", buf);

    if (errno != 0) perror(msg);
    else fprintf(stderr, "%s\n", msg);

    exit(EXIT_FAILURE);
}

/*
 *  Displays the (printf-style) given message on stderr.
 */
void debug(char *fmt, ...) {
    if (DEBUG) {
        char buf[MAX_BUF];
        va_list args;

        va_start(args, fmt);
        vsnprintf(buf, MAX_BUF, fmt, args);
        va_end(args);

        fprintf(stderr, "DEBUG: %s", buf);
    }
}

