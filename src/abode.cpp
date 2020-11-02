#include <abode.h>

//let's define the socketio client
SocketIoClient webSocket;

HTTPClient http;

//these could change, they're the certificate details
//needed for websocket
const char* abodeCAFingerprint = "E5 6F 64 91 24 AE EF 24 8F 86 63 12 76 34 66 69 5D 33 3F 7C 28 84 F3 77 19 59 F4 68 AE 82 F4 9D";
//needed for wifi client
const char* _rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFYjCCBEqgAwIBAgIQAQB3qgxVQKN+eX4uUYqRpDANBgkqhkiG9w0BAQsFADBG\n" \
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRUwEwYDVQQLEwxTZXJ2ZXIg\n" \
"Q0EgMUIxDzANBgNVBAMTBkFtYXpvbjAeFw0yMDAxMjEwMDAwMDBaFw0yMTAyMjEx\n" \
"MjAwMDBaMBgxFjAUBgNVBAMMDSouZ29hYm9kZS5jb20wggEiMA0GCSqGSIb3DQEB\n" \
"AQUAA4IBDwAwggEKAoIBAQCqtgdxjm5moMxmAQTBHB5xMenfqXLT4a3IAzsPOPla\n" \
"Q23kLlSg+ZoQEdW8UChvARHVwdEVkoXFyjE0sBAEp2vUjszjJ7GVVGx9pkuDPkeC\n" \
"MegN3gsClN9C1vCMXz8AkP3nDMW9yBZU/Y0HOMqT2dM0QnG3JTKPlk+jzAzioUlv\n" \
"A+2k/nYtAYPRrfx+gwCGnleRgcptsWF6pBWubtorh69FtpfwHBMq/YiwgX7rmmmE\n" \
"N0CDqYvz10welJ1GSVy6A8PW+Th9lnPSucTvwO9KKLdGLBqsaWE+4DioZ/X/UtoZ\n" \
"B12VItrZ82eDabm33/V66zvQl47agCvFj4WxL3LOrZClAgMBAAGjggJ4MIICdDAf\n" \
"BgNVHSMEGDAWgBRZpGYGUqB7lZI8o5QHJ5Z0W/k90DAdBgNVHQ4EFgQU3L2UpWZQ\n" \
"YDRxoiuI61nm319O0QwwGAYDVR0RBBEwD4INKi5nb2Fib2RlLmNvbTAOBgNVHQ8B\n" \
"Af8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMDsGA1UdHwQ0\n" \
"MDIwMKAuoCyGKmh0dHA6Ly9jcmwuc2NhMWIuYW1hem9udHJ1c3QuY29tL3NjYTFi\n" \
"LmNybDAgBgNVHSAEGTAXMAsGCWCGSAGG/WwBAjAIBgZngQwBAgEwdQYIKwYBBQUH\n" \
"AQEEaTBnMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5zY2ExYi5hbWF6b250cnVz\n" \
"dC5jb20wNgYIKwYBBQUHMAKGKmh0dHA6Ly9jcnQuc2NhMWIuYW1hem9udHJ1c3Qu\n" \
"Y29tL3NjYTFiLmNydDAMBgNVHRMBAf8EAjAAMIIBAwYKKwYBBAHWeQIEAgSB9ASB\n" \
"8QDvAHYApLkJkLQYWBSHuxOizGdwCjw1mAT5G9+443fNDsgN3BAAAAFvyD+n9gAA\n" \
"BAMARzBFAiEAjgiTcCyvvVOXghqRG6lrVzd/neGFUq5ALK+m+z7l2J8CIC8gktXN\n" \
"txAg8OszgroWo6apetf6KHKl8BnXv2brIuvlAHUAh3W/51l8+IxDmV+9827/Vo1H\n" \
"Vjb/SrVgwbTq/16ggw8AAAFvyD+o5AAABAMARjBEAiADEYmDYM13ZSqK8Wxb7Xkn\n" \
"p0+kXP39AJC9LGX5+tyMqgIgcCAsgG9dMMf2qio1k7Z3WkOrmgMw1StP2dae6iY0\n" \
"AwQwDQYJKoZIhvcNAQELBQADggEBAFo+qjzkUDDqwJhFTJeSL5UJJ4+1fCgle1d0\n" \
"q320LpcV+jWNcAfK6dytXBmkfAnjG0dhfnCtsF9MmVIyfaTnAzFXQTOV8nysLwwA\n" \
"/YiizmP1LHZcXDjJLS9BVKRm2gU6aT1w59APtdBelaXgkPPsJFPmoWeUFyeqUaQ2\n" \
"yA5sgWkYkK/z2H3UYE5qAjE3Zv7S4aq0J14fR0xAbX6o/PK0xPQ0M9sUFHME0+up\n" \
"rAOZ/LIiZucVvJuS3AHVHAFofP/R+MF3sk1XEI5tXNGz0Pu96cUt2Lj6ENoEhVyk\n" \
"HOfQiMxPQtXmpvYGmChniurk5bKoUW1SVZFLdSdbxKQiz6cjYgM=\n" \
"-----END CERTIFICATE-----\n";

