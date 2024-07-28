#include <DHT.h>
String ch;

// Constants for anemometer
volatile byte rpmcount;
volatile unsigned long last_micros;
unsigned long timeold;
unsigned long timemeasure = 25.00; // seconds
int GPIO_pulse = 2; // Arduino = D2
float rpm, rps;
float radius = 0.1; // meters - measure of the length of each the anemometer wing
float velocity_kmh;
float velocity_ms;
float omega = 0;
float calibration_value = 2.0;

// Constants for DHT sensor
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Constants for MQ-137 sensor
#define MQ137_PIN A0 // Pin connected to MQ-137 sensor
float R0 = 10.0; // Reference resistance value in clean condition

int redLED = 9;
int greenLED = 10;
int threshold = 0;
int led = 0;

void setupLEDs() {
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
}

int extractThreshold(String ch) {
  String keyword = "threshold:";
  int index = ch.indexOf(keyword); 
  if (index != -1) { // Jika "threshold:" ditemukan
    int startIndex = index + keyword.length(); // Mulai dari karakter setelah "threshold:"
    String thresholdStr = ch.substring(startIndex); // Ekstrak angka setelah "threshold:"
    return thresholdStr.toInt(); // Konversi string ke integer
  }
  
  return -1; // Jika "threshold:" tidak ditemukan, kembalikan -1
}

int extractled(String ch) {
  String keyword1 = "LED:";
  int index1 = ch.indexOf(keyword1);
  if (index1 != -1) { // Jika "threshold:" ditemukan
    int startIndex1 = index1 + keyword1.length(); // Mulai dari karakter setelah "threshold:"
    String ledStr = ch.substring(startIndex1); // Ekstrak angka setelah "threshold:"
    return ledStr.toInt(); // Konversi string ke integer
  }
  
  return -1; // Jika "threshold:" tidak ditemukan, kembalikan -1
}

void handleSerialCommand() {
  ch = ""; // Reset string ch
  while (Serial.available()) {
    char inChar = (char)Serial.read(); // Read character by character
    if (inChar == '\n') { // If newline character is found, break
      break;
    }
    ch += inChar; // Append character to string ch
  }

  int extractedThreshold = extractThreshold(ch);
  if (extractedThreshold != -1) {
    threshold = extractedThreshold;
  }

  int extractedled = extractled(ch);
  if (extractedled != -1) {
    led = extractedled;
  }
  
  if (led == 1) {
    digitalWrite(redLED, HIGH);
  } else if (led == 2) {
    digitalWrite(redLED, LOW);
  } else if (led == 3) {
    digitalWrite(greenLED, HIGH);
  } else if (led == 4) {
    digitalWrite(greenLED, LOW);
  }
}

void setup() {
  setupLEDs();
  pinMode(GPIO_pulse, INPUT_PULLUP);
  digitalWrite(GPIO_pulse, LOW);

  Serial.begin(115200); // Match baud rate with NodeMCU
  dht.begin();

  detachInterrupt(digitalPinToInterrupt(GPIO_pulse));
  attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING);
  rpmcount = 0;
  rpm = 0;
  timeold = 0;
}

void loop() {
  handleSerialCommand();
  delay(1000);

  // Read data from DHT sensor
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // Read data from MQ-137 sensor
  int analogValue = analogRead(MQ137_PIN); // Read analog value from MQ-137 pin
  float resistance = (1023.0 / analogValue) - 1; // Convert analog value to resistance
  float ppm = calculatePPM(resistance); // Calculate ammonia concentration in PPM

  // Measure RPM
  if ((millis() - timeold) >= timemeasure * 1000) {
    detachInterrupt(digitalPinToInterrupt(GPIO_pulse));
    rps = float(rpmcount) / float(timemeasure);
    rpm = 60 * rps;
    omega = 2 * PI * rps;
    velocity_ms = omega * radius * calibration_value;
    velocity_kmh = velocity_ms * 3.6;

    // Send data to NodeMCU in JSON format
    String data = "{\"Humidity\":" + String(hum) + ",\"Temp\":" + String(temp) +
                  ",\"PPM\":" + String(ppm) + ",\"rps\":" + String(rps) + ",\"rpm\":" + String(rpm) +
                  ",\"velocity_ms\":" + String(velocity_ms) + ",\"velocity_kmh\":" + String(velocity_kmh) + "}";
    Serial.println(data);

    timeold = millis();
    rpmcount = 0;
    attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING);
  }
    if (ppm > threshold) {
    digitalWrite(redLED, HIGH);

  } else {
    digitalWrite(redLED, LOW);
  }
}

float calculatePPM(float resistance) {
  // This function converts resistance into ammonia concentration in PPM
  // You may need to adjust these constants based on the sensor datasheet or reference measurements
  return pow((resistance / R0), -1.85) * 27.28;
}

void rpm_anemometer() {
  if (long(micros() - last_micros) >= 5000) {
    rpmcount++;
    last_micros = micros();
  }
}
