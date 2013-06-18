#ifndef __GECKO_H__
#define __GECKO_H__

#include <linux/parport.h>

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

#define FAULT PARPORT_STATUS_ERROR

#define AXIS_FORWARD(data, axis) ((data) | (axis))
#define AXIS_REVERSE(data, axis) ((data) & ~(axis))

#endif
