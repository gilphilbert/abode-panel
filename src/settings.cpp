#include "settings.h"
#include "Preferences.h"

Settings AbodeSettings;

Preferences pref;

bool Settings::load() {
    bool returnVal = false;
    if(pref.begin("CoreSettings")) {
        //pref.clear();
        if (pref.getString("email") != "") {
            Serial.println("Loading settings...");
            _settingsData.login.email=pref.getString("email");
            _settingsData.login.pass=pref.getString("password");
            _settingsData.login.uuid=pref.getString("uuid");
            _settingsData.net.pass=pref.getString("key");
            _settingsData.net.ssid=pref.getString("ssid");
            returnVal = true;
        }
    }
    if (returnVal == false) { Serial.println("No settings"); }
    pref.end();
    return returnVal;
}

void Settings::save() {
    if(pref.begin("CoreSettings")) {
        Serial.println("Saving settings");
        pref.putString("email", _settingsData.login.email);
        pref.putString("password", _settingsData.login.pass);
        pref.putString("uuid", _settingsData.login.uuid);
        pref.putString("key", _settingsData.net.pass);
        pref.putString("ssid", _settingsData.net.ssid);
        pref.end();
    }
}

void Settings::setNetwork(String ssid, String key) {
    _settingsData.net.ssid = ssid;
    _settingsData.net.pass = key;
}

NetSettings Settings::getNetwork() {
    return _settingsData.net;
}

void Settings::setAbodeLogin(String email, String password, String uuid) {
    _settingsData.login.email = email;
    _settingsData.login.pass = password;
    _settingsData.login.uuid = uuid;
}
AbodeLogin Settings::getAbodeLogin() {
    return _settingsData.login;
}