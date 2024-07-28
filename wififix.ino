#include <ESP8266WiFi.h>

const char* ssid = "rumah";
const char* password = "rumah123";
const char* serverIP = "192.168.100.10"; // Replace with your server IP
const int serverPort = 4000; // The same port as the TCP server

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
    
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconnecting to server...");
    if (client.connect(serverIP, serverPort)) {
      Serial.println("Reconnected to server");
      break;
    } else {
      Serial.println("Reconnection to server failed");
      delay(5000); // Wait for 5 seconds before trying again
    }
  }
}

void loop() {
  if (client.available()) {
    char ch = client.read();
    Serial.write(ch);
  }
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim(); // Hapus spasi ekstra

    if (data.startsWith("{") && data.endsWith("}")) {
      if (!client.connected()) {
        reconnect(); // Coba kembali jika terputus
      }
      
      if (client.connected()) {
        client.println(data);
      } else {
        Serial.println("Failed to send data. Connection to server lost.");
      }
    } else {
      Serial.println("Invalid data format: " + data);
    }
  }
  if (!client.connected()) {
    reconnect(); // Try to reconnect if disconnected
  }
}