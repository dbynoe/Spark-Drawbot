#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>

#include "gecko.h"

volatile int shutdown = 0;

static struct option long_options[] = {
  {"device", required_argument, NULL, 'd'},
  {0},
};

int read_data(int);
int write_data(int, unsigned char);
int status_pins(int);
void strobe_blink(int);

void sig_shutdown(int);
void zero(const int);
void step(int fd, unsigned char, unsigned char);

int main(int argc, char *argv[]) {
  int fd;
  char *device = NULL;

  signal(SIGINT, sig_shutdown);

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

  if(device == NULL) {
    printf("A parallel must be specified with the --device option\n");
    return -1;
  }

  if((fd = open(device, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open %s\n", device);
    return 10;
  }
  if(ioctl(fd, PPCLAIM)) {
    perror("PPCLAIM");
    close(fd);
    return 10;
  }

  zero(fd);

  /*
  const int limmask = X_AXIS_LIMIT | Y_AXIS_LIMIT | Z_AXIS_LIMIT | A_AXIS_LIMIT;
  int prev = 0;
  while(!shutdown) {
    int curr = status_pins(fd) & limmask;
    if(curr == prev) {
      continue;
    }
    prev = curr;

    if(curr == 0) {
      continue;
    }

    if(curr & X_AXIS_LIMIT) printf("X");
    if(curr & Y_AXIS_LIMIT) printf("Y");
    if(curr & Z_AXIS_LIMIT) printf("Z");
    if(curr & A_AXIS_LIMIT) printf("A");
    printf("\n");
  }
  */

  ioctl(fd, PPRELEASE);
  close(fd);

  return 0;
}

void zero(const int fd) {
  int delay = 300;
  int cmd, dir, status = status_pins(fd);

  if(status & Z_AXIS_LIMIT) {
    printf("Z is homed\n");
  } else if(status & X_AXIS_LIMIT) {
    printf("X is homed\n");

    int steps = 0;
    cmd = X_AXIS_STEP | Z_AXIS_STEP;

    dir = 0;
    dir = AXIS_FORWARD(dir, X_AXIS_DIR);
    dir = AXIS_REVERSE(dir, Z_AXIS_DIR);
    while(1) {
      status = status_pins(fd);
      if(status & FAULT) {
	printf("Fault!\n");
	return;
      }
      if(status & Z_AXIS_LIMIT) {
	break;
      }

      step(fd, dir, cmd);
      usleep(delay);

      ++steps;
    }

    printf("Limit Tripped\n");

    cmd = X_AXIS_STEP;
    dir = 0;
    dir = AXIS_REVERSE(dir, X_AXIS_DIR);

    while(status & Z_AXIS_LIMIT) {
      status = status_pins(fd);
      if(status & FAULT) {
	return;
      }

      step(fd, dir, cmd);
      usleep(delay);

      --steps;
    }

    printf("%d\n", steps);

    return;
  } else if(status & Y_AXIS_LIMIT) {
    printf("Y is homed\n");
  } else if(status & A_AXIS_LIMIT) {
    printf("A is homed\n");
  } else {
    delay = 500;
    cmd = X_AXIS_STEP | Z_AXIS_STEP;
    dir = AXIS_REVERSE(dir, X_AXIS_DIR);
    dir = AXIS_FORWARD(dir, Z_AXIS_DIR);

    while(1) {
      status = status_pins(fd);

      if(status & FAULT) {
	printf("Fault!\n");
	break;
      }
      if(status & X_AXIS_LIMIT) {
	zero(fd);
	break;
      }
      step(fd, dir, cmd);
      usleep(delay);
    }
  }

  return;
}

void step(int fd, unsigned char dir, unsigned char cmd) {
  write_data(fd, dir | cmd);
  write_data(fd, dir);
}

int write_data(int fd, unsigned char data) {
  int err = ioctl(fd, PPWDATA, &data);
  if(err) {
    return err;
  }
}

int status_pins(const int fd) {
  int val;
  ioctl(fd, PPRSTATUS, &val);
  return val ^ PARPORT_STATUS_BUSY;
}

void strobe_blink(int fd) {
  struct ppdev_frob_struct frob;

  frob.mask = PARPORT_CONTROL_STROBE; // Change only the strobe pin

  frob.val = PARPORT_CONTROL_STROBE; // Set the strobe
  ioctl(fd, PPFCONTROL, &frob);
  usleep(1);

  frob.val = 0; // clear the strobe
  ioctl(fd, PPFCONTROL, &frob);
  usleep(2);
}

void sig_shutdown(int sig) {
  shutdown = 1;
}
