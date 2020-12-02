#include "screen.h"

#include "settings.h"

//ticker for lvgl
#include <Ticker.h>
#define LVGL_TICK_PERIOD 10
Ticker tick;
static void lv_tick_handler(void) {
  lv_tick_inc(LVGL_TICK_PERIOD);
}

#if defined(SCREEN_MODEL_ER_50)
  #include <GSL1680.h>
  #define CTP_WAKE        14
  #define CTP_INTRPT      32
  GSL1680 TS = GSL1680();

  #define INNER_CIRCLE_DIA  230
  #define OUTER_CIRCLE_DIA  260
  #define BUTTON_GROWTH     120
  #define CENTER_OFFSET     -48
  #define STATUS_LABEL_Y    120
  #define STATUS_LABEL_FONT &rubik_36
  #define CHEVRON_FONT      &rubik_22
  #define CHEVRON_SPACING   40
  #define MODE_BUTTON_X     290
  #define SMALL_HOME_BTN    90

#elif defined(SCREEN_MODEL_ER_43)
  #include <FT5206.h>
  #define CTP_INT         33
  uint8_t registers[FT5206_REGISTERS];
  uint16_t new_coordinates[5][2];
  uint8_t current_touches = 0;
  FT5206 TS = FT5206(CTP_INT);

  #define INNER_CIRCLE_DIA  130
  #define OUTER_CIRCLE_DIA  146
  #define BUTTON_GROWTH     68
  #define CENTER_OFFSET     -27
  #define STATUS_LABEL_Y    68
  #define STATUS_LABEL_FONT &rubik_22
  #define CHEVRON_FONT      &rubik_16
  #define CHEVRON_SPACING   20
  #define CORE_ICON_WIDTH   50
  #define CORE_ICON_HEIGHT  62
  #define MODE_BUTTON_X     174
  #define SMALL_HOME_BTN    51
#endif



#include "XGLCD.h"
//set up the display
XGLCD _tft = XGLCD();

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//how long the screen stays on for (ms)
#define SCREEN_TIMEOUT 30000
bool screenState = false; //is the display on or off

//int screenBrightness = 1023; //curent backlight level

//our default colors
const lv_color_t AbodeBlue = lv_color_hex(0x3BB3CC);
const lv_color_t AbodeBlueLight = lv_color_hex(0x87EBFF);
const lv_color_t AbodeGreen = lv_color_hex(0x44D8BA);
const lv_color_t AbodeRed = lv_color_hex(0xDB6654);

const int freq = 5000;
const int rledChannel = 10;
const int gledChannel = 11;
const int bledChannel = 12;
const int resolution = 8;
#define RED_LED       A1
#define BLUE_LED      A0
//#define GREEN_LED     A2

struct homepage {
  lv_obj_t * outerCircle = NULL;
  lv_obj_t * innerCircle = NULL;
  lv_obj_t * image = NULL;
  lv_obj_t * status = NULL;
  lv_obj_t * chevrons[6];
  lv_obj_t * leftButton = NULL;
  lv_obj_t * rightButton = NULL;
  lv_obj_t * countdown = NULL;
  bool countdownReady = false;
  bool isShrunk = false;
  short chevronAnim = -1;
};
homepage hp;


//return code for the loop function telling the caller what needs to happen (see REQEUST_ defines in screen.h)
short requestCode = REQUEST_IDLE;

/* Display flushing function for lvgl/tft_eSPI*/
void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  _tft.drawBitmap(area->x1, area->y1, area->x2, area->y2, (uint16_t *)color_p);                    // copy 'color_array' to the specifed coordinates
  lv_disp_flush_ready(disp);
}

void my_disp_map(int32_t x1,int32_t y1,int32_t x2,int32_t y2, const lv_color_t* color_p) {
  _tft.drawBitmap(x1, y1, x2, y2, (uint16_t *)color_p);                        // Copy 'color_p' to the specified area
}


