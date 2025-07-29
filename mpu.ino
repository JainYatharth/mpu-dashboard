//#define WIFI_SSID "S21"
//#define WIFI_PASSWORD "hema1234567"
//#define API_KEY "AIzaSyBZsMiDagVrqPcDqqeaMvBGcWpdGsOpOx8"
//#define DATABASE_URL "mpuiot-default-rtdb.asia-southeast1.firebasedatabase.app"
//#define USER_EMAIL "yatharthjain2004@gmail.com"
//#define USER_PASSWORD "firebasempu"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <SparkFunMPU9250-DMP.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Redmi K50i"
#define WIFI_PASSWORD "HardikSardana"
#define API_KEY "AIzaSyBZsMiDagVrqPcDqqeaMvBGcWpdGsOpOx8"
#define DATABASE_URL "mpuiot-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "yatharthjain2004@gmail.com"
#define USER_PASSWORD "firebasempu"

// AP fallback
const char *ap_ssid = "MPU9250-Offline";
const char *ap_password = "12345678";

// ---------- OBJECTS ----------
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
MPU9250_DMP imu;

// ---------- HTML PAGE ----------
const char *html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>MPU9250 Dashboard</title>
  <script>
    var socket;
    window.onload = function() {
      socket = new WebSocket("ws://" + location.hostname + ":81");
      socket.onmessage = function(event) {
        document.getElementById("data").innerHTML = event.data;
      };
    };
  </script>
</head>
<body>
  <h2>MPU9250 Live Data</h2>
  <div id="data">Waiting for data...</div>
</body>
</html>
)rawliteral";

// ---------- FUNCTIONS ----------
void startAccessPoint() {
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: "); Serial.println(IP);

  server.on("/", []() {
    server.send(200, "text/html", html);
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {});
  Serial.println("Offline fallback mode started.");
}

void sendSensorDataToClients(String data) {
  webSocket.broadcastTXT(data);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Sensor init
  if (imu.begin() != INV_SUCCESS) {
    Serial.println("Failed to initialize MPU9250");
    while (1);
  }
  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
  imu.setGyroFSR(2000);
  imu.setAccelFSR(2);
  imu.setSampleRate(10);
  imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL, 10);

  // Try Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected. Starting Firebase mode.");

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  } else {
    Serial.println("\nWi-Fi failed. Starting fallback AP mode.");
    startAccessPoint();
  }
}

// ---------- LOOP ----------
void loop() {
  if (imu.fifoAvailable()) {
    if (imu.dmpUpdateFifo() == INV_SUCCESS) {
      imu.updateAccel();
      imu.updateGyro();

      float gx = imu.calcGyro(imu.gx);
      float gy = imu.calcGyro(imu.gy);
      float gz = imu.calcGyro(imu.gz);
      float ax = imu.calcAccel(imu.ax);
      float ay = imu.calcAccel(imu.ay);
      float az = imu.calcAccel(imu.az);

      String jsonData = "Gyro (°/s): " + String(gx, 2) + ", " + String(gy, 2) + ", " + String(gz, 2) +
                        " | Accel (g): " + String(ax, 2) + ", " + String(ay, 2) + ", " + String(az, 2);

      if (WiFi.status() == WL_CONNECTED) {
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/gyro/x", gx);
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/gyro/y", gy);
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/gyro/z", gz);
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/accel/x", ax);
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/accel/y", ay);
        Firebase.RTDB.setFloat(&fbdo, "/mpu9250/accel/z", az);
        Serial.println("Data sent to Firebase.");
      } else {
        server.handleClient();
        webSocket.loop();
        sendSensorDataToClients(jsonData);
      }
    }
  }

  delay(100);
}