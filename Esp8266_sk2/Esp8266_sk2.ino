#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Servo.h>



const char* ssid = "MASTER";// Enter your WIFI SSID
const char* password = "luckydumpy"; // Enter your WIFI Password


#define BOTtoken "6457478878:AAG1fhdmBBkmNXQyUJEhOrPe06liq3f55qc" // Enter the bottoken you got from botfather
#define CHAT_ID "795644234" // Enter your chatID you got from chatid bot


X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Global variables
float previousCapacity = -1.0; // Initialize with an invalid value

// Define the pins for the HC-SR04 sensor
const int trigPin1 = D2; // Trigger pin
const int echoPin1 = D1; // Echo pin

const int ledFull = D3; // LED pin for full condition

// Define the pins for the second HC-SR04 sensor
const int trigPin2 = D8; // Trigger pin for the second sensor - green
const int echoPin2 = D7; // Echo pin for the second sensor - yell

// Define the servo
Servo servo;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Hello!");

  // Set the trigger pin as an OUTPUT
  pinMode(trigPin1, OUTPUT);
  // Set the echo pin as an INPUT
  pinMode(echoPin1, INPUT);

  // Set the trigger pin as an OUTPUT
  pinMode(trigPin2, OUTPUT);
  // Set the echo pin as an INPUT
  pinMode(echoPin2, INPUT);

  pinMode(ledFull, OUTPUT);
  digitalWrite(ledFull, LOW);


  // Attach the servo to D6
  servo.attach(D6);

  configTime(0, 0, "pool.ntp.org");      
  client.setTrustAnchors(&cert);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int a = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    a++;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Wifi Connected!", "");
  bot.sendMessage(CHAT_ID, "System has Started!!", "");

}

// Function to calculate distance
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



// void setup() {
//   pinMode(D3, OUTPUT);
//   digitalWrite(D3, LOW);
//   Serial.begin(9600);
//   Serial.print("HI");
// }

// void loop() {

// }