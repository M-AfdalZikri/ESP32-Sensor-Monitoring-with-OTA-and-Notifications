#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Change these to your network credentials
const char *ssid = "Your_SSID";
const char *password = "Your_Password";

#define fire_threshold 1000
#define gas_threshold 1300

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create Event Source at /events
AsyncEventSource events("/events");

// Sensor pins
const int gasSensorPin = 34;
const int fireSensorPin = 35;
// const int pirSensorPin = 32;

// ESP32 client IP address
const char *clientIP = "192.168.1.124"; // Replace with ESP32 client IP address

// Task handles
TaskHandle_t TaskFireHandle = NULL;
TaskHandle_t TaskGasHandle = NULL;
// TaskHandle_t TaskPirHandle = NULL;

// Global sensor status
volatile bool fireDetected = false;
volatile bool gasDetected = false;
// volatile bool motionDetected = false;

// WiFi Initialization
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println('.');
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Sensor Notifications</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); max-width: 600px; margin: 0 auto; }
    .status { font-size: 1.4rem; }
    .status.red { color: red; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>ESP32 Sensor Notifications</h1>
  </div>
  <div class="content">
    <div class="card">
      <p class="status" id="fire-status">Fire Status: Loading...</p>
      <p class="status" id="gas-status">Gas Status: Loading...</p>
      <p class="status" id="pir-status">PIR Status: Loading...</p>   
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');

 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);

 source.addEventListener('fire-status', function(e) {
  console.log("fire-status", e.data);
  var fireStatus = document.getElementById("fire-status");
  fireStatus.innerHTML = "Fire Status: " + e.data;
  if (e.data == "Fire Detected") {
    fireStatus.classList.add("red");
  } else {
    fireStatus.classList.remove("red");
  }
 }, false);

 source.addEventListener('gas-status', function(e) {
  console.log("gas-status", e.data);
  var gasStatus = document.getElementById("gas-status");
  gasStatus.innerHTML = "Gas Status: " + e.data;
  if (e.data == "Gas Detected") {
    gasStatus.classList.add("red");
  } else {
    gasStatus.classList.remove("red");
  }
 }, false);

 source.addEventListener('pir-status', function(e) {
  console.log("pir-status", e.data);
  var pirStatus = document.getElementById("pir-status");
  pirStatus.innerHTML = "PIR Status: " + e.data;
  if (e.data == "Motion Detected") {
    pirStatus.classList.add("red");
  } else {
    pirStatus.classList.remove("red");
  }
 }, false);
}
</script>
</body>
</html>)rawliteral";

// Function to control the alarm
void controlAlarm()
{
  WiFiClient client;
  if (fireDetected || gasDetected)
  {
    if (client.connect(clientIP, 80))
    {
      client.print(String("GET ") + "/alarm/on" + " HTTP/1.1\r\n" +
                   "Host: " + clientIP + "\r\n" +
                   "Connection: close\r\n\r\n");
    }
  }
  else
  {
    if (client.connect(clientIP, 80))
    {
      client.print(String("GET ") + "/alarm/off" + " HTTP/1.1\r\n" +
                   "Host: " + clientIP + "\r\n" +
                   "Connection: close\r\n\r\n");
    }
  }
}

// Task for fire sensor
void TaskFire(void *pvParameters)
{
  (void)pvParameters;
  int fireSensorStatus;
  String fireStatus;
  String fireValue;

  for (;;)
  {
    fireSensorStatus = analogRead(fireSensorPin);
    fireValue = String(fireSensorStatus);

    // Processing fire sensor status
    if (fireSensorStatus > fire_threshold)
    {
      fireStatus = "No Fire";
      fireDetected = false;
      Serial.print("No Fire | Value: ");
      Serial.println(fireSensorStatus);
    }
    else
    {
      fireStatus = "Fire Detected";
      fireDetected = true;
      Serial.print("Fire Detected | Value: ");
      Serial.println(fireSensorStatus);
    }

    // Send Event to Web Client with Fire Status and Sensor Value
    events.send(fireStatus.c_str(), "fire-status", millis());
    events.send(fireValue.c_str(), "fire-value", millis());

    // Control alarm based on fire sensor status
    controlAlarm();

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task for gas sensor
void TaskGas(void *pvParameters)
{
  (void)pvParameters;
  int gasSensorStatus;
  String gasStatus;
  String gasValue;

  for (;;)
  {
    gasSensorStatus = analogRead(gasSensorPin);
    gasValue = String(gasSensorStatus);

    // Processing gas sensor status
    if (gasSensorStatus > gas_threshold)
    {
      gasStatus = "Gas Detected";
      gasDetected = true;
      Serial.print("Gas Detected | Value: ");
      Serial.println(gasSensorStatus);
    }
    else
    {
      gasStatus = "No Gas";
      gasDetected = false;
      Serial.print("No Gas | Value: ");
      Serial.println(gasSensorStatus);
    }

    // Send Event to Web Client with Gas Status and Sensor Value
    events.send(gasStatus.c_str(), "gas-status", millis());
    events.send(gasValue.c_str(), "gas-value", millis());

    // Control alarm based on gas sensor status
    controlAlarm();

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup()
{
  // Disable brownout detection
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  pinMode(fireSensorPin, INPUT);
  pinMode(gasSensorPin, INPUT);
  // pinMode(pirSensorPin, INPUT);

  initWiFi();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  server.addHandler(&events);

  AsyncElegantOTA.begin(&server); // Start ElegantOTA

  server.begin();

  // Create task for fire sensor
  xTaskCreatePinnedToCore(
      TaskFire,        // Function to be executed as task
      "TaskFire",      // Name for this task
      8192,            // Task stack size (can be adjusted)
      NULL,            // Parameters passed to task
      1,               // Task priority (higher value means higher priority)
      &TaskFireHandle, // Handle for this task
      0                // Core where task will run (0 or 1)
  );

  // Create task for gas sensor
  xTaskCreatePinnedToCore(
      TaskGas,        // Function to be executed as task
      "TaskGas",      // Name for this task
      8192,           // Task stack size (can be adjusted)
      NULL,           // Parameters passed to task
      1,              // Task priority (higher value means higher priority)
      &TaskGasHandle, // Handle for this task
      0               // Core where task will run (0 or 1)
  );
}

void loop()
{
  // Main loop doesn't contain code because all logic is moved to tasks
}
