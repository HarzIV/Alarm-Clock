#ifndef CREDENTIALSERVER_H
#define CREDENTIALSERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <constants.h>
#include <AlarmServer.h>

AsyncWebServer credentialServer(80);
Preferences credentialStorage;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

/**
 * @brief Try's out the provided credentials
 *
 * @note This function try's out the ssid and password,
 * if the credentials work a connection is established,
 * they get saved to NVS and the alarm server gets started.
 * If the credentials dont work the ESP gets restarted
 *
 * @param ssid
 * @param password
 */
void tryCredentials(const char *ssid, const char *password)
{
  // Initialize ESP32 in station mode and connect to external WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  for (int i = 0; i < 10; i++)
  {
    delay(500);
    Serial.print(".");
    // If WiFi wasn't connected after 10 attempts, re request credentials
    if (WiFi.status() != WL_CONNECTED && i == 9)
    {
      Serial.println("WiFi ssid or password incorrect, or WiFi unavailable");
      ESP.restart();
    }
    // If WiFi was connected successfully
    else if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.println("WiFi connected successfully!");

      // Save submitted credentials
      credentialStorage.begin("credentials", false);
      credentialStorage.putString("ssid", ssid);
      credentialStorage.putString("password", password);
      credentialStorage.putBool("status", true);
      credentialStorage.end();
      Serial.println("Saved credentials");

      credentialServer.end();
      Serial.println("Ended credential server");

      beginAlarmServer();

      break;
    }
  }
}

/**
 * @brief Starts a server where WiFi credentials can be submitted
 *
 * @note If credentials get submitted the tryCredentials function gets called
 *
 */
void beginCredentialServer()
{
  // Create ESP32 WiFi network
  WiFi.softAP(ssidOWN, passwordOWN);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route for serving the index.html file
  credentialServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(LittleFS, "/index.html", "text/html"); });

  // Route for serving the index.html file
  credentialServer.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(LittleFS, "/connect.html", "text/html"); });

  // Route for serving the styles.css file
  credentialServer.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(LittleFS, "/styles.css", "text/css"); });

  // Route to handle form submission
  credentialServer.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) { // Get submitted ssid and password
    static char ssid[32] = "";
    static char password[64] = "";

    strcpy(ssid, request->arg("ssid").c_str());
    strcpy(password, request->arg("password").c_str());

    // Redirect user success page
    request->send(LittleFS, "/connect.html", "text/html");

    // Print SSID and password to Serial monitor
    Serial.printf("SSID: %s\n", ssid);
    Serial.printf("Password: %s\n", password);

    // connect to WiFi
    Serial.printf("Connecting to %s\n", ssid);
    tryCredentials(ssid, password);
  });

  Serial.println("HTTP credential server started");
  credentialServer.begin();
}

/**
 * @brief Function to handle connecting to WiFi
 *
 * @note Try's to establish a connection, using credential
 * stored in NVS. If this works it will start the alarm server.
 * If there are no credentials in NVS or it is unable to establish
 * a connection it will reboot the ESP and start the credential server.
 *
 */
void initWiFi()
{
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
  }
  else
  {
    Serial.println("LittleFS Mount Successful");
  }

  credentialStorage.begin("credential", true);
  if (credentialStorage.getBool("status", false))
  {
    String saved_ssid = credentialStorage.getString("ssid");
    String saved_password = credentialStorage.getString("password");
    credentialStorage.end();

    Serial.print("Saved ssid: ");
    Serial.println(saved_ssid);
    Serial.print("Saved password: ");
    Serial.println(saved_password);
    Serial.print("Connecting to saved WiFi");

    // Initialize ESP32 in station mode and connect to external WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(saved_ssid.c_str(), saved_password.c_str());
    for (int i = 0; i < 10; i++)
    {
      delay(500);
      Serial.print(".");
      // If WiFi wasn't connected after 10 attempts, re request credentials
      if (WiFi.status() != WL_CONNECTED && i == 9)
      {
        // Set status to false so that entering credentials will be forced after restart
        Serial.println("\nRestart");
        credentialStorage.putBool("status", false);
        ESP.restart();
      }
      else if (WiFi.status() == WL_CONNECTED) // If WiFi was connected successfully
      {
        Serial.print("\nIP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("WiFi connected successfully!");

        beginAlarmServer();

        break;
      }
    }
  }
  else
  {
    // Start WebServer to enter credentials
    Serial.println("WiFi ssid or password incorrect, or WiFi unavailable");
    Serial.println("Enter new ssid/password on WebPage");
    credentialStorage.end();
    beginCredentialServer();
  }
}

#endif