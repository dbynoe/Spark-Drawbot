#include "kinematics.h"
#include "rtapi.h"
#include "rtapi_app.h"
#include "hal.h"

EXPORT_SYMBOL(kinematicsType);
EXPORT_SYMBOL(kinematicsForward);
EXPORT_SYMBOL(kinematicsInverse);
EXPORT_SYMBOL(kinematicsHome);

int comp_id;

int rtapi_app_main(void) {
  comp_id = hal_init("drawkins");
  if(comp_id > 0) {
    hal_ready(comp_id);
    return 0;
  }
  return comp_id;
}

void rtapi_app_exit(void) {
  hal_exit(comp_id);
}

int kinematicsForward(const double *joints,
		      EmcPose *pos,
		      const KINEMATICS_FORWARD_FLAGS *fflags,
		      KINEMATICS_INVERSE_FLAGS *iflags)
{
  return 0;
}

int kinematicsInverse(const EmcPose *pos,
		      double *joints,
		      const KINEMATICS_INVERSE_FLAGS *iflags,
		      KINEMATICS_FORWARD_FLAGS *fflags)
{
  return 0;
}

int kinematicsHome(EmcPose *world,
		   double *joint,
		   KINEMATICS_FORWARD_FLAGS *fflags,
		   KINEMATICS_INVERSE_FLAGS *iflags)
{
  // TODO: read in current joint positions from HAL

  *fflags = 0;
  *iflags = 0;
  return 0;
}

KINEMATICS_TYPE kinematicsType(void) {
  return KINEMATICS_IDENTITY;
}
