#include "kinematics.h"
#include "rtapi.h"
#include "rtapi_app.h"
#include "rtapi_math.h"
#include "hal.h"

struct haldata {
  hal_float_t *x, *y, *z, *a;
  hal_float_t *xy, *xz, *xa;
  hal_float_t *yz, *ya, *za;

  hal_float_t *fbx, *fby, *fbz, *fba;

  hal_bit_t *homing;
} *haldata = 0;

#define X (*(haldata->x))
#define Y (*(haldata->y))
#define Z (*(haldata->z))
#define A (*(haldata->a))

EXPORT_SYMBOL(kinematicsType);
EXPORT_SYMBOL(kinematicsForward);
EXPORT_SYMBOL(kinematicsInverse);
EXPORT_SYMBOL(kinematicsHome);

void drawbot_home(void*, long);

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

    // Current motor positions
    if((status = hal_pin_float_new("drawbot.kins.pos.x", HAL_IO, &(haldata->x), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.pos.y", HAL_IO, &(haldata->y), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.pos.z", HAL_IO, &(haldata->z), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.pos.a", HAL_IO, &(haldata->a), comp_id)) < 0) break;

    // Maximum draw area extents
    if((status = hal_pin_float_new("drawbot.kins.extent.xy", HAL_OUT, &(haldata->xy), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.extent.xz", HAL_OUT, &(haldata->xz), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.extent.xa", HAL_OUT, &(haldata->xa), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.kins.extent.yz", HAL_OUT, &(haldata->yz), comp_id)) < 0) break;
    if((status = hal_pin_float_new("drawbot.kins.extent.ya", HAL_OUT, &(haldata->ya), comp_id)) < 0) break;

    if((status = hal_pin_float_new("drawbot.kins.extent.za", HAL_OUT, &(haldata->za), comp_id)) < 0) break;

    if((status = hal_pin_bit_new("drawbot.homing", HAL_IO, &(haldata->homing), comp_id)) < 0) break;

    // Homing function
    if((status = hal_export_funct("drawbot.home", drawbot_home, NULL, 1, 0, comp_id)) < 0) break;

    // Axis feedback
    if((status = hal_pin_float_new("drawbot.home.0.position-fb", )
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

  joints[0] = *(haldata->x);
  joints[1] = *(haldata->y);
  joints[2] = *(haldata->z);
  joints[3] = *(haldata->a);

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
  if(*(haldata->homing)) {
    
  }
}
