/*
 * written by Jake Pring from Circuit Specialists.com
 * licensed as GPLv3
  commands:
    A00101A1 closes relay
    A00101A2 opens relay
 */

#include <ESP8266WiFi.h>

const char* ssid = "circuitspecialsts.com";
const char* password = "C!rCu!t!";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  
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
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // sort http request
  int val;
  if (req.indexOf("/turnoff") != -1)
    val = 0;
    Serial.write(A00101A2);
  else if (req.indexOf("/turnon") != -1)
    val = 1;
    Serial.write(A00101A1);
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nRelay has been set to the ";
  s += (val)?"on":"off";
  s += "state\n</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

