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

#define DEBOUNCE_COUNT 16

volatile int shutdown = 0;

struct axis_state {
  int x, y, z, a;
  int xy, xz, xa;
  int yz, ya;
  int za;
};

static struct option long_options[] = {
  {"device", required_argument, NULL, 'd'},
  {0},
};

int read_data(int);
int write_data(int, unsigned char);
int status_pins(int);
void strobe_blink(int);

void sig_shutdown(int);
int zero(const int, struct axis_state *p);
void step(int fd, unsigned char, unsigned char, int);

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
    printf("A parallel port must be specified with the --device option\n");
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

  struct axis_state state = {0};
  if(zero(fd, &state)) {
    printf("Fault!");
  }
  else {
    printf("XY: %d\n", state.xy);
    printf("XZ: %d\n", state.xz);
    printf("XA: %d\n", state.xa);

    printf("YZ: %d\n", state.yz);
    printf("YA: %d\n", state.ya);

    printf("ZA: %d\n", state.za);
  }

  ioctl(fd, PPRELEASE);
  close(fd);

  return 0;
}

int zero(const int fd, struct axis_state *state) {
  int delay = 500;
  int cmd, dir, status, limit, tsteps;

  memset(state, 0, sizeof(state));

  cmd = X_AXIS_STEP | Z_AXIS_STEP;
  dir = AXIS(Z_AXIS_DIR, X_AXIS_DIR);

  limit = 0;
  while(1) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    if(status & X_AXIS_LIMIT) {
      if(limit < DEBOUNCE_COUNT) {
	++limit;
      } else {
	break;
      }
    }
    step(fd, dir, cmd, delay);
  }
  printf("X homed\n");

  tension(fd, AXIS(0, Z_AXIS_DIR), Z_AXIS_STEP, X_AXIS_LIMIT, 1000);
  printf("XZ tensioned\n");

  limit = 0;
  while(1) {
    status = status_pins(fd);
    if(status & X_AXIS_LIMIT) {
      break;
    }
    step(fd, dir, cmd, delay);
  }

  cmd = X_AXIS_STEP | Z_AXIS_STEP;
  dir = AXIS(X_AXIS_DIR, Z_AXIS_DIR);
  while(1) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    if(status & Z_AXIS_LIMIT) {
      break;
    }

    step(fd, dir, cmd, delay);
    ++state->x;
  }
  state->xz = state->x;
  printf("Z homed\n");

  limit = 0;
  cmd = Z_AXIS_STEP | Y_AXIS_STEP;
  dir = AXIS(Z_AXIS_DIR, Y_AXIS_DIR);
  while(1) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    if(status & Y_AXIS_LIMIT) {
      if(limit < DEBOUNCE_COUNT) {
	++limit;
      } else {
	break;
      }
    }

    step(fd, dir, cmd, delay);
    ++state->z;
  }
  printf("Y homed\n");

  tsteps = tension(fd, AXIS(0, Z_AXIS_DIR), Z_AXIS_STEP, Y_AXIS_LIMIT, 1000);
  if(tsteps < 0) {
    return tsteps;
  }
  state->z -= tsteps;
  state->yz = state->z;
  printf("YZ tensioned\n");

  cmd = X_AXIS_STEP;
  dir = AXIS(0, X_AXIS_DIR);
  while(state->x > state->z) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    step(fd, dir, cmd, delay);
    --state->x;
  }
  state->xy = state->x;
  printf("XY tensioned\n");

  limit = 0;
  cmd = Y_AXIS_STEP | A_AXIS_STEP;
  dir = AXIS(Y_AXIS_DIR, A_AXIS_DIR);
  while(1) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    if(status & A_AXIS_LIMIT) {
      if(limit < DEBOUNCE_COUNT) {
	++limit;
	usleep(delay);
	continue;
      }
      break;
    }
    if(state->y > state->xz) {
      cmd |= X_AXIS_STEP;
      cmd |= Z_AXIS_STEP;
      dir = AXIS_FORWARD(dir, X_AXIS_DIR);
      dir = AXIS_FORWARD(dir, Z_AXIS_DIR);
      ++state->x;
      ++state->z;
    }
    limit = 0;
    ++state->y;
    step(fd, dir, cmd, delay);
  }

  tsteps = tension(fd, AXIS(0, Y_AXIS_DIR), Y_AXIS_STEP, A_AXIS_LIMIT, 1000);
  if(tsteps < 0) {
    return tsteps;
  }
  state->y -= tsteps;
  state->ya = state->y;

  return 0;
}

int tension(int fd, int dir, int cmd, int lim, int delay) {
  int status, limit = 0, steps = 0;
  while(1) {
    status = status_pins(fd);
    if(status & FAULT) {
      return -1;
    }
    if(!(status & lim)) {
      if(limit < DEBOUNCE_COUNT) {
	++limit;
	usleep(1000);
	continue;
      } else {
	break;
      }
    }
    step(fd, dir, cmd, delay);
    ++steps;
  }
  return steps;
}

void step(int fd, unsigned char dir, unsigned char cmd, int delay) {
  write_data(fd, dir | cmd);
  write_data(fd, dir);

  if(delay > 0) {
    usleep(delay);
  }
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
