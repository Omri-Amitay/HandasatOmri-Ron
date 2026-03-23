#ifndef Website_h
#define Website_h

#include "Arduino.h"
#include "Config.h"
#include "Tunables.h"
#include <WiFi.h>
#include <WebServer.h>

class Website {
  public:
    Website(TunablesManager* tunablesManager);
    void init();
    void handleRoot();
    void handleData();
    void handleSet();

    void update();
  private:
    TunablesManager* tunablesManager;
    WebServer server;
    
};

#endif