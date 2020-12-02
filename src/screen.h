#include <lvgl.h>
#include "abode.h"

#define REQUEST_IDLE            0

#define REQUEST_MODE_STANDBY     1
#define REQUEST_MODE_HOME        2
#define REQUEST_MODE_AWAY        3

#define LIGHT_SENSOR_PIN         33

//Select your screen model
#define SCREEN_MODEL_ER_43
//#define SCREEN_MODEL_ER_50

LV_FONT_DECLARE(rubik_16);
LV_FONT_DECLARE(rubik_22);
LV_FONT_DECLARE(rubik_36);

#if defined(SCREEN_MODEL_ER_50)
    LV_IMG_DECLARE(icon_standby);
    LV_IMG_DECLARE(icon_home);
    LV_IMG_DECLARE(icon_danger);
    LV_IMG_DECLARE(home_small_white);
    LV_IMG_DECLARE(away_small_white);
    #define ICON_STANDBY    &icon_standby
    #define ICON_HOME       &icon_home
    #define ICON_DANGER     &icon_danger
    #define ICON_HOME_SM_WH &home_small_white
    #define ICON_AWAY_SM_WH &home_small_white
#elif defined(SCREEN_MODEL_ER_43)
    LV_IMG_DECLARE(icon_standby_43);
    LV_IMG_DECLARE(icon_home_43);
    LV_IMG_DECLARE(icon_danger_43);
    LV_IMG_DECLARE(home_small_white);
    LV_IMG_DECLARE(away_small_white);
    #define ICON_STANDBY    &icon_standby_43
    #define ICON_HOME       &icon_home_43
    #define ICON_DANGER     &icon_danger_43
    #define ICON_HOME_SM_WH &home_small_white
    #define ICON_AWAY_SM_WH &home_small_white
#endif

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
        void startTimer(String mode, int seconds);
        void updateTimer(int seconds);

        void setBrightness(short brightness);
        void off();
        void wake();

        short loop();
};