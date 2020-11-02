#include "configure.h"
#include "settings.h"

const int ELEMENT_COUNT = 10;
String storage_array[ELEMENT_COUNT];
Vector<String> netlist(storage_array);

lv_obj_t* wifiContainer;

TaskHandle_t WiFiTask;

String _netssid;
String _netpass;

short _wifi_connecting = CONNECT_IDLE;

void getWiFiNetworks(void * param) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
        netlist.push_back(WiFi.SSID(i));
    }
    vTaskDelete(NULL);
  } else {
    vTaskDelete(NULL);
  }
}

void setNetwork(void * param) {
  _wifi_connecting = CONNECT_CONNECTING;
  WiFi.mode(WIFI_STA);
  WiFi.begin(_netssid.c_str(), _netpass.c_str());

  const short timeout = 12000;
  int connect_timer = millis() + timeout;
  while (WiFi.status() != WL_CONNECTED && millis() < connect_timer) {  }

  if(WiFi.status() == WL_CONNECTED) {
    _wifi_connecting = CONNECT_SUCCESS;
  } else {
    _netssid = "";
    _netpass = "";
    _wifi_connecting = CONNECT_FAILED;
  }
  

  vTaskDelete(NULL);
  //now we need to check to see if we can access my.goabode.com
}

void loadWifi() {
  lv_obj_t * scr = lv_obj_create(NULL, NULL);

  lv_obj_t *label = lv_label_create(scr, NULL);
  lv_label_set_text(label, "Select WiFi Network");
  lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

  label = lv_label_create(scr, NULL);
  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label, 400);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(label, "To get started, select your wireless network from the list below");
  lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 60);

  lv_obj_t * cont;
  cont = lv_cont_create(scr, NULL);
  int y = 110;
  int height = 320 - y;
  lv_obj_set_pos(cont, 0, y);
  lv_obj_set_size(cont, 480, height);

  lv_obj_t * preload = lv_spinner_create(cont, NULL);
  lv_obj_set_size(preload, 80, 80);
  lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_spinner_set_type(preload, LV_SPINNER_TYPE_CONSTANT_ARC);

  lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 250, 500, false);
  wifiContainer = cont;
  xTaskCreate(getWiFiNetworks, "wifiTask", 10000, NULL, 0, &WiFiTask);
}
void loadWifi(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    loadWifi();
  }
}

void Configure::wifiSelect(void) {
  loadWifi();
}

void Configure::welcome(void) {

  lv_obj_t * scr1 = lv_obj_create(NULL, NULL);
  //lv_obj_set_style_local_bg_color(scr1, LV_OBJ_PART_MAIN, 0, primary_color);

  lv_obj_t *label = lv_label_create(scr1, NULL);
  lv_label_set_text(label, "Welcome!");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -85);

  label = lv_label_create(scr1, NULL);
  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label, 400);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(label, "This device can control your Abode alarm system, we just need to complete a few setup steps to get started");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -20);

  lv_obj_t * btn1 = lv_btn_create(scr1, NULL);
  lv_obj_set_event_cb(btn1, loadWifi);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, 100);
  label = lv_label_create(btn1, NULL);
  lv_label_set_text(label, "Get Started");

  lv_scr_load_anim(scr1, LV_SCR_LOAD_ANIM_NONE, 250, 500, true);
}

void joinNetwork(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    lv_obj_t * scr = lv_obj_create(NULL, NULL);

    lv_obj_t *label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Joining network");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    label = lv_label_create(scr, NULL);
    lv_label_set_text(label, _netssid.c_str());
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);

    lv_obj_t * preload = lv_spinner_create(scr, NULL);
    lv_obj_set_size(preload, 100, 100);
    lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 20);
    lv_spinner_set_type(preload, LV_SPINNER_TYPE_CONSTANT_ARC);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 500, true);

    xTaskCreate(setNetwork, "wifiTask", 10000, NULL, 0, &WiFiTask);
  }
}

void savePass(lv_obj_t * obj, lv_event_t event) {
  if(event == LV_EVENT_VALUE_CHANGED) {
    _netpass = (String)lv_textarea_get_text(obj);
  }
}

void enterKey(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    lv_obj_t * label = lv_obj_get_child(obj, NULL);
    _netssid = (String)lv_label_get_text(label);

    lv_obj_t * scr = lv_obj_create(NULL, NULL);

    label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Join network");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    String txt = "Enter network key for " + _netssid;
    label = lv_label_create(scr, NULL);
    lv_label_set_text(label, txt.c_str());
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 35);

    lv_obj_t * ta = lv_textarea_create(scr, NULL);
    lv_obj_set_width(ta, 240);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_pwd_mode(ta, true);
    lv_textarea_set_text(ta, "InThePNW1234");
    lv_textarea_set_cursor_hidden(ta, true);
    lv_textarea_set_placeholder_text(ta, "Password");
    lv_obj_align(ta, NULL, LV_ALIGN_IN_TOP_MID, 0, 65);
    lv_obj_set_event_cb(ta, savePass);

    lv_obj_t *kb = lv_keyboard_create(scr, NULL);
    lv_keyboard_set_cursor_manage(kb, true);
    lv_keyboard_set_textarea(kb, ta);

    lv_obj_t * btn = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb(btn, loadWifi);
    lv_obj_align(btn, kb, LV_ALIGN_OUT_TOP_LEFT, 8, -8);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, LV_SYMBOL_LEFT " Back");

    btn = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb(btn, joinNetwork);
    lv_obj_align(btn, kb, LV_ALIGN_OUT_TOP_RIGHT, -8, -8);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Join");

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 500, true);
  }
}

