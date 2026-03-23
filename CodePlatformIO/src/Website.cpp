#include "Website.h"
#include "Tunables.h"
#include "Arduino.h"
#include "Config.h"
#include <WiFi.h>
#include <WebServer.h>

Website::Website(TunablesManager* tunablesManager)
  :server(80),
  tunablesManager(tunablesManager)
{}

void Website::init(){
  Serial.println("Website initialized successfully!");
  WiFi.begin(WiFiName , WiFiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", [this]() { handleRoot(); });
  server.on("/data", [this]() { handleData(); });
  server.on("/set", [this]() { handleSet(); });
  this->server.begin();
}

void Website::handleRoot(){
  this->server.send(200, "text/html", HTML_MAIN);
}

void Website::handleData(){
  this->server.send(200, "application/json", tunablesManager->toJson());
}

void Website::handleSet(){
  if (!server.hasArg("group") || !server.hasArg("field") || !server.hasArg("value")) {
    server.send(400, "text/plain", "Missing args");
    return;
  }

  String group = server.arg("group");
  String field = server.arg("field");
  double value = server.arg("value").toDouble();

  bool ok = tunablesManager->setValue(group, field, value);

  if (ok) {
    Serial.print(" Updated Table successfully! ");
    server.send(200, "text/plain", "OK");
  } else {
    Serial.print(" Something went wrong! ");
    server.send(404, "text/plain", "Field or group not found");
  }
}

void Website::update(){
  server.handleClient();
}

