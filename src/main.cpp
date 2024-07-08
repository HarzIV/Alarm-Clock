// Include library's
#include <Arduino.h>
#include <WiFi.h>
#include "sntp.h"

// Include project specific files
#include "constants.h"
#include "CredentialServer.h"

bool functionExecuted = false;
unsigned long trigTime;

String currentTime;

void setup()
{
  Serial.begin(115200);
  pinMode(alarmPin, OUTPUT);

  display.shutdown(0, false);
  display.shutdown(1, false);
  display.setIntensity(0, 15);
  display.setIntensity(1, 15);
  display.clearDisplay(0);
  display.clearDisplay(1);

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice a year,
   * in such a case time adjustment won't be handled automaticaly.
   */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  initWiFi();

  currentTime = getTime();

  displayTime(currentTime);
}

void loop()
{
  if (currentTime != getTime())
  {
    currentTime = getTime();

    displayTime(currentTime);

    if (!functionExecuted)
    {
      if (soundAlarm(currentTime, retrieveAlarms()))
      {
        trigTime = millis();
        functionExecuted = true;
      }
    }
  }

  if (functionExecuted)
  {
    int runTime = 5 * 1000 * 60;

    if (trigTime <= millis() + runTime)
    {
      digitalWrite(alarmPin, HIGH);
      delay(beepTime);
      digitalWrite(alarmPin, LOW);
      delay(beepTime);
      // Serial.println("BEEP");
    }
    else
    {
      digitalWrite(alarmPin, LOW);
      functionExecuted = false;
    }
  }
}