//touch read. this function also handles turning on the display and ignoring touches for that action
int lastTouch = 0;
int firstTouch = 0;

#if defined(SCREEN_MODEL_ER_50)
bool touchRead(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
  data->state = LV_INDEV_STATE_REL;
  int NBFinger = TS.dataread();
  if (NBFinger == 0) {
    return false;
  } else {
    //if the screen is off, turn it off and ignore the touch
    
    if (screenState == false) {
      lastTouch = millis();
      firstTouch = millis();
      screenState = true;
      Serial.println("Waking");
      _tft.displayOn(true);
      _tft.backlight(true);
      return false;
    } else {
      if (millis() < firstTouch + 250) {
        Serial.println("Skipping");
        return false;
      }
    }
    lastTouch = millis();
    
    data->point.x = TS.readFingerX(0);
    data->point.y = TS.readFingerY(0);
    data->state = LV_INDEV_STATE_PR;
  }
  return false;
}
#elif defined(SCREEN_MODEL_ER_43)
bool touchRead(lv_indev_drv_t * indev, lv_indev_data_t *data) {
  if (TS.touched()){
    TS.getTSregisters(registers);
    current_touches = TS.getTScoordinates(new_coordinates, registers);
    if (current_touches < 1) {
      data->state = LV_INDEV_STATE_REL;
      return false;
    }

    if (screenState == false) {
      lastTouch = millis();
      firstTouch = millis();
      screenState = true;
      Serial.println("Waking");
      _tft.displayOn(true);
      _tft.backlight(true);
      return false;
    } else {
      if (millis() < firstTouch + 250) {
        Serial.println("Skipping");
        return false;
      }
    }
    lastTouch = millis();
    
    data->point.x = new_coordinates[0][0];
    data->point.y = new_coordinates[0][1];
    data->state = LV_INDEV_STATE_PR;
  }
  else
    data->state = LV_INDEV_STATE_REL;
  return false;
}
#endif

//super bright
void Screen::setBrightness(short br) {
  //screenBrightness = br;
  //_tft.brightness(br);
}

//off completely
void Screen::off() { 
  screenState = false;
  _tft.displayOn(false);
  _tft.backlight(false);
}

//on to the current level
void Screen::wake() {
  screenState = true;
  lastTouch = millis();
  _tft.displayOn(true);
  _tft.backlight(true);
}

//set up the display
void Screen::begin() {

  //init lvgl
  lv_init();

  //start display
  _tft.begin();     
  delay(300);
  
  //TS.begin(CTP_WAKE, CTP_INTRPT);
  TS.begin();
  TS.setTouchLimit(1);
  
  //Screen::off();

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = _tft.width();
  disp_drv.ver_res = _tft.height();
  disp_drv.flush_cb = displayFlush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchRead;
  lv_indev_drv_register(&indev_drv);

  //set the theme
  lv_theme_t * th = lv_theme_material_init(AbodeBlue, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, &rubik_16, &rubik_16, &rubik_22, &rubik_36);     
  lv_theme_set_act(th);

  //attach the tick handler to the timer
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

  //ledcSetup(bledChannel, freq, resolution);
  //ledcAttachPin(BLUE_LED, bledChannel);
  //ledcWrite(bledChannel, 0);
}

// a simple loading screen
void Screen::loading() {
  lv_obj_t * scr = lv_cont_create(NULL, NULL);
  lv_obj_set_style_local_bg_color(scr, LV_OBJ_PART_MAIN, 0, AbodeBlue);

  lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 250, 500, true);
}