String _login_token = "";
String _oauth_token = "";
String _session = "";

bool Abode::doAbodeLogin(String login_id, String login_pass) {
  bool returnVal = false;

  JSONVar postData;
  postData["id"] = login_id;
  postData["password"] = login_pass;
  postData["uuid"] = "ESP";
  String postString = JSON.stringify(postData);

  //do login
  http.begin("https://my.goabode.com/api/auth2/login", _rootCA);
  //add the header stating it's JSON being sent
  http.addHeader("Content-Type", "application/json");
  //send the request
  int httpcode = http.POST(postString);
  //check too see it was OK
  if(httpcode == 200) {
    //get the payload
    String payload = http.getString();
    //parse it
    JSONVar data = JSON.parse(payload);
    //extract the token
    _login_token = (const char*) data["token"];
    returnVal = true;
  }
  http.end();
  return returnVal;
}

bool Abode::getAbodeToken() {
  bool returnVal = false;
  //now get the oauth token
  http.begin("https://my.goabode.com/api/auth2/claims", _rootCA);
  //attach the cookie
  http.addHeader("ABODE-API-KEY", _login_token);
  //make the request
  int httpcode = http.GET();
  //check it's OK
  if(httpcode == 200) {
    //extract the payload
    String payload = http.getString();
    //parse it
    JSONVar data = JSON.parse(payload);
    //get the token
    _oauth_token = (const char*) data["access_token"];
    returnVal = true;
  }
  http.end();
  return returnVal;
}

bool Abode::getAbodeSession() {
  bool returnVal = false;
  //now get the oauth token
  http.begin("https://my.goabode.com/api/v1/session", _rootCA);
  //attach the cookie
  http.addHeader("ABODE-API-KEY", _login_token);
  //make the request
  int httpcode = http.GET();
  //check it's OK
  if(httpcode == 200) {
    //extract the payload
    String payload = http.getString();
    //parse it
    JSONVar data = JSON.parse(payload);
    //get the token
    _session = (const char*) data["id"];
    returnVal = true;
  }
  http.end();
  return returnVal;
}

bool Abode::begin(String username, String password) {
  doAbodeLogin(username, password);
  if (_login_token != "") {
    getAbodeToken();
    getAbodeSession();
    const String sheaders = "Origin: https://my.goabode.com/\nAuthorization: Bearer " + _oauth_token + "\nCookie: SESSION=" + _session + ";key=" + _login_token;
    int n = sheaders.length();  
    char headers[n + 1];
    strcpy(headers, sheaders.c_str());
  
    //attach them
    webSocket.setExtraHeaders(headers);
  
    //start the socket 
    webSocket.beginSSL("my.goabode.com", 443, "/socket.io/?transport=websocket", abodeCAFingerprint);

    return true;
  } else {
    // tell the device that we couldn't login
    return false;
  }
}

void Abode::loop() {
  webSocket.loop();
}

void Abode::on(const char* event, std::function<void (const char * payload, size_t length)> func) {
  webSocket.on(event, func);
}

bool Abode::isConnected(void) {
  return webSocket.isConnected();
}

String Abode::getAPI(String url, String body) {
  //now get the oauth token
  http.begin("https://my.goabode.com/api/v1/" + url, _rootCA);
  //attach the cookie
  http.addHeader("ABODE-API-KEY", _login_token);
  //make the request
  int httpcode = http.GET();
  //check it's OK
  if(httpcode == 200) {
    //extract the payload
    String payload = http.getString();
    http.end();
    return payload;
  }
  http.end();
  return "";
}

void Abode::setMode(String panel_mode) {
  Serial.print("Setting mode::");
  Serial.println(panel_mode);
  //some data, including the area (although Abode currently only supports one) and mode
  String params = "{ area: 1, mode: " + panel_mode + " }";
  //this is the URL for mode change (same as get, just that we're not PUTting)
  http.begin("https://my.goabode.com/api/v1/panel/mode/1/" + panel_mode, _rootCA);
  //attach the cookie
  http.addHeader("ABODE-API-KEY", _login_token);
  //PUT the request
  int httpcode = http.PUT(params);
  //check it's OK
  if(httpcode == 200) {
    Serial.println("OK");
  } else {
    Serial.println("Error");
  }
  //close the connection
  http.end();
}

String Abode::getMode(void) {
  JSONVar re = JSON.parse(getAPI("panel", ""));
  if(re.hasOwnProperty("mode")) {
    if(re["mode"].hasOwnProperty("area_1")) {
      return (const char*) re["mode"]["area_1"];
    }
  }
  return "";
}

uint8_t Abode::getExit(String mode) {
  JSONVar re = JSON.parse(getAPI("panel", ""));
  if (JSON.typeof(re) == "object") {
    JSONVar modes = re["areas"]["1"]["modes"];
    JSONVar keys = modes.keys();
    for (int i = 0; i < keys.length(); i++) {
      String name = (const char*) modes[keys[i]]["name"];
      if (name == mode) {
        const char* exit = (const char*) modes[keys[i]]["exit"];
        return atoi(exit);
      }
    }
  }
  return 255;
}