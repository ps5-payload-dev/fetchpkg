/* Copyright (C) 2024 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curl/curl.h>

#include "parson.h"
#include "dl.h"


/**
 *
 **/
typedef size_t (dl_data_write_t)(void *ptr, size_t length, size_t nmemb, void *ctx);


/**
 *
 **/
typedef struct dl_manifest_state {
  char  *data;
  size_t size;
  int    error;
} dl_manifest_state_t;


/**
 *
 **/
typedef struct dl_package_state {
  FILE *file;
  int   error;
  size_t remaining;

  struct {
    dl_progress_t *cb;
    void* ctx;
  } on_progress;
} dl_package_state_t;


/**
 *
 **/
static size_t
dl_manifest_write(void *ptr, size_t length, size_t nmemb, void *ctx) {
  dl_manifest_state_t *state = (dl_manifest_state_t*)ctx;
  size_t size = length * nmemb;

  if(!(state->data=realloc(state->data, state->size + size + 1))) {
    state->error = errno;
    return 0;
  }

  memcpy(state->data + state->size, ptr, size);
  state->size += size;
  state->data[state->size] = 0;

  return size;
}


/**
 *
 **/
static size_t
dl_package_write(void *ptr, size_t length, size_t nmemb, void *ctx) {
  dl_package_state_t *state = (dl_package_state_t*)ctx;
  int n;

  if(!(n=fwrite(ptr, length, nmemb, state->file))) {
    state->error = ferror(state->file);
  }

  if(n <= state->remaining) {
    state->remaining -= n;
  } else {
    state->remaining = 0;
  }

  if(state->on_progress.cb &&
     state->on_progress.cb(state->on_progress.ctx,
			   ftell(state->file),
			   state->remaining)) {
    return 0;
  }

  return n;
}


/**
 *
 **/
static int
dl_fetch(const char* url, dl_data_write_t* cb, void* ctx) {
  const char* proxy;
  int error = 0;
  CURL *curl;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  if(!(curl=curl_easy_init())) {
    fprintf(stderr, "dl_fetch: curl_easy_init() failed\n");
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctx);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  if((proxy=getenv("CURL_PROXY"))) {
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
  }

  if(curl_easy_perform(curl) != CURLE_OK) {
    fprintf(stderr, "dl_fetch: curl_easy_perform() failed\n");
    error = -1;
  }
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return error;
}


/**
 *
 **/
static JSON_Value*
dl_manifest(const char *url) {
  dl_manifest_state_t state = {0};
  JSON_Value *json = 0;

  if(!dl_fetch(url, dl_manifest_write, &state)) {
    if(state.error) {
      fprintf(stderr, "dl_manifest: %s\n", strerror(state.error));
    } else if(!(json=json_parse_string(state.data))) {
      fprintf(stderr, "dl_manifest: json_tokener_parse() failed\n");
    }
  }

  if(state.data) {
    free(state.data);
  }

  return json;
}


/**
 *
 **/
static int
dl_package_piece(dl_package_state_t* state, const char *url) {
  if(dl_fetch(url, dl_package_write, state)) {
    return -1;
  }

  if(state->error) {
    fprintf(stderr, "dl_package_piece: %s\n", strerror(state->error));
    return state->error;
  }

  return 0;
}


/**
 *
 **/
int
dl_package(const char* manifest_url, const char* path, dl_progress_t* cb,
	   void* ctx) {
  dl_package_state_t state = {0};
  JSON_Object *manifest = 0;
  JSON_Array *pieces = 0;
  JSON_Object *piece = 0;
  JSON_Value *val = 0;
  const char* url;
  int error = 0;

  state.on_progress.cb = cb;
  state.on_progress.ctx = ctx;

  if(!(val=dl_manifest(manifest_url))) {
    return -1;
  }

  manifest = json_value_get_object(val);
  if(!(pieces=json_object_get_array(manifest, "pieces"))) {
    fprintf(stderr, "dl_package: malformed manifest\n");
    error = -1;

  } else if(!(state.file=fopen(path, "wb"))) {
    fprintf(stderr, "dl_package: %s\n", strerror(errno));
    error = -1;
  }

  state.remaining = json_object_get_number(manifest, "originalFileSize");
  for(int i=0; i<json_array_get_count(pieces) && !error; i++) {
    piece = json_array_get_object(pieces, i);
    url = json_object_get_string(piece, "url");
    error = dl_package_piece(&state, url);
  }

  if(state.file) {
    fclose(state.file);
  }

  json_value_free(val);

  return error;
}


