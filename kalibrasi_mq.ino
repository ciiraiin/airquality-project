#define mq135Pin 0 // GPIO 0 ESP32C3
#define mq7Pin 1   // GPIO 1 ESP32C3

const int RCO = 10000;  // Resistor beban di MQ-7 (10k ohm)
const int RCO2 = 20000; // Resistor beban di MQ-135 (20k ohm)
double ppm = 428.11;    //ppm co2 di udara (lihat di co2.earth)

const float aCO2 = 110.7432567;  // Didapat dari regresi linier grafik datasheet sensor MQ135
const float bCO2 = -2.856935538;

const float VsCO = 4.6;  // tegangan sumber yang terukur dengan multimeter 
const float VsCO2 = 4.4; // bisa juga ditulis dengan 5.0 V sesuai yang terbaca di ESP

const int R1 = 1000;  // Resistor divider
const int R2 = 2000;  // Resistor divider

float RoCO = 0.0; 
float RoCO2 = 0.0; 

void setup() {
  Serial.begin(115200);
  pinMode(mq7Pin, INPUT);
  pinMode(mq135Pin, INPUT);
}

void loop() {
  float adcCO = analogRead(mq7Pin); 
  float adcCO2 = analogRead(mq135Pin);
  
  float vRefCO  = VsCO * R2 / (R1 + R2);       // tegangan dari voltage divider (<=3.3V)
  float vRefCO2 = VsCO2 * R2 / (R1 + R2);

  float VadcCO  = (adcCO / 4095.0) * vRefCO;   // tegangan terukur di pin A0 MQ7
  float VadcCO2 = (adcCO2 / 4095.0) * vRefCO2; // tegangan terukur di pin A0 MQ135

  float vCO  = VadcCO * 1.5;   // tegangan asli sensor MQ7 (1.5 didapat dari (R1+R2)/R2)
  float vCO2 = VadcCO2 * 1.5;  // tegangan asli sensor MQ135

  // === Menghitung nilai Rs CO dan CO2 ===
  float RsCO = ((VsCO - vCO) / vCO) * RCO;      // resistansi internal saat terpapar gas CO
  float RsCO2 = ((VsCO - vCO2) / vCO2) * RCO2;  // resistansi internal saat terpapar gas CO2

  // === Menghitung nilai Ro CO dan CO2 di udara bersih ===
  RoCO = RsCO / 27.5;      // 27.5 adalah rasio sensor MQ7 di saat udara bersih
  float ratio = pow((ppm/aCO2), (1.0/bCO2));
  RoCO2 = RsCO2 / ratio;
  
  Serial.print("CO ");
  Serial.print("adc: "); Serial.print(adcCO);
  Serial.print(" v: "); Serial.print(VadcCO);  Serial.print("V");
  Serial.print(" Rs: "); Serial.print(RsCO);
  Serial.print(" Ro: "); Serial.print(RoCO);  Serial.println(" ohm");
  Serial.print("CO2 ");
  Serial.print("adc: "); Serial.print(adcCO2);
  Serial.print(" v: "); Serial.print(VadcCO2);  Serial.print("V");
  Serial.print(" Rs: "); Serial.print(RsCO2);
  Serial.print(" Ro: "); Serial.print(RoCO2);  Serial.println(" ohm");
  Serial.println("");
  delay(1000);
}