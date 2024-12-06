#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int adcPin34 = 34; // ADC pin for resistance measurement
const int adcPin35 = 35; // ADC pin for external voltage measurement

const float Rk = 9130.0;       // Known resistor value (ohms)
const float Vin = 3.3;         // ESP32 input voltage (volts)
const int samples = 50;        // Number of samples for averaging
const float R1 = 9780.0;       // Resistor from GPIO 35 to GND (ohms)
const float R2 = 9570.0;       // Resistor from GPIO 35 to Vext (ohms)
const float VextOffset = 0.28; // Calibration offset for external voltage (volts)

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");
  delay(2000);
}

void loop() {
  // Resistance measurement
  float adcValue34 = averageADC(adcPin34);
  float Vout34 = (adcValue34 / 4095.0) * Vin;
  float calibratedVout34 = Vout34 - 0.075;
  float Rx = calculateResistance(calibratedVout34);

  // External voltage measurement
  float adcValue35 = averageADC(adcPin35);
  float Vout35 = (adcValue35 / 4095.0) * Vin;
  float Vext = Vout35 > 0 ? (calculateExternalVoltage(Vout35) + VextOffset) : 0.0;

  // Serial monitor output
  if (Rx > 0) {
    Serial.print("R: ");
    Serial.print(Rx, 2);
    Serial.print(" Ohm, Vout: ");
    Serial.println(calibratedVout34, 3);
  } else {
    Serial.println("R: NC");
  }
  Serial.print("Vext: ");
  Serial.println(Vext, 3);

  // LCD display output
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R: ");
  if (Rx > 0) {
    lcd.print(Rx, 1);
    lcd.print(" Ohm");
  } else {
    lcd.print("NC");
  }
  lcd.setCursor(0, 1);
  lcd.print("V: ");
  lcd.print(Vext, 2);
  lcd.print(" V");

  delay(1000);
}

float averageADC(int pin) {
  long adcSum = 0;
  for (int i = 0; i < samples; i++) {
    adcSum += analogRead(pin);
    delay(2);
  }
  return adcSum / (float)samples;
}

float calculateResistance(float Vout) {
  if (Vout <= 0 || Vout >= Vin) return 0.0;
  return (Vin / Vout - 1) * Rk;
}

float calculateExternalVoltage(float Vout) {
  return Vout * (R1 + R2) / R2;
}
