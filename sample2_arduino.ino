#include <LiquidCrystal.h>
#include <Wire.h>
#include <DHT.h>  // Correct library for DHT sensor
#include <SPI.h>
#include <LoRa.h>

// DHT sensor settings
#define DHTPIN A1          // Pin connected to the DHT sensor
#define DHTTYPE DHT11      // DHT 11 or DHT 22
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor

// Pin definitions
int mos = A0;     // Soil moisture sensor pin
int relay = A2;   // Relay pin for controlling irrigation

// Variables
int tempc = 0, humc = 0;
String moss = "";
int counter = 0;

LiquidCrystal lcd(6, 7, 5, 4, 3, 8); // Initialize the LCD

void setup() {
  // Start serial communication and initialize pins
  Serial.begin(9600);
  pinMode(mos, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);  // Initially, set the relay to LOW (off)

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Lora Based Smart");
  lcd.setCursor(0, 1);
  lcd.print("Irrigation System");
  delay(3000);
  lcd.clear();
  lcd.print("For Remote Areas");
  delay(3000);

  // Initialize LoRa communication
  Serial.println("LoRa Sender");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    lcd.clear();
    lcd.print("LoRa Failed");
    while (1);  // Stay here if LoRa initialization fails
  }

  lcd.clear();
  lcd.print("LoRa Initialised");
  lcd.setCursor(0, 1);
  lcd.print("Successfully");
  delay(3000);

  // Display labels on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");    // Temperature label
  lcd.setCursor(8, 0);
  lcd.print("H:");    // Humidity label
  lcd.setCursor(0, 1);
  lcd.print("M:");    // Moisture label
  lcd.setCursor(8, 1);
  lcd.print("P:");    // Pump label
  
  dht.begin();  // Initialize DHT sensor
}

void loop() {
  // Read temperature and humidity from DHT sensor
  humc = dht.readHumidity();
  tempc = dht.readTemperature();

  // Display temperature and humidity on the LCD
  lcd.setCursor(2, 0);
  convertl(tempc);  // Display temperature
  lcd.setCursor(10, 0);
  convertl(humc);   // Display humidity

  // Check soil moisture
  moss = "";
  if (digitalRead(mos) == LOW) {
    lcd.setCursor(2, 1);
    lcd.print("Wet ");
    digitalWrite(relay, LOW);  // Turn off irrigation
    lcd.setCursor(10, 1);
    lcd.print("OFF ");
    moss = "Wet";
  }
  if (digitalRead(mos) == HIGH) {
    lcd.setCursor(2, 1);
    lcd.print("Dry ");
    digitalWrite(relay, HIGH);  // Turn on irrigation
    lcd.setCursor(10, 1);
    lcd.print("ON  ");
    moss = "Dry";
  }

  // Send data via LoRa every 200 cycles
  if (counter >= 200) {
    counter = 0;
    lcd.setCursor(14, 1);
    lcd.print("LS");  // Indicate data sending

    // Send packet via LoRa
    LoRa.beginPacket();
    LoRa.print(int(tempc));  // Send temperature data
    LoRa.print(",");
    LoRa.print(int(humc));   // Send humidity data
    LoRa.print(",");
    LoRa.print(moss);        // Send moisture status
    LoRa.endPacket();

    delay(5000);  // Wait for a while before sending the next packet

    lcd.setCursor(14, 1);
    lcd.print("  ");  // Clear "LS"
  }

  counter++;  // Increment the counter
  delay(100);  // Delay for LCD update
}

// Function to convert values to LCD format
void convertl(unsigned int value) {
  unsigned int a, b, c, d, e, f, g, h;

  a = value / 10000;
  b = value % 10000;
  c = b / 1000;
  d = b % 1000;
  e = d / 100;
  f = d % 100;
  g = f / 10;
  h = f % 10;

  a = a | 0x30;
  c = c | 0x30;
  e = e | 0x30;
  g = g | 0x30;
  h = h | 0x30;

  lcd.write(c); lcd.write(e); lcd.write(g); lcd.write(h);  // Display value on LCD
}
