#include <Arduino.h>
#include "lv_xg.h"
#include "lvgl.h"
#include <Ticker.h>

#define LVGL_TICK_PERIOD 20
Ticker tick;
static void lv_tick_handler(void) {
  lv_tick_inc(LVGL_TICK_PERIOD);
}

void setup() {
  Serial.begin(9600);

  lv_xg_init();
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
  
  lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);
  lv_theme_set_act(th);

  lv_obj_t * scr = lv_cont_create(NULL, NULL);
  lv_disp_load_scr(scr);

  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Arduino! (V7.0)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -50);
}

void loop() {
  lv_task_handler();
  delay(5);
}

