#include "kinematics.h"
#include "rtapi.h"
#include "rtapi_app.h"
#include "rtapi_math.h"
#include "hal.h"

struct hal_joint_t {
  hal_bit_t *homed;
  hal_bit_t *home_switch;
  hal_bit_t *home;
};

struct haldata {
  hal_float_t *xy, *xz, *xa;
  hal_float_t *yz, *ya, *za;

  hal_bit_t *homing;
  hal_bit_t *running;

  struct hal_joint_t joint[4];
} *haldata = 0;

EXPORT_SYMBOL(kinematicsType);
EXPORT_SYMBOL(kinematicsForward);
EXPORT_SYMBOL(kinematicsInverse);
EXPORT_SYMBOL(kinematicsHome);

void drawbot_home(void*, long);
int export_joint(int, struct hal_joint_t*);

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
    if((status = hal_pin_float_new("drawbot.extent.xy", HAL_OUT, &(haldata->xy), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.xz", HAL_OUT, &(haldata->xz), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.xa", HAL_OUT, &(haldata->xa), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.extent.yz", HAL_OUT, &(haldata->yz), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.extent.ya", HAL_OUT, &(haldata->ya), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.extent.za", HAL_OUT, &(haldata->za), comp_id)) < 0) break;

    if((status = hal_pin_bit_new("drawbot.is-homing", HAL_OUT, &(haldata->homing), comp_id)) < 0) break;
    if((status = hal_pin_bit_new("drawbot.is-running", HAL_IN, &(haldata->running), comp_id)) < 0) break;

    if((status = export_joint(0, &(haldata->joint[0]))) < 0) break;
    if((status = export_joint(1, &(haldata->joint[1]))) < 0) break;
    if((status = export_joint(2, &(haldata->joint[2]))) < 0) break;
    if((status = export_joint(3, &(haldata->joint[3]))) < 0) break;

    if((status = hal_export_funct("drawbot.home", drawbot_home, NULL, 1, 0, comp_id)) < 0) break;
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
    if((status = hal_pin_bit_newf(HAL_IN, &(joint->homed), comp_id, "drawbot.%d.is-homed", num)) < 0) break;
    if((status = hal_pin_bit_newf(HAL_IN, &(joint->home_switch), comp_id, "drawbot.%d.home-sw", num)) < 0) break;
    if((status = hal_pin_bit_newf(HAL_OUT, &(joint->home), comp_id, "drawbot.%d.home", num)) < 0) break;
  } while(false);

  return status;
}

int kinematicsForward(const double *joints,
		      EmcPose *pos,
		      const KINEMATICS_FORWARD_FLAGS *fflags,
		      KINEMATICS_INVERSE_FLAGS *iflags)
{
  return -1;
}

int kinematicsInverse(const EmcPose *pos,
		      double *joints,
		      const KINEMATICS_INVERSE_FLAGS *iflags,
		      KINEMATICS_FORWARD_FLAGS *fflags)
{
  return -1;
}

int kinematicsHome(EmcPose *world,
		   double *joints,
		   KINEMATICS_FORWARD_FLAGS *fflags,
		   KINEMATICS_INVERSE_FLAGS *iflags)
{
  *fflags = 0;
  *iflags = 0;

  joints[0] = 0;
  joints[1] = 0;
  joints[2] = 0;
  joints[3] = 0;

  joints[4] = 0;
  joints[5] = 0;
  joints[6] = 0;
  joints[7] = 0;
  joints[8] = 0;

  return kinematicsForward(joints, world, fflags, iflags);
}

KINEMATICS_TYPE kinematicsType(void) {
  return KINEMATICS_IDENTITY;
}

void drawbot_home(void *args, long period) {
  int idx, state = 0;

  if(*(haldata->running)) {
    *(haldata->homing) = 0;
    return;
  }

  for(idx = 0; idx < 4; ++idx) {
    if(*(haldata->joint[idx].homed)) {
      *(haldata->joint[idx].home) = 0;
      state |= (1 << idx);
    } else if(*(haldata->joint[idx].home_switch)) {
      *(haldata->joint[idx].home) = 1;
      state |= (1 << idx);
    } else {
      *(haldata->joint[idx].home) = 0;
    }
  }

  switch(state) {
  case 0x0:
  case 0x1:
  case 0x2:
  case 0x3:
  case 0x4:
  case 0x5:
  case 0x6:
  case 0x7:
  case 0x8:
  case 0x9:
  case 0xa:
  case 0xb:
  case 0xc:
  case 0xd:
  case 0xe:
    *(haldata->homing) = 1;
    return;
  case 0xf:
  default:
    *(haldata->homing) = 0;
    return;
  }
}
