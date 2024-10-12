#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif
#include <Firebase_ESP_Client.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);


/* 1. Define the WiFi credentials */
#define WIFI_SSID "WE_1C415D"
#define WIFI_PASSWORD "cdce9ca7"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBIYJ6E7WANZBIXlZ1t1h1DB-dMcT9mNSA"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "aminfadel2004@gmail.com"
#define USER_PASSWORD "F123321f"

/* 4. Define the RTDB URL */
#define DATABASE_URL "https://my-iot-sys-default-rtdb.europe-west1.firebasedatabase.app/"

#define PHOTORESISTOR_PIN 4

#define UUID 99

String databasePath;

bool ledState;
uint8_t photoResValue;

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

void setup() {
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  timeClient.begin();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;

}

void loop() {
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  timeClient.update();

  // Generate some example data
  float flowrate = random(20, 30);
  float temperature = random(40, 60);
  float pressure = random(40, 60);

  unsigned long epochTime = timeClient.getEpochTime();
Serial.print("Epoch Time: ");
Serial.println(epochTime);
  String flowratePath = "/devices/" + String(UUID) + "/flowrate/" + String(epochTime);
  String temperaturePath = "/devices/" + String(UUID) + "/temperature/" + String(epochTime);
  String pressurePath = "/devices/" + String(UUID) + "/pressure/" + String(epochTime);
  String ledPath = "/devices/" + String(UUID) + "/led_state";

  String Path = "/devices/" + String(UUID) + "/real-time/";
  
  FirebaseJson JSON;
  JSON.set("flowrate", flowrate);
  JSON.set("temperature", temperature);
  JSON.set("pressure", pressure);
  JSON.set("timestamp", epochTime);

  // Send data to Firebase
  if (Firebase.RTDB.setJSON(&fbdo, Path, &JSON)) {
    Serial.println("Sent successfully");
  } else {
    Serial.println("Failed to send");
    Serial.println("Reason: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getBool(&fbdo, ledPath, &ledState)) {
    digitalWrite(2, !ledState);
  } else {
    Serial.println("Reason: " + fbdo.errorReason());
  }

  delay(100);  // Wait for 5 seconds before the next update
}