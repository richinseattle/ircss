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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ss.h"

void ss_send(seed_t *seed, freq_t *pool, int pool_sz, char *msg) { 
  int cur_freq = 0, i;
  
  for (i = 0, seed->pos = 0; i < strlen(msg); i++, seed->pos++) {
    if (seed->pos >= seed->sz)
      seed->pos = 0;

    cur_freq = seed->arr[seed->pos];
    if (cur_freq >= pool_sz)
      cur_freq %= pool_sz;

    if (strlen(pool[cur_freq].stream) < MAX_MSG) {
      char buf[] = {msg[i], '\0'};
      strcat(pool[cur_freq].stream, buf);
    }
  }
}

char *ss_recv(seed_t *seed, freq_t *pool, int pool_sz) {
  char *msg = calloc(MAX_MSG, sizeof(char));
  //char msg[MAX_MSG];
  int  cur_freq = 0, i;
  int pool_ct[pool_sz], msg_len = 0;
  
  msg[0] = '\0';

  for (i = 0; i < pool_sz; i++)
    pool_ct[i] = 0;
  
  for (i = 0; i < pool_sz; i++)
    msg_len += strlen(pool[i].stream);
  
  for (i = 0, seed->pos = 0; i < msg_len; i++, seed->pos++) {
    if (seed->pos >= seed->sz)
      seed->pos = 0;

    cur_freq = seed->arr[seed->pos];
    if (cur_freq >= pool_sz)
      cur_freq %= pool_sz;
  
    char buf[] = {pool[cur_freq].stream[pool_ct[cur_freq]], '\0'};
    strcat(msg, buf);
    pool_ct[cur_freq]++;
  }
   
  return msg;
}

void init_freq(freq_t *freq) {
  freq->stream = calloc(MAX_MSG, sizeof(char));
}

void kill_freq(freq_t *freq) {
  free(freq->stream);
}

seed_t init_seed(char *seed_in) {
  seed_t seed = {calloc(MAX_SEED, sizeof(int)), 0, 0};
  int i;
  
  for (i = 0; i < strlen(seed_in) && i < MAX_SEED; i++) {
    seed.arr[i] = seed_in[i] - '0';

    if (seed.arr[i] < 0 || seed.arr[i] > 9) {
      fprintf(stderr, "invalid seed\n");
      exit(EXIT_FAILURE);
    }
  } 

  seed.sz = i;
  
  return seed;
}

void kill_seed(seed_t *seed) {
  free(seed->arr);
}