//resizes a button, keeping the center of the button anchored
void resizeButton(lv_obj_t *obj, short growBy) {
  lv_anim_path_t path;
  lv_anim_path_init(&path);
  lv_anim_path_set_cb(&path, lv_anim_path_ease_in_out);

  lv_coord_t w = lv_obj_get_width(obj);
  lv_coord_t h = lv_obj_get_height(obj);
  
  lv_coord_t x = lv_obj_get_x(obj);
  lv_coord_t y = lv_obj_get_y(obj);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_width);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 125);
  lv_anim_set_values(&a, w, w + growBy);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_height);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 125);
  lv_anim_set_values(&a, h, h + growBy);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  //now get the x/y coordiantes from the obj (remember, it was just clicked so it's obj) and then remove 1/2 of the amount it's being shrunk by (should be 15)
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 125);
  lv_anim_set_values(&a, x, x - (growBy / 2));
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 125);
  lv_anim_set_values(&a, y, y - (growBy / 2));
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);
  
}

void hideModeSelect(bool resetButton = true) {
  if (resetButton) {
    resizeButton(hp.innerCircle, BUTTON_GROWTH);
    lv_obj_fade_in(hp.image, 125, 0);
    lv_obj_fade_in(hp.outerCircle, 125, 0);
    lv_obj_set_hidden(hp.outerCircle, false);
    lv_obj_set_hidden(hp.image, false);
  }
  lv_obj_del(hp.leftButton);
  hp.leftButton = NULL;
  lv_obj_del(hp.rightButton);
  hp.rightButton = NULL;
  for (int i = 0; i < 6; i++) {
    //remove chevrons
    lv_obj_del(hp.chevrons[i]);
    hp.chevrons[i] = NULL;
  }
  hp.isShrunk = false;

  lv_obj_set_drag(hp.innerCircle, false);
}

void buttonFlyToMain(lv_obj_t *obj) {

  lv_coord_t end_x = (_tft.width() / 2) - (INNER_CIRCLE_DIA / 2);
  lv_coord_t end_y = ((_tft.height() / 2) - (INNER_CIRCLE_DIA / 2) + CENTER_OFFSET);

  lv_anim_path_t path;
  lv_anim_path_init(&path);
  lv_anim_path_set_cb(&path, lv_anim_path_ease_in_out);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_width);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 250);
  lv_anim_set_values(&a, lv_obj_get_width(obj), INNER_CIRCLE_DIA);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_height);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 250);
  lv_anim_set_values(&a, lv_obj_get_height(obj), INNER_CIRCLE_DIA);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 250);
  lv_anim_set_values(&a, lv_obj_get_x(obj), end_x);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
  lv_anim_set_var(&a, obj); 
  lv_anim_set_time(&a, 250);
  lv_anim_set_values(&a, lv_obj_get_y(obj), end_y);
  lv_anim_set_path(&a, &path);
  lv_anim_start(&a);

  hideModeSelect(false);
}

//used for the nice chevrons on the home screen
static void anim_opacity_cb(lv_obj_t * obj, lv_anim_value_t v) {
  lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
};

