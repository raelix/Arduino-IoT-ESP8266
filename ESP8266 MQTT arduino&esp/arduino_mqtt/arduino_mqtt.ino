#include <SoftwareSerial.h>
#include <EmonLib.h>

#define EnergySaveEnable 1
#define InvertPinSignal 0
#define fisicalSwitch 2
#define power A0
#define rele 11
#define interruptPin 1 //pin 3 arduino uno
#define ThresholdInterruptDelay 100
const int sleepTime = 2000;
const int vac = 230;

SoftwareSerial esp8266(10, 9);
EnergyMonitor emon1;

int retries = 0;
boolean releState = 0;
int powerValue = 0;
boolean isHigh = true;
long lastMs;
long lastSent;

void setup() {
  initPins();
  initSerial();
  releState = digitalRead(fisicalSwitch);
  emon1.current(power, 9.3);
  attachInterrupt(interruptPin, changeState, CHANGE);
  lastMs = millis();
  lastSent = millis();
}

void loop() {
  delay(500);
  checkFisicalButtonChanged();
  checkEnergySaveMode();
  if (millis() - lastSent >= sleepTime) {
    computePower();
    lastSent = millis();
    esp8266.print(getMessage());
    Serial.println("potenza aggiornata!");
    if (esp8266.available() > 0) {
      Serial.println(esp8266.readString());
    }
  }
}

void computePower() {
  powerValue = (emon1.calcIrms(1480)) * 230 ;
  if (powerValue <= 13)
    powerValue = 0;
}

void initPins() {
  pinMode(rele, OUTPUT);
  pinMode(fisicalSwitch, INPUT_PULLUP);
  if (InvertPinSignal != 1)
    digitalWrite(rele, LOW);
  else
    digitalWrite(rele, HIGH);

}

void initSerial() {
  esp8266.begin(115200);
  Serial.begin(9600);
}

void changeState() {
  if (isHigh && millis() - lastMs > ThresholdInterruptDelay) {
    isHigh = false;
    if (InvertPinSignal != 1)
      digitalWrite(rele, LOW);
    else
      digitalWrite(rele, HIGH);

    lastMs = millis();
  }
  else if (!isHigh && millis() - lastMs > ThresholdInterruptDelay) {
    isHigh = true;
    if (InvertPinSignal != 1)
      digitalWrite(rele, HIGH);
    else
      digitalWrite(rele, LOW);
    lastMs = millis();
  }
}

void checkFisicalButtonChanged() {
  if (digitalRead(fisicalSwitch) != releState) {
    if (isHigh)
      sendMessageSwitch(0);
    else
      sendMessageSwitch(1);
    releState = digitalRead(fisicalSwitch);
    changeState();
  }
}

void checkEnergySaveMode() {
  if (EnergySaveEnable == 1)
    if (((InvertPinSignal == 1 && digitalRead(rele) == LOW) || (InvertPinSignal != 1 && digitalRead(rele) == HIGH))
        && powerValue <= 0) {
      if (retries >= 20) {
        if (isHigh)
          sendMessageSwitch(0);
        else
          sendMessageSwitch(1);

        changeState();
        retries = 0;
      }
      else {
        retries++;
      }
    }
}

void sendMessageSwitch(int value) {
  String pwr = "powerSwitch:";
  pwr += value;
  pwr += ";";
  esp8266.print(pwr);
}

String getMessage() {
  String pwr = "power:";
  pwr += powerValue;
  pwr += ";";
  return pwr;
}

