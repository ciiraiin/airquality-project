#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define mq135Pin 0    // GPIO 0 ESP32C3
#define mq7Pin 1      // GPIO 1 ESP32C3
#define DHT22_PIN 3   // GPIO 3 ESP32C3
#define DHTTYPE DHT22
#define SCREEN_WIDTH 128 // Lebar OLED dalam piksel
#define SCREEN_HEIGHT 64 // Tinggi OLED dalam piksel

const int RCO = 10000;    // Resistor beban di MQ-7 (10k ohm)
const int RCO2 = 20000;   // Resistor beban di MQ-135 (20k ohm)

const int RoCO = 7400;    // Resistansi MQ7 di udara bersih (perkiraan)
const int RoCO2 = 240000; // Resistansi MQ135 di udara bersih (perkiraan)
double ppm = 428.11;      // ppm co2 di udara (lihat di co2.earth)

const float aCO = 99.042;      // a dan b didapat dari regresi linier nilai grafik datasheet sensor
const float bCO = -1.518;
const float aCO2 = 110.7432567;
const float bCO2 = -2.856935538;

const float VsCO = 4.5;  // tegangan sumber yang terukur dengan multimeter 
const float VsCO2 = 4.3; // bisa juga ditulis dengan 5.0 V sesuai yang terbaca di ESP

const int R1 = 1000;  // Resistor divider
const int R2 = 2000;  // Resistor divider

DHT dht (DHT22_PIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  pinMode(mq135Pin, INPUT);
  pinMode(mq7Pin, INPUT);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
     Serial.println(F("SSD1306 allocation failed"));
  for(;;);
  }
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  float adcCO  = analogRead(mq7Pin);     // nilai ADC raw CO
  float adcCO2 = analogRead(mq135Pin);   // nilai ADC raw CO2
  
  float vRefCO = VsCO * R2 / (R1 + R2);     // tegangan dari voltage divider (<=3.3V)
  float vRefCO2 = VsCO2 * R2 / (R1 + R2);

  float VadcCO  = (adcCO / 4095.0) * vRefCO;   // tegangan terukur di pin A0 MQ7
  float VadcCO2 = (adcCO2 / 4095.0) * vRefCO2; // tegangan terukur di pin A0 MQ135

  float vCO  = VadcCO * 1.5;   // tegangan asli sensor MQ7 (1.5 didapat dari (R1+R2)/R2)
  float vCO2 = VadcCO2 * 1.5;  // tegangan asli sensor MQ135

  // === Menghitung nilai Rs CO dan CO2 ===
  float RsCO  = ((VsCO - vCO) / vCO) * RCO;      // Resistansi internal MQ7 saat terpapar gas CO
  float RsCO2 = ((VsCO2 - vCO2) / vCO2) * RCO2;  // Resistansi internal MQ135 saat terpapar gas CO2

  // === Menghitung rasio CO dan CO2 ===
  float ratioCO  = RsCO / RoCO; 
  float ratioCO2 = RsCO2 / RoCO2;

  // Konversi rasio ke PPM menggunakan rumus kurva datasheet
  float ppmCO  = aCO * pow(ratioCO, bCO);
  float ppmCO2 = aCO2 * pow(ratioCO2, bCO2);

  Serial.print("Temperature : "); Serial.print(t);   Serial.print(" C");
  Serial.print(" Humidity : ");   Serial.print(h); Serial.println(" %");
  Serial.print("CO ");
  Serial.print("adc: ");  Serial.print(adcCO);
  Serial.print(" v: ");   Serial.print(vCO);      Serial.print("V");
  Serial.print(" Rt: ");  Serial.print(ratioCO);
  Serial.print(" gas: "); Serial.print(ppmCO);    Serial.println(" ppm");
  Serial.print("CO2 ");
  Serial.print("adc: ");  Serial.print(adcCO2);
  Serial.print(" v: ");   Serial.print(vCO2);     Serial.print("V");
  Serial.print(" Rt: ");  Serial.print(ratioCO2);
  Serial.print(" gas: "); Serial.print(ppmCO2);   Serial.println(" ppm");
  Serial.println("");

   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(10, 0);
   display.println("Air Quality Sensor");
   display.setCursor(0, 20);
   display.print("CO  : ");  display.print(ppmCO);   display.println(" ppm");
   display.print("CO2 : ");  display.print(ppmCO2);  display.println(" ppm");
   display.print("Temp: ");  display.print(t);       display.println(" C");
   display.print("Humd: ");  display.print(h);       display.println(" %");
   display.display();
   delay(1000);
}