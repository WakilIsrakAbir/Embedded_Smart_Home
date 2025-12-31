#include <Keypad.h>
#include <Servo.h>
#include <DHT.h>

//  PIN SETUP 
#define LDR_PIN A0
#define BUZZER 2

#define TRIG_PIN A5
#define ECHO_PIN A4

#define ROWS 4
#define COLS 4
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 4, 5, 6};  
byte colPins[COLS] = {7, 8, 9, 10};  
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Servo gateServo;
String password = "4444";   // Default password
String inputPassword = " ";

#define GAS_SENSOR A1

#define PIR_PIN 12
#define LED_PIN 13

#define DHTPIN A2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define RELAY_PIN A3  // Fan control

// SETUP
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  gateServo.attach(11);
  gateServo.write(0); // Locked position

  dht.begin();
  Serial.println("=== Smart Home Automation System Started ===");
}

// LOOP 
void loop() {


  //ultrasonic
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Open gate if object/person is close
  if (distance < 20) { 
    Serial.println("🚪 Gate Opening (Ultrasonic Trigger)");
    gateServo.write(90);  
    delay(3000);
    gateServo.write(0);    
  }

  
  // 1. Security with LDR
  int ldrValue = analogRead(LDR_PIN);
  if (ldrValue < 600) { 
    digitalWrite(BUZZER, HIGH);
    Serial.println("⚠ Intruder Detected!");
  } else {
    digitalWrite(BUZZER, LOW);
  }
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // 2. Password Protected Gate 
  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
    if (key == '#') {
      if (inputPassword == password) {
        Serial.println(" Password Correct – Gate Opened");
        gateServo.write(90); 
        delay(3000);
        gateServo.write(0);  
      } else {
        Serial.println(" Wrong Password – Access Denied");
      }
      inputPassword = "";
    } else {
      inputPassword += key;
      Serial.print("Key Pressed: ");
      Serial.println(key);
    }
  }

  // 3. Kitchen Gas Leakage 
  int gasValue = analogRead(GAS_SENSOR);
  if (gasValue > 250) { 
    digitalWrite(BUZZER, HIGH);
    Serial.println("⚠ Gas Leakage Detected!");
  }
  Serial.print("Gas Sensor Value: ");
  Serial.println(gasValue);

  //  4. Automatic Lighting with PIR
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("💡 Light ON (Motion Detected)");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("💡 Light OFF (No Motion)");
  }
  Serial.print("PIR State: ");
  Serial.println(pirState);

  //  5. Automatic Fan Control with DHT11 
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    Serial.print("🌡 Temp: ");
    Serial.print(temp);
    Serial.print(" °C | Humidity: ");
    Serial.print(hum);
    Serial.println(" %");
    Serial.println(temp);

    if (temp > 20) {
      digitalWrite(RELAY_PIN, HIGH); 
      Serial.println(" Fan Status: ON");
    } else {
      digitalWrite(RELAY_PIN, LOW); 
      Serial.println(" Fan Status: OFF");
    }
  } else {
    Serial.println("Error reading DHT11 sensor!");
  }
  

  Serial.println("-----------------------------------");
  delay(3000);
}
