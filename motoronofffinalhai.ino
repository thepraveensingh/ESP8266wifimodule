#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define BLYNK_TEMPLATE_ID "TMPL3MJa8-wnE"
#define BLYNK_TEMPLATE_NAME "Plant monitor"
#define BLYNK_AUTH_TOKEN "icQK8GXg4kACjt6d0IePczgXAV7XyOB9"

char auth[] = "icQK8GXg4kACjt6d0IePczgXAV7XyOB9";  
char ssid[] = "Redmi";                   
char pass[] = "12345670";                        

DHT dht(D4, DHT11); // DHT sensor pin, sensor type (D4 DHT11 Temperature Sensor)
BlynkTimer timer;

// Define component pins
#define soil A0         // A0 Soil Moisture Sensor
#define PIR D5          // D5 PIR Motion Sensor

int PIR_ToggleValue;
int buttonState;
int motorState = LOW;  // Initialize motor state to off

void setup() {
  Serial.begin(9600);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(PIR, INPUT);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(11, 1);
  lcd.print("W:OFF");
  
  // Call the functions
  timer.setInterval(1000L, soilMoistureSensor);  // Call soilMoistureSensor every 1 second
  timer.setInterval(1000L, DHT11sensor);         // Call DHT11sensor every 1 second
}

// Get the DHT11 sensor values
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
}

// Get the soil moisture values
void soilMoistureSensor() {
  int value = analogRead(soil);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  if (value < 20) {
    digitalWrite(D0, HIGH);  // Turn the motor on
    digitalWrite(D1, LOW);
  } else {
    digitalWrite(D0, LOW);   // Turn the motor off
    digitalWrite(D1, LOW);   // Ensure D1 remains LOW when motor is off
  }

  Blynk.virtualWrite(V3, value);
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print(" ");
}

// Get the PIR sensor values
void PIRsensor() {
  bool value = digitalRead(PIR);
  if (value) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");
    WidgetLED LED(V5);
    LED.on();
  } else {
    WidgetLED LED(V5);
    LED.off();
  }  
}

// Handle button press to control motor
BLYNK_WRITE(V2) {
  buttonState = param.asInt();
  
}

// Toggle PIR sensor value
BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();  
}

void loop() {
  if (PIR_ToggleValue == 1) {
    PIRsensor();
  }
  if (buttonState == 1) {
    // Button pressed, toggle motor state
    motorState = !motorState;
    digitalWrite(D0, motorState);
    digitalWrite(D1, LOW);
  }
  Blynk.run(); // Run the Blynk library
  timer.run(); // Run the Blynk timer
}
