#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <LoRa.h>

LiquidCrystal lcd(13, 12, 14, 27, 26, 25);  // Adjust pins for your ESP32 model

#define ss   5   // LoRa SS Pin
#define rst  15  // LoRa Reset Pin
#define dio0 2   // LoRa DIO0 Pin


const char *ssid = "iotserver";
const char *password = "iotserver123";

int tempc = 0, humc = 0;
String moss = "";
HTTPClient http;

void wifiinit() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Connecting to WiFi...");
  wifiinit();

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    lcd.clear();
    lcd.print("LoRa Failed");
    while (1);
  }

  lcd.clear();
  lcd.print("LoRa Initialised");
  lcd.setCursor(0, 1);
  lcd.print("Successfully");
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");    
  lcd.setCursor(8, 0);
  lcd.print("H:");    
  lcd.setCursor(0, 1);
  lcd.print("M:");    
  lcd.setCursor(8, 1);
  lcd.print("P:");    
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    String loradata = LoRa.readString();
    int i1 = loradata.indexOf(',');
    int i2 = loradata.indexOf(',', i1 + 1);
    
    String temps = loradata.substring(0, i1);
    String hums = loradata.substring(i1 + 1, i2);
    moss = loradata.substring(i2 + 1);

    tempc = temps.toInt();
    humc = hums.toInt();

    lcd.setCursor(2, 0);
    convertl(tempc);
    lcd.setCursor(10, 0);
    convertl(humc);
    lcd.setCursor(2, 1);
    lcd.print(moss);

    if (moss == "Wet") {
      lcd.setCursor(10, 1);
      lcd.print("OFF");
    } else if (moss == "Dry") {
      lcd.setCursor(10, 1);
      lcd.print("ON ");
    }
  
    // Send data to server
    String pf_data = "http://projectsfactoryserver.in/storedata.php?name=" + String("iot538") + "&s1=" + String(tempc) + "&s2=" + String(humc) + "&s3=" + moss;
    http.begin(pf_data);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
  }

  delay(10);
}

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

  lcd.write(c);
  lcd.write(e);
  lcd.write(g);
  lcd.write(h);
}
