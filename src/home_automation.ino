#define BLYNK_TEMPLATE_ID "**********"
#define BLYNK_TEMPLATE_NAME "lamp automation"
#define BLYNK_AUTH_TOKEN "*****************************"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

char ssid[] = "x";
char pass[] = "12345678";

#define RELAY_PIN D1

BlynkTimer timer;
WidgetRTC rtc;

bool manualControl = false;
int lastCheckedDay = -1;
bool relayState = false;

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  manualControl = true;
  digitalWrite(RELAY_PIN, pinValue);
  relayState = pinValue;
  updateStatus();
}

BLYNK_WRITE(V1) {
  int resetValue = param.asInt();
  if (resetValue == 1) {
    manualControl = false;
    updateStatus();
  }
}

void checkSchedule() {
  int currentHour = hour();
  int currentMinute = minute();
  int currentDay = day();

  if (currentHour == 0 && lastCheckedDay != currentDay) {
    manualControl = false;
    lastCheckedDay = currentDay;
    updateStatus();
  }

  if (manualControl) return;

  bool shouldBeOn = (currentHour >= 18 && currentHour < 22) ||
                    (currentHour == 22 && currentMinute == 0);

  digitalWrite(RELAY_PIN, shouldBeOn ? HIGH : LOW);
  relayState = shouldBeOn;
  updateStatus();
}

void updateStatus() {
  String modeText = manualControl ? "Manual" : "Auto";
  String relayText = relayState ? "Relay ON" : "Relay OFF";
  Blynk.virtualWrite(V2, relayText + " (" + modeText + ")");
  Blynk.virtualWrite(V3, relayState ? 255 : 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  rtc.begin();
  timer.setInterval(60000L, checkSchedule);
  timer.setInterval(5000L, updateStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}
