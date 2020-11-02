#include <Arduino.h>

#include "settings.h"

#include "screen.h"
Screen screenManager;

#include "configure.h"
Configure configure;

#include "abode.h"
Abode abode;

bool _running = false;
bool _gotCredentials = false;

int32_t exitTimer = 0;

bool setNetwork() {
  WiFi.mode(WIFI_STA);
  NetSettings net = AbodeSettings.getNetwork();
  Serial.println(net.ssid);
  Serial.println(net.pass);
  WiFi.begin(net.ssid.c_str(), net.pass.c_str());

  const short timeout = 12000;
  int connect_timer = millis() + timeout;
  while (WiFi.status() != WL_CONNECTED && millis() < connect_timer) {  }

  if(WiFi.status() == WL_CONNECTED) {
    return true;
  }
  return false;
}

//function to handle gateway mode change
void statusChange(const char * payload, size_t length) {
  exitTimer = 0;
  screenManager.setMode(payload);
  screenManager.home(); // <------------- needs to be more graceful, is the user on the homepage?
}

//show when connection happens
void connectHandler(const char* payload, size_t length) {
  Serial.println("Connected to Abode cloud");
  screenManager.setMode(abode.getMode());
  //fire the status change function
  screenManager.setState(ABODE_CONNECT);
}

//show when disconnected
void disconnectHandler(const char* payload, size_t length) {
  Serial.println("Disconnected from Abode cloud");
  screenManager.setMode("");
  screenManager.setState(ABODE_DISCONNECT);
  //abode.begin(login_id, login_pass);
}

//a change to the timeline
void timelineChange(const char* payload, size_t length) {
  JSONVar data = JSON.parse(payload);
  if (data.hasOwnProperty("event_code")) {
    const char* temp = (const char*) data["event_code"];
    int temp2 = atoi(temp);
    Serial.println(temp2);
    switch(atoi(data["event_code"])) {
      case ABODE_EVENT_ARMING:
        Serial.println(data);
        String eventName = (const char*) data["event_name"];
        if (eventName.indexOf("Home") > 0) {
          exitTimer = millis() + (abode.getExit(ABODE_MODE_HOME)) * 1000;
        } else if (eventName.indexOf("Away") > 0) {
          exitTimer = millis() + (abode.getExit(ABODE_MODE_AWAY)) * 1000;
        }
        break;
    }
  }
}

//print any payload (Abode event)
void printPayload(const char* payload, size_t length) {
  Serial.print("Status changed::");
  Serial.println(payload);
}

bool configureAbode() {
  AbodeLogin login = AbodeSettings.getAbodeLogin();
  bool state = abode.begin(login.email, login.pass);
  
  if (state) {
    Serial.println("Setup up Abode callbacks");
    abode.on(ABODE_CONNECT, connectHandler);
    abode.on(ABODE_DISCONNECT, disconnectHandler);
    abode.on(ABODE_MODE, statusChange);
    abode.on(ABODE_TIMELINE, timelineChange);
    abode.on(ABODE_UPDATE, printPayload);
  }
  
  screenManager.setMode(abode.getMode());
  return state;
}

void setup() {

  //ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  //ledcAttachPin(32, 10);
  //analogReadResolution(10);
  //ledcWrite(10,768);

  Serial.begin(9600); /* prepare for possible serial debug */

  screenManager.begin();
  //screenManager.loading();

  if(!AbodeSettings.load()) {
    configure.welcome();
  } else {
    _gotCredentials = true;
  }
/*
    if (setNetwork()) {
      if (configureAbode()) {
        Serial.println("Logged in!");
        screenManager.home();
        screenManager.setDay();
        screenManager.wake();
      } else {
        Serial.println("Couldn't log in");
      }
    } else {
      Serial.println("Couldn't join network");
    }
  }
*/
}

void loop() {
  //we need to include the loop for the Abode class
  abode.loop();

  switch (screenManager.loop()) {
    case REQUEST_MODE_STANDBY:
      abode.setMode(ABODE_MODE_STANDBY);
      break;
    case REQUEST_MODE_HOME:
      abode.setMode(ABODE_MODE_HOME);
      break;
    case REQUEST_MODE_AWAY:
      abode.setMode(ABODE_MODE_AWAY);
      break;
  }
  
  switch (configure.loop()) {
    case CONFIGURE_WIFI:
      Serial.println("Connected to Wifi");
      Serial.println(AbodeSettings.getNetwork().ssid);
      if (!_running) {
        //now we need to get them to sign into Abode
        configure.abode();
      }
      break;
    case CONFIGURE_ABODE:
      Serial.println("Got Abode Credentials");
      //now let's try to login!
      bool loginSuccess = configureAbode();
      if (loginSuccess) {
        _running = true;
        AbodeSettings.save();
        screenManager.home();
      } else {
        configure.abode();
      }
      Serial.println(loginSuccess);
      break;
  }
  

  if (_running == false && _gotCredentials == true) {
    if (WiFi.isConnected() == false) {
      //connect to wifi
      Serial.print("Connecting to wifi...");
      if (setNetwork()) {
        Serial.println("done.");
      } else {
        Serial.println("couldn't connect.");
      }
    }
    if (abode.isConnected() == false) {
      //connect to Abode
      Serial.print("Connecting to Abode Cloud...");
      if (configureAbode()) {
        Serial.println("done.");
        _running = true;
        screenManager.home();
        screenManager.setDay();
        screenManager.wake();
      } else {
        Serial.println("couldn't connect.");
      }
    }
  }
}






/*
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 

#include "lv_xg.h"
#include "lvgl.h"
#include <Ticker.h>
#define LVGL_TICK_PERIOD 20
Ticker tick;
static void lv_tick_handler(void) {
  lv_tick_inc(LVGL_TICK_PERIOD);
}
void testcb(_lv_obj_t * obj, lv_event_t event) {
  Serial.println("Clicked!");
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

  lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, 0);
  label = lv_label_create(btn, NULL);
  lv_label_set_text(label, "Button!");
  lv_obj_set_event_cb(btn, testcb);

}

void loop() {

  lv_task_handler();
  delay(5);

}

*/
