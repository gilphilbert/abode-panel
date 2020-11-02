#ifndef ABODE_CLIENT
#define ABODE_CLIENT

#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include <WebSocketsClient.h>
#include <SocketIoClient.h>


#define ABODE_CONNECT     "connect"
#define ABODE_DISCONNECT  "disconnect"

#define ABODE_MODE_STANDBY  "standby"
#define ABODE_MODE_HOME     "home"
#define ABODE_MODE_AWAY     "away"
#define ABODE_MODE          "com.goabode.gateway.mode"

#define ABODE_EVENT_ARMING  6061

#define ABODE_TIMELINE      "com.goabode.gateway.timeline"
#define ABODE_UPDATE        "com.goabode.device.update"

class Abode {
  private:
    const char* abodeCAFingerprint;
    const char* _rootCA;
    String _login_token;
    String _oauth_token;
    String _session;
    bool ws_status;

    bool doAbodeLogin(String login_id, String login_pass);
    bool getAbodeToken();
    bool getAbodeSession();

    String getAPI(String url, String body);
  public:
    bool begin(String username, String password);
    void loop();
    void on(const char* event, std::function<void (const char * payload, size_t length)>);
    bool isConnected(void);

    void setMode(String panel_mode);
    String getMode(void);
    uint8_t getExit(String mode);
};

#endif