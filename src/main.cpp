#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "secrets.h"
#include "constants.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;
String supabaseKey = SUPABASE_API_KEY;
String supabaseUrl = SUPABASE_URL;

String hostname;
String deviceIp;

String response;
String previousResponse;
bool isNewResponse = false;

// Define the task handles for the getTelegramTask, sendTelegramTask and otaTask
TaskHandle_t getTelegramTaskHandle;
TaskHandle_t sendTelegramTaskHandle;
TaskHandle_t otaTaskHandle;

// Root CA certificate of supabase
const char *rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock()
{
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

int browseService(const char *service, const char *proto)
{
  Serial.printf("Browsing for service _%s._%s.local. ... ", service, proto);
  int n = MDNS.queryService(service, proto);
  if (n == 0)
  {
    Serial.println("no services found");
    return 0;
  }

  Serial.print(n);
  Serial.println(" service(s) found");
  for (int i = 0; i < n; ++i)
  {
    // Print details for each service found
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(MDNS.hostname(i));
    Serial.print(" (");
    Serial.print(MDNS.IP(i));
    Serial.print(":");
    Serial.print(MDNS.port(i));
    Serial.println(")");
  }
  Serial.println();

  return n;
}

String getHostnameP1Meter(int n)
{
  String hostname = "";
  for (int i = 0; i < n; ++i)
  {
    if (MDNS.hostname(i).startsWith("p1meter"))
    {
      hostname = MDNS.hostname(i);
      break;
    }
  }
  return hostname;
}

void pingGoogle()
{
  bool success = Ping.ping("www.google.com", 3);

  if (!success)
  {
    Serial.println("Ping failed");
    return;
  }

  Serial.println("Ping succesful.");
}

// Function to get the telegram data
void getTelegram()
{
  HTTPClient http;

  // Announce to serial that the task is running
  Serial.println("Task: Telegram");

  // Access the p1-meter API and get the telegram
  // If the http request was successful, save the response if it is not the same as the previous response
  http.begin("http://" + deviceIp + "/api/v1/telegram");

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      response = http.getString();
      if (response != previousResponse)
      {
        // Save the response as the previous response
        previousResponse = response;

        // Notify the sendTelegram task to send the data
        xTaskNotifyGive(sendTelegramTaskHandle);
      }
    }
  }
  else
  {
    Serial.println("Error on HTTP request");
    http.end();
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP.restart();
  }
  http.end();
}

void sendToSupabase()
{
  WiFiClientSecure *client = new WiFiClientSecure;
  if (client)
  {
    client->setCACert(rootCACertificate);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("[HTTPS] begin...\n");
      if (https.begin(*client, supabaseUrl))
      { // HTTPS
        Serial.println("[HTTPS] POST... ");

        // Specify the content type as plain/text
        https.addHeader("Content-Type", "plain/text");

        // Add authentication header
        https.addHeader("Authorization", "Bearer " + supabaseKey);

        // Add the content length
        https.addHeader("Content-Length", String(response.length()));

        // Send the POST request with the response as payload
        int httpsCode = https.POST(response);

        // httpsCode will be negative on error
        if (httpsCode > 0)
        {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] POST... code: %d\n", httpsCode);
        }
        else
        {
          Serial.printf("[HTTPS] POST... failed, error (code: %d): %s\n", httpsCode, https.errorToString(httpsCode).c_str());
        }

        https.end();
      }
      else
      {
        Serial.printf("[HTTPS] Unable to connect\n");
      }
    }

    delete client;
  }
  else
  {
    Serial.println("Unable to create client");
  }
}

// Task to get the telegram data every 10 seconds
void getTelegramTask(void *pvParameters)
{
  time_t nowSecs;
  struct tm timeinfo;

  // Wait until the clock is synced
  while (time(nullptr) < 8 * 3600 * 2)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  nowSecs = time(nullptr);
  gmtime_r(&nowSecs, &timeinfo);

  // Calculate the delay until the next minute starts
  int delayToNextMinute = (60 - timeinfo.tm_sec) * 1000;
  vTaskDelay(delayToNextMinute / portTICK_PERIOD_MS);

  // Define the wake time for the first iteration
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Start the task loop
  while (true)
  {
    getTelegram();

    // Delay the task until 60 seconds have passed since the previous wake time or until the first minute boundary
    vTaskDelayUntil(&xLastWakeTime, INTERVAL_SEND / portTICK_PERIOD_MS);
  }
}

void sendToSupabaseTask(void *pvParameters)
{
  while (true)
  {
    // Announce to serial that the task is running
    Serial.println("Task: Supabase");

    // Wait for the getTelegram task to notify that new data is available
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Send the data to Supabase
    sendToSupabase();
  }
}

void otaTask(void *pvParameters)
{
  while (true)
  {
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");

  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("hw-p1meter-forwarder");
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to: " + WiFi.SSID());
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // Set clock for SSL
  setClock();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setMdnsEnabled(true);
  // ArduinoOTA.setHostname("hw-p1meter-forwarder");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  // Find the number of homewizard services
  int n = browseService("hwenergy", "tcp");

  if (n == 0)
  {
    Serial.println("No p1-meter found");
    delay(5000);
    ESP.restart();
  }

  // Get the hostname of the first p1-meter found
  hostname = getHostnameP1Meter(n);
  deviceIp = MDNS.IP(0).toString();

  // Create the task to get the telegram
  xTaskCreatePinnedToCore(
      getTelegramTask,        // Function that should be called
      "Get Telegram",         // Name of the task (for debugging)
      4096,                   // Stack size (bytes)
      NULL,                   // Parameter to pass
      1,                      // Task priority
      &getTelegramTaskHandle, // Task handle
      0);                     // Core to run the task on

  // Create the task to send data to Supabase
  xTaskCreatePinnedToCore(
      sendToSupabaseTask,      // Function that should be called
      "Send to Supabase",      // Name of the task (for debugging)
      4096,                    // Stack size (bytes)
      NULL,                    // Parameter to pass
      2,                       // Task priority
      &sendTelegramTaskHandle, // Task handle
      1);                      // Core to run the task on

  // Create task for OTA
  xTaskCreatePinnedToCore(
      otaTask,        // Function that should be called
      "OTA",          // Name of the task (for debugging)
      4096,           // Stack size (bytes)
      NULL,           // Parameter to pass
      3,              // Task priority
      &otaTaskHandle, // Task handle
      1);             // Core to run the task on
}

void loop()
{
  // Empty loop
}