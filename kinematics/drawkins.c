#include <linuxcnc/kinematics.h>
#include <linuxcnc/rtapi.h>
#include <linuxcnc/rtapi_app.h>
#include <linuxcnc/hal.h>

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
