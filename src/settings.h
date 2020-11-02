#ifndef ABODE_SETTINGS
#define ABODE_SETTINGS

#include <Arduino.h>

struct NetSettings {
  String ssid = "";
  String pass = "";
};

struct AbodeLogin {
  String email = "";
  String pass = "";
  String uuid = "";
};

struct SettingsData {
  NetSettings net;
  AbodeLogin login;
};

class Settings {
  private:
    SettingsData _settingsData;
  public:
    bool load();
    void save();

    void setNetwork(String ssid, String key);
    NetSettings getNetwork();

    void setAbodeLogin(String email, String password, String uuid);
    AbodeLogin getAbodeLogin();
};

extern Settings AbodeSettings;

#endif