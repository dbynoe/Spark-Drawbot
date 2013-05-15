#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>

// TODO: Read this in from command line
#define DEVICE "/dev/parport0"

#define X_AXIS_STEP 0x01
#define Y_AXIS_STEP 0x04
#define Z_AXIS_STEP 0x10
#define A_AXIS_STEP 0x40

#define X_AXIS_DIR 0x02
#define Y_AXIS_DIR 0x08
#define Z_AXIS_DIR 0x20
#define A_AXIS_DIR 0x80

#define X_AXIS_LIMIT PARPORT_STATUS_ACK
#define Y_AXIS_LIMIT PARPORT_STATUS_BUSY
#define Z_AXIS_LIMIT PARPORT_STATUS_PAPEROUT
#define A_AXIS_LIMIT PARPORT_STATUS_SELECT

#define AXIS_FORWARD(data, axis) ((data) |= (axis))
#define AXIS_REVERSE(data, axis) ((data) &= ~(axis))

static struct option long_options[] = {
  {"device", required_argument, NULL, 'd'},
  {0},
};

int read_data(int);
int write_data(int, unsigned char);
int status_pins(int);
int strobe_blink(int);

int main(int argc, char *argv[]) {
  int n, fd, mode;

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

  if((fd = open(device, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open %s\n", device);
    return 10;
  }
  if(ioctl(fd, PPCLAIM)) {
    perror("PPCLAIM");
    close(fd);
    return 10;
  }

  for(n = 0; n < 1000; ++n) {
    write_data(fd, 0x03);
    strobe_blink(fd);
    write_data(fd, 0x02);
    strobe_blink(fd);
  }

  for(n = 0; n < 1000; ++n) {
    write_data(fd, 0x01);
    strobe_blink(fd);
    write_data(fd, 0x00);
    strobe_blink(fd);
  }
  status_pins(fd);

  ioctl(fd, PPRELEASE);
  close(fd);

  return 0;
}

int read_data(int fd) {
  int mode, res;
  unsigned char data;

  mode = IEEE1284_MODE_ECP;
  res=ioctl(fd, PPSETMODE, &mode); /* ready to read ? */
  mode=255;
  res=ioctl(fd, PPDATADIR, &mode); /* switch output driver off */
  printf("ready to read data !\n");
  fflush(stdout);
  sleep(10);
  res=ioctl(fd, PPRDATA, &data); /* now fetch the data! */
  printf("data=%02x\n", data);
  fflush(stdout);
  sleep(10);
  data=0;
  res=ioctl(fd, PPDATADIR, data);
  return 0;
}

int write_data(int fd, unsigned char data) {
  return ioctl(fd, PPWDATA, &data);
}

int status_pins(const int fd) {
  int val;
  ioctl(fd, PPRSTATUS, &val);
  return val ^ PARPORT_STATUS_BUSY;
}

int strobe_blink(int fd) {
  struct ppdev_frob_struct frob;

  frob.mask = PARPORT_CONTROL_STROBE; /* change only this pin ! */

  frob.val = PARPORT_CONTROL_STROBE; /* set STROBE ... */
  ioctl(fd, PPFCONTROL, &frob);
  usleep(1);

  frob.val = 0; /* and clear again */
  ioctl(fd, PPFCONTROL, &frob);
  usleep(2);
}
