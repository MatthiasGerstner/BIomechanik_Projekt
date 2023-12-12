#include <Arduino.h>
#include <Adafruit_ADS1X15.h> // Including the ADS1115 library by Adafruit
#include <SPI.h>

#define Pressure_Sensor_Pin A1
#define O2_Sensor_Pin A0
#define OOM202 0

float measurePressure();
float measureO2();
void calibrateO2();
float enterNumber(String message);

Adafruit_ADS1115 ads; // creating ADS1115 object instance to access its member functions

// Global Variables
int16_t adc_OOM202;
float mV_OOM202, mV_avgValue;
float current_O2Percent = 0.00;
float mV_baselineO2 = 9.19; // millivolt value for Ambient Air (normally for around 20.9% O2)
const float ambientAir_O2Percent = 20.9;
float ambientAir_O2Percent_input = 0.00;
float V, P; // Voltage & Pressure

void setup()
{
  Serial.begin(9600);
  ads.setGain(GAIN_SIXTEEN); // 16x gain  +/- 0.256V  1 bit =  0.0078125m
  ads.begin();               // Initializing the ADS1115
}
/*
void loop()
{
  float pressure = measurePressure();
  float o2 = measureO2();

  Serial.print("Pressure:");
  Serial.print(pressure, 1);
  Serial.println(" KPa");

  Serial.print("O2:");
  Serial.print(o2, 1);
  Serial.println(" %");

  Serial.println();
  delay(1000);
}
*/
void loop()
{
  ambientAir_O2Percent_input = enterNumber("Enter current ambient O2 percentage: ");
  Serial.print("Ambient Air O2 Percentage: ");
  Serial.println(ambientAir_O2Percent_input);
  calibrateO2();
  Serial.print("O2 Preasure:  ");
  Serial.println(measureO2());
  delay(1000);
}

float measurePressure()
{

  // Connect sensor’s output (SIGNAL) to Analog 0
  V = analogRead(Pressure_Sensor_Pin) * 4.5 / 1024;
  P = (V) * 250;

  return P;
}

float measureO2()
{

  adc_OOM202 = ads.readADC_SingleEnded(OOM202); // Read the ADC Value of OOM202 Sensor
  mV_OOM202 = ads.computeVolts(adc_OOM202);     // Compute the voltage Input from OOM202 Sensor
  mV_OOM202 = mV_OOM202 * 1000;                 // Convert volatge computed to millivolt (mV)

  current_O2Percent = ((mV_OOM202 / mV_baselineO2) * ambientAir_O2Percent); // Convert analog voltage to percentage

  return current_O2Percent;
}

void calibrateO2()
{
  float error = 0.00, baseline = 0.00;
  for (int count = 0; count < 100; count++)
  {
    measureO2();
    mV_avgValue += mV_OOM202;
    delay(10);
  }
  mV_avgValue = mV_avgValue / 100;
  mV_baselineO2 = mV_avgValue;

  baseline = ((mV_avgValue * ambientAir_O2Percent) / ambientAir_O2Percent_input);

  if (mV_baselineO2 > baseline)
    error = mV_baselineO2 - baseline;
  if (baseline > mV_baselineO2)
    error = baseline - mV_baselineO2;

  mV_baselineO2 -= error;
  Serial.print("Setting new baseline: ");
  Serial.println(mV_baselineO2);
  delay(1000);
  return;
}

float enterNumber(String message)
{
  float number;
  bool numberFinished = false;
  String inputString = ""; // Use a String to accumulate input characters

  Serial.println(message);

  while (!numberFinished) {
    if (Serial.available() > 0) {
      char inputChar = (char)Serial.read(); // Read the incoming character

      // Check for newline character, which indicates the end of user input
      if (inputChar == '\n') {
        numberFinished = true; // Set the flag to true to exit the loop
      } else {
        // Append the incoming character to the input string
        inputString += inputChar;
      }
    }
  }

  // Once the input is finished, convert the input string to a float
  number = inputString.toFloat();

  return number;
}