#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <signal.h>

volatile int shutdown = 0;

static struct option long_options[] = {
  {"device", required_argument, NULL, 'd'},
  {0},
};

void signal_shutdown(int sig) {
  shutdown = 1;
}

int main(int argc, char *argv[]) {
  char *device = NULL;

  while(1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "", long_options, &option_index);
    if(c == -1) {
      break;
    }

    switch(option_index) {
    case 0: {
      if(device) {
	free(device);
      }
      device = malloc(strlen(optarg));
      strcpy(device, optarg);
    } break;
    default:
      break;
    }
  }

  signal(SIGINT, signal_shutdown);
  while(!shutdown) {
  }

  return 0;
}
