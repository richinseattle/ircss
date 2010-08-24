/*
 * Copyright (c) 2010 John Gordon <jgor@indiecom.org> and Doug Farre <dougfarre@gmail.com>
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

#ifndef _SS_H_
#define _SS_H_

#define MAX_SEED 128
#define MAX_MSG 512

typedef struct freq {char *host; char *user; char *chan; char *stream;} freq_t;

typedef struct seed {int *arr; int sz; int pos;} seed_t;

void ss_send(seed_t *, freq_t *, int, char *);

char *ss_recv(seed_t *, freq_t *, int);

void init_freq(freq_t *);

void kill_freq(freq_t *);

seed_t init_seed(char *);

void kill_seed(seed_t *);

#endif