void showModeSelect(lv_obj_t * obj, lv_event_t event) {
	if (event == LV_EVENT_CLICKED) {
    if (!hp.isShrunk) {
      hp.isShrunk = true;

      //resize the main circle
      int grow_by = 0 - BUTTON_GROWTH;
      resizeButton(obj, grow_by);

      //fade out the other circles
      lv_obj_fade_out(hp.image, 125, 0);
      lv_obj_fade_out(hp.outerCircle, 125, 0);

      //set the main button to be draggable
      lv_obj_set_drag(obj, true);
      lv_obj_set_drag_dir(obj, LV_DRAG_DIR_BOTH);

      lv_obj_t * scr = lv_scr_act();

      int start = (INNER_CIRCLE_DIA - BUTTON_GROWTH) / 2 + 45;
      int x = 0 - start - (CHEVRON_SPACING * 2); // negative start (left of center) with space for two chevrons
      int d = 375; //delays
      for (int i = 0; i < 6; i++) {
        //select correct arrow
        const char* text = "<";
        if (i > 2) { text = ">"; }

        //create the arrow
        hp.chevrons[i] = lv_label_create(scr, NULL);
        lv_obj_set_style_local_opa_scale(hp.chevrons[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_text_font(hp.chevrons[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, CHEVRON_FONT);
        lv_label_set_text(hp.chevrons[i], text);
        lv_obj_align(hp.chevrons[i], scr, LV_ALIGN_CENTER, x, CENTER_OFFSET);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) anim_opacity_cb);
        lv_anim_set_var(&a, hp.chevrons[i]);
        lv_anim_set_time(&a, 125);
        lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_delay(&a, d);
        lv_anim_set_playback_time(&a, 125);
        lv_anim_set_playback_delay(&a, 500);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, 500);
        lv_anim_start(&a);

        //select the next "x" coordinate
        x = x + CHEVRON_SPACING;
        if (i > 2) {
          d = d + 125;
        } else if (i < 2) {
          d = d - 125;
        } else {
          x = start;
        }
      }

      //style for a 30% opaque circle (white)
      static lv_style_t outer_circle_style;
      lv_style_init(&outer_circle_style);
      //lv_style_set_bg_color(&outer_circle_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_style_set_radius(&outer_circle_style, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
      lv_style_set_border_width(&outer_circle_style, LV_STATE_DEFAULT, 0);

      //draw the outer circle
      hp.leftButton = lv_obj_create(scr, NULL);
      lv_obj_add_style(hp.leftButton, LV_OBJ_PART_MAIN, &outer_circle_style);
      lv_obj_set_style_local_bg_color(hp.leftButton, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, AbodeGreen);
      lv_obj_set_size(hp.leftButton, SMALL_HOME_BTN, SMALL_HOME_BTN);
      lv_obj_align(hp.leftButton, scr, LV_ALIGN_CENTER, -MODE_BUTTON_X, CENTER_OFFSET);
      //insert the image
      lv_obj_t * img = lv_img_create(hp.leftButton, NULL);
      lv_img_set_src(img, ICON_HOME_SM_WH);
      lv_obj_align(img, hp.leftButton, LV_ALIGN_CENTER, 0, 0);

      //draw the outer circle
      hp.rightButton = lv_obj_create(scr, NULL);
      lv_obj_add_style(hp.rightButton, LV_OBJ_PART_MAIN, &outer_circle_style);
      lv_obj_set_style_local_bg_color(hp.rightButton, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, AbodeRed);
      lv_obj_set_size(hp.rightButton, SMALL_HOME_BTN, SMALL_HOME_BTN);
      lv_obj_align(hp.rightButton, scr, LV_ALIGN_CENTER, MODE_BUTTON_X, CENTER_OFFSET);
      //insert the image
      img = lv_img_create(hp.rightButton, NULL);
      lv_img_set_src(img, ICON_AWAY_SM_WH);
      lv_obj_align(img, hp.rightButton, LV_ALIGN_CENTER, 0, 0);

    }
  } else if (event == LV_EVENT_DRAG_END) {
    short x = lv_obj_get_x(obj);
    //short y = lv_obj_get_y(obj);
    //lv_coord_t ly = lv_obj_get_y(hp.leftButton);
    //lv_coord_t ry = lv_obj_get_y(hp.rightButton);
    
    lv_coord_t lx = lv_obj_get_x(hp.leftButton);
    lv_coord_t rx = lv_obj_get_x(hp.rightButton);

    if (x > (lx - 45) && x < (lx + 45)) {
      Serial.println("Left button");
      //requestCode = REQUEST_MODE_HOME;
      lv_obj_t * tmp = hp.innerCircle;
      hp.innerCircle = hp.leftButton;
      hp.leftButton = tmp;
      lv_label_set_text(hp.status, "Arming");
      lv_obj_del(lv_obj_get_child(hp.innerCircle, NULL)); //remove the image
      lv_obj_align(hp.status, NULL, LV_ALIGN_CENTER, 0, STATUS_LABEL_Y);
      hp.countdown = lv_label_create(lv_scr_act(), hp.status);
      lv_label_set_text(hp.countdown, "");
      lv_obj_align(hp.countdown, lv_scr_act(), LV_ALIGN_CENTER, 0, CENTER_OFFSET);// <!--- needs to be updated with every 'tick'
      lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, AbodeGreen);
      lv_obj_set_style_local_bg_color(hp.innerCircle, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      hp.countdownReady = true;
      buttonFlyToMain(hp.innerCircle);
    } else if (x > (rx - 45) && x < (rx + 45)) {
      Serial.println("Right button");
      //requestCode = REQUEST_MODE_AWAY;
      lv_obj_t * tmp = hp.innerCircle;
      hp.innerCircle = hp.rightButton;
      hp.rightButton = tmp;
      lv_label_set_text(hp.status, "Arming");
      lv_obj_del(lv_obj_get_child(hp.innerCircle, NULL)); //remove the image
      lv_obj_align(hp.status, NULL, LV_ALIGN_CENTER, 0, STATUS_LABEL_Y);
      buttonFlyToMain(hp.innerCircle);
    } else {
      lv_obj_align(obj, lv_scr_act(), LV_ALIGN_CENTER, 0, CENTER_OFFSET);
      hideModeSelect();
    }
  }
}
void Screen::home() {
  //need to check if we're already on the home page. if so, we should just change the background color and text and be done

  //style the screen with the correct background and text color
  static lv_style_t home_page_style;
  lv_style_init(&home_page_style);
  if (_mode == ABODE_MODE_STANDBY) {
    lv_style_set_bg_color(&home_page_style, LV_STATE_DEFAULT, AbodeBlue);
  } else if(_mode == ABODE_MODE_HOME || _mode == ABODE_MODE_AWAY) {
    lv_style_set_bg_color(&home_page_style, LV_STATE_DEFAULT, AbodeGreen);
  } else {
    lv_style_set_bg_color(&home_page_style, LV_STATE_DEFAULT, AbodeRed);
  }
  lv_style_set_text_color(&home_page_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  lv_obj_t * scr = lv_obj_create(NULL, NULL);
  lv_obj_add_style(scr, LV_OBJ_PART_MAIN, &home_page_style);

  //time
  lv_obj_t * label = lv_label_create(scr, NULL);
  lv_label_set_text(label, "22:56\nOctober 21st");
  lv_obj_align(label, scr, LV_ALIGN_IN_TOP_LEFT, 8, 6);

  //temperature style
  static lv_style_t weather_style;
  lv_style_init(&weather_style);
  lv_style_set_text_color(&weather_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_opa(&weather_style, LV_STATE_DEFAULT, LV_OPA_80);

  label = lv_label_create(scr, NULL);
  lv_obj_add_style(label, LV_OBJ_PART_MAIN, &weather_style);
  lv_label_set_text(label, "64" DEGREES_SYMBOL "F");
  lv_obj_align(label, scr, LV_ALIGN_IN_TOP_LEFT, 8, 50);

  //style for a 30% opaque circle (white)
  static lv_style_t outer_circle_style;
  lv_style_init(&outer_circle_style);
  lv_style_set_bg_color(&outer_circle_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_bg_opa(&outer_circle_style, LV_STATE_DEFAULT, LV_OPA_40);
  lv_style_set_radius(&outer_circle_style, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_border_width(&outer_circle_style, LV_STATE_DEFAULT, 0);

  //draw the outer circle
  lv_obj_t * circle = lv_obj_create(scr, NULL);
  lv_obj_add_style(circle, LV_OBJ_PART_MAIN, &outer_circle_style);
  lv_obj_set_size(circle, OUTER_CIRCLE_DIA, OUTER_CIRCLE_DIA);
  lv_obj_align(circle, scr, LV_ALIGN_CENTER, 0, CENTER_OFFSET);

  //use a modified style for the inner circle (opaque)
  static lv_style_t inner_circle_style;
  lv_style_copy(&inner_circle_style, &outer_circle_style);
  lv_style_set_bg_opa(&inner_circle_style, LV_STATE_DEFAULT, LV_OPA_100);
  hp.outerCircle = circle;

  //draw the inner circle
  circle = lv_obj_create(scr, NULL);
  lv_obj_add_style(circle, LV_OBJ_PART_MAIN, &inner_circle_style);
  lv_obj_set_size(circle, INNER_CIRCLE_DIA, INNER_CIRCLE_DIA);
  lv_obj_align(circle, scr, LV_ALIGN_CENTER, 0, CENTER_OFFSET);
  lv_obj_set_event_cb(circle, showModeSelect);
  hp.innerCircle = circle;

  //insert the image
  lv_obj_t * img = lv_img_create(scr, NULL);
  if (_mode == ABODE_MODE_STANDBY) {
    lv_img_set_src(img, ICON_STANDBY);
  } else if(_mode == ABODE_MODE_HOME || _mode == ABODE_MODE_AWAY) {
    lv_img_set_src(img, ICON_HOME);
  }
  lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, CENTER_OFFSET);
  hp.image = img;

  //status label style
  static lv_style_t state_label;
  lv_style_init(&state_label);
  lv_style_set_text_color(&state_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&state_label, LV_STATE_DEFAULT, STATUS_LABEL_FONT);

  //lazy way of capitalizing first char (can't believe String doesn't support chaining...)
  String modeString = _mode.substring(0, 1);
  modeString.toUpperCase();
  modeString.concat(_mode.substring(1));

  //status label
  label = lv_label_create(scr, NULL);
  lv_obj_add_style(label, LV_OBJ_PART_MAIN, &state_label);
  lv_label_set_text(label, modeString.c_str());
  lv_obj_align(label, scr, LV_ALIGN_CENTER, 0, STATUS_LABEL_Y);
  hp.status = label;

  //bottom bar style
  static lv_style_t bottom_bar_style;
  lv_style_init(&bottom_bar_style);
  lv_style_set_bg_color(&bottom_bar_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_radius(&bottom_bar_style, LV_STATE_DEFAULT, 0);
  lv_style_set_border_width(&bottom_bar_style, LV_STATE_DEFAULT, 0);

  //draw the bottom bar
  lv_obj_t * bar = lv_obj_create(scr, NULL);
  lv_obj_add_style(bar, LV_OBJ_PART_MAIN, &bottom_bar_style);
  lv_obj_set_size(bar, _tft.width(), round(_tft.height() * .1));
  lv_obj_align(bar, scr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 250, 500, true);
}

void Screen::setState(const char * state) {
  _state = (String)state;
  Serial.print("State::");
  Serial.println(_state);
}

void Screen::setMode(String mode) {
  _mode = mode;
  Screen::wake();
  Serial.print("Mode::");
  Serial.println(_mode);
  hp.countdownReady = false;
  //if (mode == ABODE_MODE_STANDBY) {
  //  ledcWrite(bledChannel, 255);
  //}
}

void Screen::startTimer(String mode, int seconds) {
  if(hp.countdownReady == true) {
    char buffer [10];
    itoa(seconds, buffer, 10);
    lv_label_set_text(hp.countdown, buffer);
    lv_obj_align(hp.countdown, lv_scr_act(), LV_ALIGN_CENTER, 0, CENTER_OFFSET);// <!--- needs to be updated with every 'tick'
  }
}

void Screen::updateTimer(int seconds) {
  if(hp.countdownReady == true) {
    char buffer [10];
    itoa(seconds, buffer, 10);
    lv_label_set_text(hp.countdown, buffer);
    lv_obj_align(hp.countdown, lv_scr_act(), LV_ALIGN_CENTER, 0, CENTER_OFFSET);// <!--- needs to be updated with every 'tick'
  }
}

//int lastBrightnessChange = 0;
//short lastBrightness = 0;
//#define BRIGHTNESS_INTERVAL 3000
short Screen::loop() {
  int m = millis();
  lv_task_handler();

  if (screenState && m > lastTouch + SCREEN_TIMEOUT) {
    Screen::off();
  }

  if(hp.isShrunk && m > (lastTouch + 3000)) {
    hideModeSelect();
  }

/*
  short lightness = map(analogRead(LIGHT_SENSOR_PIN), 0, 4095, 1, 255);
  if ((lightness > (lastBrightness + 20) || lightness < (lastBrightness - 20)) && (m > lastBrightnessChange + BRIGHTNESS_INTERVAL)) {
    _tft.brightness(lightness); /// <!--- TODO: Smooth brightness change
    //if(_mode == ABODE_MODE_STANDBY) {
    //  Serial.println(lightness);
    //  ledcWrite(bledChannel, lightness);
    //}
    //lastBrightnessChange = m;
    //lastBrightness = lightness;
  }
  */

  //now return any requests to the main loop
  short retCode = requestCode;
  requestCode = REQUEST_IDLE;
  return retCode;
}


    /*
    static const char * btns[] = {"Standby", "Home", "Away", ""};

    static lv_style_t modalBgStyle;
    lv_style_init(&modalBgStyle);
    lv_style_set_bg_opa(&modalBgStyle, LV_STATE_DEFAULT, LV_OPA_20);
    lv_style_set_bg_color(&modalBgStyle, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_radius(&modalBgStyle, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&modalBgStyle, LV_STATE_DEFAULT, 0);

    static lv_style_t modalStyle;
    lv_style_init(&modalStyle);
    lv_style_set_radius(&modalStyle, LV_STATE_DEFAULT, 3);
    lv_style_set_border_width(&modalStyle, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&modalStyle, LV_STATE_DEFAULT, AbodeBlue);

    static lv_style_t modalButtons;
    lv_style_init(&modalButtons);
    lv_style_set_radius(&modalButtons, LV_STATE_DEFAULT, 3);
    lv_style_set_border_width(&modalButtons, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&modalButtons, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_color(&modalButtons, LV_STATE_DEFAULT, AbodeBlue);

    lv_obj_t *mboxContainer = lv_cont_create(lv_layer_top(), NULL);
    lv_obj_add_style(mboxContainer, LV_OBJ_PART_MAIN, &modalBgStyle);
    lv_obj_set_size(mboxContainer, LV_HOR_RES, LV_VER_RES);
    lv_cont_set_fit(mboxContainer, LV_FIT_NONE);
    
    mbox = lv_msgbox_create(mboxContainer, NULL);
    lv_msgbox_set_text(mbox, "Select mode");
    lv_msgbox_add_btns(mbox, btns);
    lv_obj_add_style(mbox, LV_OBJ_PART_MAIN, &modalStyle);
    lv_obj_add_style(mbox, LV_MSGBOX_PART_BTN, &modalButtons);
    lv_obj_set_width(mbox, 320);
    lv_obj_set_event_cb(mbox, selectMode);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_shadow_opa(mbox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    */
   /*

lv_obj_t *mbox;
void selectMode(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_DELETE && obj == mbox) {
    Serial.println("deleting msgbox");
		lv_obj_del_async(lv_obj_get_parent(obj));
//		mboxContainer = NULL; 
	} else if(event == LV_EVENT_VALUE_CHANGED) {
    String newMode = lv_msgbox_get_active_btn_text(obj);
    if (newMode == "Standby") {
      requestCode = REQUEST_MODE_STANDBY;
    } else if (newMode = "Home") {
      requestCode = REQUEST_MODE_HOME;
    } else {
      requestCode = REQUEST_MODE_AWAY;
    }
    lv_msgbox_start_auto_close(mbox, 0);
  }  
}
*/