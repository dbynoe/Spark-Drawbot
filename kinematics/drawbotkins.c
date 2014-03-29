// This file is part of the Four Cable Sand Plotter Code. It free software: you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// The project was designed and constructed by David Bynoe - http://www.davidbynoe.com
// This software is based on software authored by Kevin Loney - http://brainsinjars.com/
//
// Copyright (C) 2014 David Bynoe

#include <float.h>

#include "kinematics.h"
#include "rtapi.h"
#include "rtapi_app.h"
#include "rtapi_math.h"
#include "hal.h"

struct hal_joint_t {
  hal_bit_t *homed;
  hal_bit_t *home_switch;
  hal_bit_t *home;

  hal_float_t *jog;
  hal_bit_t *position;

  hal_bit_t started, tripped;
};

struct haldata {
  hal_float_t *radius;	// radius of the drawing surface
  hal_float_t *limit;	// distance from the carriage pivot to the string - this is depreciated DB

  // Dimensions of the drawbot
  hal_float_t *dimx;
  hal_float_t *dimy;
  hal_float_t *dimz;

  // Size of the drawing bed
  hal_float_t *limx;
  hal_float_t *limy;

  hal_bit_t *occupied;

  struct hal_joint_t joint[4];
} *haldata = 0;

EXPORT_SYMBOL(kinematicsType);
EXPORT_SYMBOL(kinematicsForward);
EXPORT_SYMBOL(kinematicsInverse);
EXPORT_SYMBOL(kinematicsHome);

int export_joint(int, struct hal_joint_t*);


void drawbot_home(void*, long);
void drawbot_home_x(struct hal_joint_t*);
void drawbot_home_y(struct hal_joint_t*);
void drawbot_home_z(struct hal_joint_t*);
void drawbot_home_a(struct hal_joint_t*);

int comp_id;

int rtapi_app_main(void) {
  int status = 0;

  comp_id = hal_init("drawbotkins");
  if(comp_id < 0) {
    return comp_id;
  }

  do {
    haldata = hal_malloc(sizeof(struct haldata));
    if(!haldata) {
      status = -1;
      break;
    }

    // Maximum draw area extents
    if((status = hal_pin_float_new("drawbot.extent.radius", HAL_IN, &(haldata->radius), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.limit", HAL_IN, &(haldata->limit), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.extent.dim-x", HAL_IN, &(haldata->dimx), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.dim-y", HAL_IN, &(haldata->dimy), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.dim-z", HAL_IN, &(haldata->dimz), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.extent.lim-x", HAL_IN, &(haldata->limx), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.lim-y", HAL_IN, &(haldata->limy), comp_id)) < 0) break;

    if((status = hal_pin_bit_new("drawbot.is-occupied", HAL_IN, &(haldata->occupied), comp_id)) < 0) break;

    if((status = export_joint(0, &(haldata->joint[0]))) < 0) break;
    if((status = export_joint(1, &(haldata->joint[1]))) < 0) break;
    if((status = export_joint(2, &(haldata->joint[2]))) < 0) break;
    if((status = export_joint(3, &(haldata->joint[3]))) < 0) break;

  } while(0);

  if(status) {
    hal_exit(comp_id);
  } else {
    hal_ready(comp_id);
  }
  return status;
}

void rtapi_app_exit(void) {
  hal_exit(comp_id);
}

int export_joint(int num, struct hal_joint_t *joint) {
  int status = 0;

  do {
    joint->started = 0;
    joint->tripped = 0;

    if((status = hal_pin_bit_newf(HAL_IN, &(joint->homed), comp_id, "drawbot.%d.is-homed", num)) < 0) break;
    if((status = hal_pin_bit_newf(HAL_IN, &(joint->home_switch), comp_id, "drawbot.%d.home-sw", num)) < 0) break;
    if((status = hal_pin_bit_newf(HAL_OUT, &(joint->home), comp_id, "drawbot.%d.home", num)) < 0) break;

    if((status = hal_pin_float_newf(HAL_OUT, &(joint->jog), comp_id, "drawbot.%d.jog", num)) < 0) break;
    if((status = hal_pin_bit_newf(HAL_IN, &(joint->position), comp_id, "drawbot.%d.in-position", num)) < 0) break;
  } while(false);

  return status;
}

