#ifndef ALARMSERVER_H
#define ALARMSERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <constants.h>
#include <FS.h>
#include <LittleFS.h>

AsyncWebServer alarmServer(80);

/**
 * @brief Takes the submitted alarms and saves them to NVS.
 *
 * @param request
 * @param data
 * @param len
 * @param index
 * @param total
 */
void handleListSubmit(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    JsonArray alarms = doc["alarms"];
    Serial.println("Received alarms:");

    String allAlarms;
    for (JsonVariant alarm : alarms)
    {
        String alarmString = alarm.as<String>();
        Serial.println(alarmString);

        allAlarms += alarmString + ",";
    }

    saveAlarms(allAlarms);
    Serial.println(retrieveAlarms());

    request->send(200, "text/plain", "Alarms Received");
}

/**
 * @brief Start a server where a client set alarms.
 *
 */
void beginAlarmServer()
{
    alarmServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, "/set_alarm.html", "text/html"); });

    alarmServer.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, "/script.js", "application/javascript"); });

    alarmServer.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, "/styles.css", "text/css"); });

    alarmServer.on("/submit-alarms", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleListSubmit);

    alarmServer.on("/getTimes", HTTP_GET, [](AsyncWebServerRequest *request)
                   {String savedAlarms = retrieveAlarms();
                    Serial.println("Sent saved alarms to Client.");
                    request->send(200, "text/plain", savedAlarms); });

    alarmServer.begin();
}

#endif