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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

#include "dl.h"


#define MiB(x) (x / (1024*1024))


/**
 *
 **/
static const char*
basename(const char *path) {
  const char* ptr = path;

  while(path && *path) {
    if(*path == '/') {
      ptr = path+1;
    }
    path++;
  }

  return ptr;
}


/**
 *
 **/
static int
endswith(const char *str, const char* suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if(str_len < suffix_len) {
    return 0;
  }

  return strcmp(str+str_len-suffix_len, suffix) == 0;
}


/**
 *
 **/
static int
on_progress(void* ctx, size_t written, size_t remaining) {
  const char* filename = (const char*)ctx;
  static time_t start_time = 0;
  static time_t prev_time = 0;
  time_t now = time(0);

  if(start_time == 0) {
    start_time = prev_time = now;
  }

  if(prev_time < now) {
    double progress = 100.0 * written / (written + remaining);
    double speed = MiB((double)written / (now - start_time));

    fprintf(stdout, "filename: %s | progress: %6.2f%% | avg speed: %6.2f MiB/s\r",
	    filename, progress, speed);
    fflush(stdout);

    prev_time = now;
  }

  return 0;
}


int main(int argc, char *argv[]) {
  char output[PATH_MAX] = {0};
  char url[PATH_MAX] = {0};
  char buf[PATH_MAX];
  int opt;

  while((opt=getopt(argc, argv, "p:o:")) != -1) {
    switch (opt) {
    case 'p':
      sprintf(buf, "CURL_PROXY=%s", optarg);
      putenv(buf);
      break;

    case 'o':
      strncpy(output, optarg, sizeof(output));
      break;

    default:
      fprintf(stderr, "Usage: %s [-p PROXY] [-o PATH] URL\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if(optind >= argc) {
    fprintf(stderr, "Usage: %s [-p PROXY] [-o PATH] URL\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  strncpy(url, argv[optind], sizeof(url));
  if(endswith(url, "_sc.pkg") || endswith(url, "-DP.pkg")) {
    url[strlen(url)-7] = 0;
    strcat(url, ".json");

  } else if(endswith(url, "_0.pkg")) {
    url[strlen(url)-6] = 0;
    strcat(url, ".json");
  }

  if(!output[0]) {
    strcpy(output, basename(url));
    if(endswith(output, ".json")) {
      output[strlen(output)-5] = 0;
      strcat(output, ".pkg");
    }
  }

  return dl_package(url, output, on_progress, (void*)basename(output));
}
