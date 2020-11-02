#ifndef ABODE_CONFIGURE
#define ABODE_CONFIGURE

#include "lvgl.h"
#include "Vector.h"
#include "WiFi.h"

#define CONFIGURE_IDLE      0
#define CONFIGURE_WIFI      1
#define CONFIGURE_ABODE     2

#define CONNECT_IDLE        -1
#define CONNECT_CONNECTING  0
#define CONNECT_SUCCESS     1
#define CONNECT_FAILED      2
/*
LV_FONT_DECLARE(rubrik_16);
LV_FONT_DECLARE(rubrik_22);
LV_FONT_DECLARE(rubrik_36);
*/
class Configure {
    private:
    public:
      void welcome();

      void wifiSelect();
      void abode();

      short loop();
};

#endif