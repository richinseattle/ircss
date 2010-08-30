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
#define MAX_FREQ 10

typedef struct freq {char *host; char *user; char *chan; char *stream;} freq_t;

typedef struct seed {int *arr; int sz; int pos;} seed_t;

typedef struct user {char *nick; char *user; char *real; char *host; int reg;} user_t;

typedef struct settings {freq_t pool[MAX_FREQ]; int pool_sz; seed_t seed; user_t user; int cli_sockfd;} settings_t;

void ss_send(settings_t *, char *);

char *ss_recv(settings_t *);

void init_freq(freq_t *);

void kill_freq(freq_t *);

int init_seed(seed_t *, char *);

void kill_seed(seed_t *);

void init_settings(settings_t *);

void kill_settings(settings_t *);

#endif

