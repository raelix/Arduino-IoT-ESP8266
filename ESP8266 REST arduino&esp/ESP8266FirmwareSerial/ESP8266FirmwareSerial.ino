/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "NETGEAR65";
const char* password = "giammi20102011";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int power;
int powerSwitch;
String rcv = "";

void setup(void)
{
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  // Start Serial
  Serial.begin(9600);
  powerSwitch = 0;
  power = 0;
  rest.variable("power", &power);
  rest.variable("powerSwitch", &powerSwitch);

  // Function to be exposed
  rest.function("powerSwitch", powerSwitchs);

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");

  // Connect to WiFi
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
  if (Serial.available() > 0) {
    char a = Serial.read();
    rcv += a;

    if (rcv.indexOf(";") != -1) {
      if (rcv.indexOf("power:") != -1) {
        String values = rcv.substring(rcv.indexOf("power:") + 6, rcv.length() - 1 );
        Serial.print("recived power: ");
        Serial.println(values);
        power = values.toInt();
        rcv = "";
      }
      else if (rcv.indexOf("powerSwitch:") != -1) {
        String values = rcv.substring(rcv.indexOf("powerSwitch:") + 12, rcv.length() - 1 );
        Serial.print("recived switch: ");
        Serial.println(values);
        power = values.toInt();
        rcv = "";
        powerSwitchs(values);
      }
    }
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);

}

// Custom function accessible by the API
int powerSwitchs(String command) {

  // Get state from command
  int state = command.toInt();
  powerSwitch = state;
  digitalWrite(0, state);
  return 1;
}
