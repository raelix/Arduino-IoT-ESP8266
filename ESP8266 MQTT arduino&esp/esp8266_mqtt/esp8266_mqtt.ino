#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const String powerStr = "potenza tv";
const String powerSwitchStr = "interruttore tv";
char* topic = "smarthome/potenza tv";
char* topicSwitch = "smarthome/interruttore tv";
char* ssid = "NETGEAR65";
char* password = "giammi20102011";
char* server = "192.168.0.10";
int power;
int oldPower;
int powerSwitch;
int oldPowerSwitch;
String rcv = "";
long lastSend = 0;

IPAddress ip(192, 168, 0, 55);
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("arrived message");
  char message_buff[100];
  int k = 0;
  for (int i = 0; i < length; i++) {
    message_buff[i] = payload[i];
    k = i;
  }
  message_buff[k] = '\0';
  String msgString = String(message_buff);
  Serial.println(msgString);
  if(msgString.indexOf("command") != -1){
  if (powerSwitch == 1)
    powerSwitchs("0");
  else
    powerSwitchs("1");
  }
}

void setup() {
  powerSwitch = 0;
  power = 0;
  oldPowerSwitch = 0;
  oldPower = 0;
  Serial.begin(115200);
  delay(10);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  IPAddress ip(192, 168, 0, 55);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);
    /* if (client.publish(topic, "hello from ESP8266")) {
       Serial.println("Publish ok");
     }
     else {
       Serial.println("Publish failed");
     }*/
    if (client.subscribe(topicSwitch)) {
      Serial.print("Subcribe to ");
      Serial.println(powerSwitchStr);
    }
    else {
      Serial.println("error subscribing");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
  lastSend = millis();
}

void loop() {
  client.loop();
  static int counter = 0;
  if (Serial.available() > 0) {
    char a = Serial.read();
    rcv += a;
    //recive power or switch changes from arduino's serial
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
        powerSwitch = values.toInt();
        rcv = "";
        powerSwitchs(values);
      }
    }
  }
  //send new data to mqtt broker
  if (millis() - lastSend >= 1000 && ( oldPower != power || powerSwitch != oldPowerSwitch)) {
    lastSend = millis();
    int typeOtopic = -1;
    String payload = "";
    if (oldPower != power) {
      oldPower = power;
      payload = "{\"report\":{\"value\":";
      payload += power;
      payload += "}}";
      typeOtopic = 0;
    }
    else if (powerSwitch != oldPowerSwitch) {
      oldPowerSwitch = powerSwitch ;
      payload = "{\"report\":{\"value\":";
      payload += powerSwitch;
      payload += "}}";
      typeOtopic = 1;
    }
    if (client.connected()) {
      Serial.print("Sending payload: ");
      Serial.println(payload);
      if (typeOtopic == 0) {
        if (client.publish(topic, (char*) payload.c_str())) {
          Serial.println(topic);
        }
        else {
          Serial.println("Publish failed");
        }
      }
      else if (typeOtopic == 1) {
        if (client.publish(topicSwitch, (char*) payload.c_str())) {
          Serial.println(topicSwitch);
        }
        else {
          Serial.println("Publish failed");
        }
      }

    }
  }
}

int powerSwitchs(String command) {

  // Get state from command
  int state = command.toInt();
  powerSwitch = state;
  digitalWrite(0, state);
  return 1;
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}
