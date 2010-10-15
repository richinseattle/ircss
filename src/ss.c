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
#include <string.h>
#include <stdlib.h>
#include "ss.h"

//void ss_send(seed_t *seed, freq_t *pool, int pool_sz, char *msg) { 
void ss_send(settings_t *settings, char *msg) {  
  int cur_freq = 0, i;
  
  for (i = 0, settings->seed.pos = 0; i < strlen(msg); i++, settings->seed.pos++) {
    if (settings->seed.pos >= settings->seed.sz)
      settings->seed.pos = 0;

    cur_freq = settings->seed.arr[settings->seed.pos];
    if (cur_freq >= settings->pool_sz)
      cur_freq %= settings->pool_sz;

    if (strlen(settings->pool[cur_freq].stream) < MAX_MSG) {
      char buf[] = {msg[i], '\0'};
      strcat(settings->pool[cur_freq].stream, buf);
    }
  }
}

//char *ss_recv(seed_t *seed, freq_t *pool, int pool_sz) {
void *ss_recv(void *ptr) {  
  settings_t *settings = (settings_t *) ptr;
  char *msg = calloc(MAX_MSG, sizeof(char));
  //char msg[MAX_MSG];
  int  cur_freq = 0, i;
  int pool_ct[settings->pool_sz], msg_len = 0;
  
  msg[0] = '\0';

  for (i = 0; i < settings->pool_sz; i++)
    pool_ct[i] = 0;
  
  for (i = 0; i < settings->pool_sz; i++)
    msg_len += strlen(settings->pool[i].stream);
  
  for (i = 0, settings->seed.pos = 0; i < msg_len; i++, settings->seed.pos++) {
    if (settings->seed.pos >= settings->seed.sz)
      settings->seed.pos = 0;

    cur_freq = settings->seed.arr[settings->seed.pos];
    if (cur_freq >= settings->pool_sz)
      cur_freq %= settings->pool_sz;
  
    char buf[] = {settings->pool[cur_freq].stream[pool_ct[cur_freq]], '\0'};
    strcat(msg, buf);
    pool_ct[cur_freq]++;
  }

  //cli_write(settings, msg);
  cli_write(settings, "this is a test static message from ss_recv()\n");
}

void init_freq(freq_t *freq) {
  freq->stream = calloc(MAX_MSG, sizeof(char));
}

void kill_freq(freq_t *freq) {
  free(freq->stream);
}

int init_seed(seed_t *seed, char *seed_in) {
  seed->arr = calloc(MAX_SEED, sizeof(char));
  int i;
  
  for (i = 0; i < strlen(seed_in) && i < MAX_SEED; i++) {
    seed->arr[i] = seed_in[i] - '0';

    if (seed->arr[i] < 0 || seed->arr[i] > 9) {
      fprintf(stderr, "invalid seed\n");
      return -1;
    }
  } 

  seed->sz = i;

  fprintf(stderr, "DEBUG: seed = ");
  for (i = 0; i < seed->sz; i++)
    fprintf(stderr, "%d", seed->arr[i]);
  fprintf(stderr, "\n");

  return 0;
}

void kill_seed(seed_t *seed) {
  free(seed->arr);
}

void init_settings(settings_t *settings) {
  char buf[MAX_SEED], seed_in[MAX_MSG];
  
/*
    settings->pool[] = {
      {"irc1.example.com", "user1", "channel1", NULL},
      {"irc2.example.com", "user2", "channel2", NULL},
      {"irc3.example.com", "user3", "channel3", NULL},
    };
*/


    //settings->pool[0]->host = "host1";// (freq_t){"irc1.example.com", "user1", "channel1", NULL};
    //settings->pool[0]->user = "user1";// (freq_t){"irc1.example.com", "user1", "channel1", NULL};
    settings->pool[0] = (freq_t){"irc1.example.com", "user1", "channel1", NULL};
     
    int i;
    settings->pool_sz = sizeof(settings->pool) / sizeof(freq_t);

    for (i = 0; i < settings->pool_sz; i++)
      init_freq(&settings->pool[i]);

    //buf = calloc(MAX_SEED, sizeof(char));
    /*printf("Enter seed: ");
    if (fgets(buf, MAX_SEED, stdin) == NULL) {
      fprintf(stderr, "Error reading seed\n");
      exit(EXIT_FAILURE);
    } */
    //buf = "8675309"; 
    strcpy(buf, "8675309");

    //seed_in = calloc(MAX_MSG, sizeof(char));
    sscanf(buf, "%s", seed_in);
    //free(buf);
    int err = init_seed(&(settings->seed), seed_in);
    if (err == -1) fprintf(stderr, "DEBUG: error initializing seed.\n");
    //free(seed_in);

   /*
    msg_in = calloc(MAX_MSG, sizeof(char));
    printf("Enter message: ");
    if (fgets(msg_in, MAX_MSG, stdin) == NULL) {
      fprintf(stderr, "Error reading message\n");
      exit(EXIT_FAILURE);
    }

    ss_send(&seed, pool, pool_sz, msg_in);
    printf("sent: %s\n", msg_in);
    free(msg_in);

    msg_out = ss_recv(&seed, pool, pool_sz);
    printf("rcvd: %s\n", msg_out);
    free(msg_out);

    kill_seed(&seed);

    for (i = 0; i < pool_sz; i++)
      kill_freq(&pool[i]);

    exit(EXIT_SUCCESS);
    */

}//end init

void kill_settings(settings_t *settings) {
  int i;
  kill_seed(&settings->seed);
  for (i = 0; i < settings->pool_sz; i++)
      kill_freq(&settings->pool[i]);

}
