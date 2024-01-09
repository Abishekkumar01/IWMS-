#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "MASTER"; // Enter your WIFI SSID
const char* password = "luckydumpy"; // Enter your WIFI Password

#define BOTtoken "6457478878:AAG1fhdmBBkmNXQyUJEhOrPe06liq3f55qc"
#define CHAT_ID "795644234"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

float previousCapacity = -1.0;

const int trigPin1 = 23; // D2 on ESP8266
const int echoPin1 = 22; // D1 on ESP8266

const int ledFull = 2; // D3 on ESP8266

const int trigPin2 = 18; // D8 on ESP8266
const int echoPin2 = 19; // D7 on ESP8266

Servo servo;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  pinMode(ledFull, OUTPUT);
  digitalWrite(ledFull, LOW);

  servo.attach(5); // GPIO5 for servo on ESP32

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Wifi Connected!", "");
  bot.sendMessage(CHAT_ID, "System has Started!!", "");
}

float distance_calc(int trigPin, int echoPin) {
  // Trigger a pulse to the HC-SR04
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the pulse on the echo pin
  long duration = pulseIn(echoPin, HIGH);

  // Convert the duration to distance (in centimeters)
  // Speed of sound at sea level is approximately 343 meters per second (or 0.0343 cm/µs)
  // Divide the duration by 2 because the sound wave travels to the object and back
  float distance_cm = (duration / 2) * 0.0343;

  return distance_cm;
}

// Function to calculate the average distance with anomaly detection
float distance_avg() {
  float sum = 0;
  int numReadings = 3;
  int validReadings = 0;

  for (int i = 0; i < numReadings; i++) {
    float reading = distance_calc(trigPin1, echoPin1);
    // Check for anomalies (values above 300 cm or negative readings)
    if (reading > 0 && reading <= 300) {
      sum += reading;
      validReadings++;
    }
  }

  if (validReadings == 0) {
    return -1.0; // No valid readings, return -1 to indicate an error
  } else {
    return sum / validReadings; // Calculate and return the average
  }
}

void sendCapacityAlert(float capacity, String chatID) {
  if (capacity <= 5 && previousCapacity > 5) {
    digitalWrite(ledFull, HIGH);
    Serial.println("100%");
    bot.sendMessage(chatID, "ALERT 100%! DUSTBIN FULL PLEASE DUMP THE TRASH LOCATION: AMITY UNIVERSITY");
  } else if (capacity <= 8 && capacity > 5 && previousCapacity > 8) {
    Serial.println("75%");
    bot.sendMessage(chatID, "ALERT 75%! DUSTBIN LOCATION: AMITY UNIVERSITY");
  } else if (capacity <= 11 && capacity > 8 && previousCapacity > 11) {
    Serial.println("50%");
    bot.sendMessage(chatID, "ALERT 50%! DUSTBIN LOCATION: AMITY UNIVERSITY");
  } else if (capacity <= 17 && capacity > 11 && previousCapacity > 17) {
    Serial.println("25%");
    bot.sendMessage(chatID, "ALERT 25%! DUSTBIN LOCATION: AMITY UNIVERSITY");
  } else if (capacity > 17 && previousCapacity <= 17) {
    digitalWrite(ledFull, LOW);
  }
}


void loop() {
 float average_distance = distance_avg();

  if (average_distance >= 0) {

    if (average_distance != previousCapacity) {
      // Send the capacity alert message
      sendCapacityAlert(average_distance, CHAT_ID); // Replace with your chat ID
      previousCapacity = average_distance;
      if (average_distance > 6)  { // capacity check point 1;
        // Check if the distance is less than 20 cm
        float distance2 = distance_calc(trigPin2, echoPin2);
        Serial.println("D@");
        Serial.println(distance2);
        if (distance2 < 20) {
          // Rotate the servo to 90 degrees
          servo.write(160);
          delay(5000); // Wait for 5 seconds
          // Bring the servo back to the initial point (e.g., 0 degrees)
          servo.write(0);
        }
      }
    }

    // Print the average distance to the serial monitor
    Serial.print("Average Distance: ");
    Serial.print(average_distance);
    Serial.println(" cm");
    
  } else {
    // Handle error or no valid readings
    Serial.println("Error or No Valid Readings");
  }

  // Wait before taking the next measurement
  delay(1000); // You can adjust the delay as needed
}