double fmin(const double x, const double y) {
  return x < y ? x : y;
}

double fmax(const double x, const double y) {
  return x > y ? x : y;
}

double fclamp(double v, double lo, double hi) {
	return fmax(fmin(v, hi), lo);
}
int kinematicsForward(const double *joints,
		      EmcPose *pos,
		      const KINEMATICS_FORWARD_FLAGS *fflags,
		      KINEMATICS_INVERSE_FLAGS *iflags)
{ //So apparently if this section is not here it will still compile, but not work. TIL. 
  return -1;
}
int kinematicsInverse(const EmcPose *pos,
		      double *joints,
		      const KINEMATICS_INVERSE_FLAGS *iflags,
		      KINEMATICS_FORWARD_FLAGS *fflags)
{//the code below was writen by Kevin, it wasn't big on comments..
  const double rt2 = sqrt(2.0);

  int idx, sx = -1, sy = 1, nx, ny;
  double dx, dy, dz, px, py, pz, offset;

  double limx = fmin(*(haldata->dimx) - 2.0 * *(haldata->limit), *(haldata->limx));
  double limy = fmin(*(haldata->dimy) - 2.0 * *(haldata->limit), *(haldata->limy));
  double limz = *(haldata->dimz);


	  px = fclamp(pos->tran.x, -0.5 * limx, 0.5 * limx);
  py = fclamp(pos->tran.y, -0.5 * limy, 0.5 * limy);
  pz = fclamp(pos->tran.z, 0.0, limz);
	// TODO: This needs to compensate for the carriage tipping the further it gets from the center
  for(idx = 0; idx < 4; ++idx) {
    double tower_x = 0.5 * sx * *(haldata->dimx);
    double tower_y = 0.5 * sy * *(haldata->dimy);

    double carriage_x = px + rt2 * sx * *(haldata->radius);
    double carriage_y = py + rt2 * sy * *(haldata->radius);

    dx = carriage_x - tower_x;
    dy = carriage_y - tower_y;
    dz = limz - pz;

    joints[idx] = sqrt(dx*dx + dy*dy + dz*dz) - *(haldata->limit);
    if(joints[idx] < 0.0) {
      joints[idx] = 0.0;
    }
	joints[idx] -= offset;

    nx = sy; ny = -sx;
    sx = nx; sy = ny;
  }

  for(; idx < 9; ++idx) {
    joints[idx] = 0.0;
  }

  return 0;
}

int kinematicsHome(EmcPose *world,
		   double *joints,
		   KINEMATICS_FORWARD_FLAGS *fflags,
		   KINEMATICS_INVERSE_FLAGS *iflags)
{
  int idx;
  //The below line calculates the 3d triangle between the tower and the centre of the bed, I have no idea how operator precedence works in C so.. brackets!
  double hypot = sqrt(((*(haldata->dimx)/2) * (*(haldata->dimx)/2))+((*(haldata->dimy)/2) * (*(haldata->dimy)/2))+( *(haldata->dimz) * *(haldata->dimz)));
  *fflags = 0;
  *iflags = 0;
  for(idx = 0; idx < 9; ++ idx) {
    joints[idx] = idx < 4 ? hypot - *(haldata->radius) : 0.0;
  }
  // Because of the way the ZERO_EMC_POSE macro the extra parens are mandatory
  ZERO_EMC_POSE((*world));
  return 0;
  
}

KINEMATICS_TYPE kinematicsType(void) {
//if this is set to kinematics both the x y z display doesn't work. 
  return KINEMATICS_INVERSE_ONLY;
}

int in_position(struct hal_joint_t *joints) { //I have no idea what this is
  int idx;
  for(idx = 0; idx < 4; ++idx) {
    if(!*(joints[idx].position)) {
      return 0;
    }
  }
  return 1;
}

void halt_motion(struct hal_joint_t *joints) {
  int idx = 0;
  for(idx = 0; idx < 4; ++idx) {
    *(joints[idx].jog) = 0.0;
  }
}