String _abodeEmail = "";
String _abodePass = "";
lv_obj_t * ta_aemail;
lv_obj_t * ta_apass;
bool credentialsComplete = false;

void saveCredentials(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    Serial.println("Saving credentials");
    _abodePass = (String)lv_textarea_get_text(ta_apass);
    credentialsComplete = true;
  }
}

void abodePassword(lv_obj_t * obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    Serial.println("Loading Password Page");
    _abodeEmail = (String)lv_textarea_get_text(ta_aemail);
    lv_obj_t * scr = lv_obj_create(NULL, NULL);

    lv_obj_t * label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Sign in to Abode");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    ta_apass = lv_textarea_create(scr, NULL);
    lv_obj_set_width(ta_apass, 240);
    lv_textarea_set_one_line(ta_apass, true);
    lv_textarea_set_pwd_mode(ta_apass, true);
    lv_textarea_set_text(ta_apass, "qqirgZY5iqtNWt8");
    lv_textarea_set_cursor_hidden(ta_apass, true);
    lv_textarea_set_placeholder_text(ta_apass, "Password");
    lv_obj_align(ta_apass, NULL, LV_ALIGN_IN_TOP_MID, 0, 70);
    //lv_obj_set_event_cb(ta, saveAPass);

    lv_obj_t * kb = lv_keyboard_create(scr, NULL);
    lv_keyboard_set_cursor_manage(kb, true);
    lv_keyboard_set_textarea(kb, ta_apass);

    lv_obj_t * btn = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb(btn, saveCredentials);
    lv_obj_align(btn, kb, LV_ALIGN_OUT_TOP_RIGHT, -8, -8);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Sign in");

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 500, true);
  }
}

void abodeEmail() {
    Serial.println("Loading Email Page");
    lv_obj_t * scr = lv_obj_create(NULL, NULL);

    lv_obj_t * label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Sign in to Abode");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    ta_aemail = lv_textarea_create(scr, NULL);
    lv_obj_set_width(ta_aemail, 240);
    lv_textarea_set_one_line(ta_aemail, true);
    //lv_textarea_set_text(ta, _abodeEmail.c_str());
    lv_textarea_set_text(ta_aemail, "gil.philbert@googlemail.com");
    lv_textarea_set_cursor_hidden(ta_aemail, true);
    lv_textarea_set_placeholder_text(ta_aemail, "Email address");
    lv_obj_align(ta_aemail, NULL, LV_ALIGN_IN_TOP_MID, 0, 35);
    //lv_obj_set_event_cb(ta, saveAEmail);

    lv_obj_t * kb = lv_keyboard_create(scr, NULL);
    lv_keyboard_set_cursor_manage(kb, true);
    lv_keyboard_set_textarea(kb, ta_aemail);

    lv_obj_t * btn = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb(btn, abodePassword);
    lv_obj_align(btn, kb, LV_ALIGN_OUT_TOP_MID, 0, -8);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Next");

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 500, true);
}

void Configure::abode() {
  abodeEmail();
}

short Configure::loop(void) {
  if(netlist.size() > 0) {
    lv_obj_clean(wifiContainer);

    lv_obj_t * list = lv_list_create(wifiContainer, NULL);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_size(list, 480, 210);

    lv_obj_t *list_btn;
    
    for(int i = 0; i < netlist.size() - 1; i++) {
      list_btn = lv_list_add_btn(list, LV_SYMBOL_WIFI, netlist[i].c_str());
      lv_obj_set_event_cb(list_btn, enterKey);
    }
    netlist.clear();
  }
  if (_wifi_connecting != CONNECT_IDLE) {
    if (_wifi_connecting == CONNECT_SUCCESS) {
      _wifi_connecting = CONNECT_IDLE;
      AbodeSettings.setNetwork(_netssid, _netpass);
      _netssid = "";
      _netpass = "";
      return CONFIGURE_WIFI;
    } else if(_wifi_connecting == CONNECT_FAILED) {
      _wifi_connecting = CONNECT_IDLE;
      loadWifi();
    }
  }
  if (credentialsComplete == true) {
    credentialsComplete = false;
    AbodeSettings.setAbodeLogin(_abodeEmail, _abodePass, "AbodePanel");
    _abodePass = "";
    return CONFIGURE_ABODE;
  }
  return CONFIGURE_IDLE;
}