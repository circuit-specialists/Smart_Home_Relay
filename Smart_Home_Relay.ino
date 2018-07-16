/*
   written by Jake Pring from CircuitSpecialists.com
   licensed as GPLv3
  commands:
    0xA00100A1 opens relay
    0xA00101A2 closes relay
*/
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "";
const char* password = "";
bool use_static_ip = false;
IPAddress static_address(192, 168, 1, 248);
IPAddress gateway_ip(192, 168, 1, 1);
IPAddress subnet_mask(255, 255, 255, 0);
IPAddress dns_one(8, 8, 8, 8);
IPAddress dns_two(8, 8, 4, 4);

// Create an instance of the server with port
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }
  Serial.println("\nWiFi connected");

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("my_esp8266");
  //ArduinoOTA.setPassword("admin");
  // MD5 hash
  //ArduinoOTA.setPasswordHash("456b7016a916a4b178dd72b947c152b7");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    switch (error) {
      case OTA_AUTH_ERROR:
        Serial.println("Auth Failed");
        break;
      case OTA_BEGIN_ERROR:
        Serial.println("Begin Failed");
        break;
      case OTA_CONNECT_ERROR:
        Serial.println("Connect Failed");
        break;
      case OTA_RECEIVE_ERROR:
        Serial.println("Receive Failed");
        break;
      case OTA_END_ERROR:
        Serial.println("End Failed");
        break;
    }
  });
  // Start the OTA server
  ArduinoOTA.begin();
  Serial.println("OTA Started... Ready");

  // Start the http server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  delay(100);
  if (use_static_ip) {
    WiFi.config(static_address, gateway_ip, subnet_mask, dns_one, dns_two);
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  ArduinoOTA.handle();
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  /*********************************************
                URL Script functions
  **********************************************/
  // sort http request
  int val;
  String _mode = "";
  if (req.indexOf("/turnon") != -1) {
    val = 0;
    relay(1);
    _mode = "Relay has been set to the on position\n</html>\n";
  }
  else if (req.indexOf("/turnoff") != -1) {
    val = 1;
    relay(0);
    _mode = "Relay has been set to the off position\n</html>\n";
  }
  else if (req.indexOf("/garagedoor") != -1) {
    relay(1);
    delay(180);
    relay(0);
    _mode = "Garagedoor cycled";
  }
  else {
    client.stop();
    return;
  }
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n" + _mode;

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

void relay(bool value) {
  const byte close_relay[] = {0xA0, 0x01, 0x01, 0xA2, 0x0D, 0x0A};
  const byte open_relay[] = {0xA0, 0x01, 0x00, 0xA1, 0x0D, 0x0A};
  if (value) {
    Serial.write(close_relay, sizeof(close_relay));
  }
  else {
    Serial.write(open_relay, sizeof(open_relay));
  }
}

