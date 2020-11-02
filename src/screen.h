#include <lvgl.h>
#include "abode.h"

#define REQUEST_IDLE            0

#define REQUEST_MODE_STANDBY     1
#define REQUEST_MODE_HOME        2
#define REQUEST_MODE_AWAY        3

LV_FONT_DECLARE(rubik_16);
LV_FONT_DECLARE(rubik_22);
LV_FONT_DECLARE(rubik_36);

LV_IMG_DECLARE(icon_standby);
LV_IMG_DECLARE(icon_home);
LV_IMG_DECLARE(icon_danger);

#define DEGREES_SYMBOL "\xC2\xB0"

class Screen {
    private:
        String _state = ABODE_DISCONNECT;
        String _mode = "";

    public:
        void begin();

        void loading();

        void home();

        void configureWifi();

        void setState(const char * state);
        void setMode(String mode);

        void setNight();
        void setDay();
        void off();
        void wake();

        short loop();
};