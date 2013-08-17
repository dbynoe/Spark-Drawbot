#include "kinematics.h"
#include "rtapi.h"
#include "rtapi_app.h"
#include "rtapi_math.h"
#include "hal.h"

#define FB_TOLERANCE 1E-3

struct hal_joint_t {
  hal_bit_t *homed;
  hal_bit_t *home_switch;
  hal_bit_t *home;

  hal_float_t *cmd;
  hal_float_t *fb;
  hal_float_t *jog;
};

struct haldata {
  hal_float_t *xy, *xz, *xa;
  hal_float_t *yz, *ya, *za;

  hal_bit_t *homing;
  hal_bit_t *occupied;

  struct hal_joint_t joint[4];
} *haldata = 0;

EXPORT_SYMBOL(kinematicsType);
EXPORT_SYMBOL(kinematicsForward);
EXPORT_SYMBOL(kinematicsInverse);
EXPORT_SYMBOL(kinematicsHome);

int export_joint(int, struct hal_joint_t*);

void drawbot_home(void*, long);
void drawbot_home_x(struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*);
void drawbot_home_y(struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*);
void drawbot_home_z(struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*);
void drawbot_home_a(struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*);

int is_moving(struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*, struct hal_joint_t*);
int is_joint_moving(struct hal_joint_t*);

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
    if((status = hal_pin_bit_new("drawbot.is-occupied", HAL_IN, &(haldata->occupied), comp_id)) < 0) break;

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

    if((status = hal_pin_float_newf(HAL_IN, &(joint->cmd), comp_id, "drawbot.%d.pos-cmd", num)) < 0) break;
    if((status = hal_pin_float_newf(HAL_IN, &(joint->fb), comp_id, "drawbot.%d.pos-fb", num)) < 0) break;

    if((status = hal_pin_float_newf(HAL_OUT, &(joint->jog), comp_id, "drawbot.%d.pos-jog", num)) < 0) break;
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
  int homing = 1;
  if(*(haldata->occupied)) {
    homing = 0;
  } else if(!*(haldata->joint[0].homed)) {
    drawbot_home_x(&(haldata->joint[0]), &(haldata->joint[1]), &(haldata->joint[2]), &(haldata->joint[3]));
  } else if(!*(haldata->joint[2].homed)) {
    drawbot_home_z(&(haldata->joint[0]), &(haldata->joint[1]), &(haldata->joint[2]), &(haldata->joint[3]));
  } else if(!*(haldata->joint[1].homed)) {
    drawbot_home_y(&(haldata->joint[0]), &(haldata->joint[1]), &(haldata->joint[2]), &(haldata->joint[3]));
  } else if(!*(haldata->joint[3].homed)) {
    drawbot_home_a(&(haldata->joint[0]), &(haldata->joint[1]), &(haldata->joint[2]), &(haldata->joint[3]));
  } else {
    homing = 0;
  }
  *(haldata->homing) = homing;
}

void drawbot_home_x(struct hal_joint_t *x, struct hal_joint_t *y, struct hal_joint_t *z, struct hal_joint_t *a) {
}

void drawbot_home_y(struct hal_joint_t *x, struct hal_joint_t *y, struct hal_joint_t *z, struct hal_joint_t *a) {
}

void drawbot_home_z(struct hal_joint_t *x, struct hal_joint_t *y, struct hal_joint_t *z, struct hal_joint_t *a) {
}

void drawbot_home_a(struct hal_joint_t *x, struct hal_joint_t *y, struct hal_joint_t *z, struct hal_joint_t *a) {
}

int is_moving(struct hal_joint_t *x, struct hal_joint_t *y, struct hal_joint_t *z, struct hal_joint_t *a) {
  return is_joint_moving(x) || is_joint_moving(y) || is_joint_moving(z) || is_joint_moving(a);
}

int is_joint_moving(struct hal_joint_t *joint) {
  hal_float_t delta = *(joint->cmd) - *(joint->fb);
  if(delta < 0.0) {
    delta = -delta;
  }
  return delta > FB_TOLERANCE;
}
