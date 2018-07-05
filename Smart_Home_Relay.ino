/*
   written by Jake Pring from Circuit Specialists.com
   licensed as GPLv3
  commands:
    0xA00100A1 opens relay
    0xA00101A2 closes relay
*/
#include <ESP8266WiFi.h>

const char* ssid = "";
const char* password = "";
byte close_relay[] = {0xA0, 0x01, 0x01, 0xA2, 0x0D, 0x0A};
byte open_relay[] = {0xA0, 0x01, 0x00, 0xA1, 0x0D, 0x0A};
const char* use_static_up = "yes";
IPAddress static_address(192, 168, 1, 248);
IPAddress gateway_ip(192, 168, 1, 1);
IPAddress subnet_mask(255, 255, 255, 0);
IPAddress dns_one(8, 8, 8, 8);
IPAddress dns_two(8, 8, 4, 4);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  delay(100);
  if (use_static_up == "yes") {
    WiFi.config(static_address, gateway_ip, subnet_mask, dns_one, dns_two);
  }
  Serial.println(WiFi.localIP());
}

void loop() {
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

  // sort http request
  int val;
  if (req.indexOf("/turnon") != -1) {
    val = 0;
    Serial.write(close_relay, sizeof(close_relay));
  }
  else if (req.indexOf("/turnoff") != -1) {
    val = 1;
    Serial.write(open_relay, sizeof(open_relay));
  }
  else {
    client.stop();
    return;
  }
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nRelay has been set to the ";
  s += (val) ? "off" : "on";
  s += "state\n</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

