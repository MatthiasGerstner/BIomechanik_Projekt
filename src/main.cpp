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
void calculateDev();
void calculateDev2bar();

Adafruit_ADS1115 ads; // creating ADS1115 object instance to access its member functions

// Global Variables
int16_t adc_OOM202;
float mV_OOM202, mV_avgValue;
float current_O2Percent = 0.00;
float mV_baselineO2 = 9.19; // millivolt value for Ambient Air (normally for around 20.9% O2)
const float ambientAir_O2Percent = 20.9;
const float ambientAir_O2Percent_2bar = 41.8;
float ambientAir_O2Percent_input = 0.00;
float V, P; // Voltage & Pressure
const float OffSetPres = 0.483; //Calibration note DFRobot

const int ARRAY_SIZE_O2 = 100; //save 100 measured Values 
const int ARRAY_SIZE_ABS = 100; //save 100 measured Values 
float O2Values[ARRAY_SIZE_O2];

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
  
    // ambientAir_O2Percent_input = enterNumber("Enter current ambient O2 percentage: ");
    // Serial.print("Ambient Air O2 Percentage: ");
    // Serial.println(ambientAir_O2Percent_input);
    measureO2();
    calibrateO2();
    calculateDev();
    Serial.print("O2 Preasure:  ");
    Serial.println(measureO2());
    Serial.print("absolut Preasure in bar:  ");
    Serial.println(measurePressure());
    // storeO2Value();


  delay(1000);

  if (P >= 2)
  {
    calculateDev2bar();
  }

 /*
 for(int i = 0; i < ARRAY_SIZE_O2; i++) {
    Serial.print("Element an Index ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(O2Values[i]);

    delay(1000);
  }
  */
  
}


/*
void storeO2Value()
{
  int currentIndex = 0;   //Current index for saving the next value in the array.

  if (currentIndex < ARRAY_SIZE_O2) 
  {
    O2Values[currentIndex] = mV_OOM202;
    currentIndex++;
  } else 
    {
    for (int i = 0; i < ARRAY_SIZE_O2 - 1; i++)
      {
      O2Values[i] = O2Values[i + 1];
      }
    O2Values[ARRAY_SIZE_O2 - 1] = mV_OOM202;
    }
  return;
}
*/


float measurePressure()
{

  // Connect sensor’s output (SIGNAL) to Analog 0

  // Änderungen aus https://www.codrey.com/arduino-projects/how-to-play-with-a-water-pressure-sensor/
  
  V = analogRead(Pressure_Sensor_Pin) * 4.5 / 1024;
  P = ((V) * 245)/100;    //Pressure in bar
 // P = (V - OffSetPres) * 245;
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

/* originale calibrate Funktion 

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
*/

//versuch calibrate function --> wird zu Average funktion 
void calibrateO2()
{
  // float O2Dev = 0.00;
  // float error = 0.00, baseline = 0.00;
  for (int count = 0; count < 100; count++)
  {
    measureO2();
    mV_avgValue += mV_OOM202;
    delay(10);
  }
  mV_avgValue = mV_avgValue / 100;
  mV_baselineO2 = mV_avgValue;  //MW der U -Werte

  // baseline = ((mV_avgValue * ambientAir_O2Percent) / ambientAir_O2Percent_input);
 
  return;
}

void calculateDev()
{
  float O2Dev1 = 0.00;
  float O2Dev = 0.00;
  if (current_O2Percent != ambientAir_O2Percent)  
  O2Dev1 = ((current_O2Percent - ambientAir_O2Percent) / ambientAir_O2Percent) * 100;   //Deviation from 20.9 in %
  O2Dev = abs(O2Dev1);
  Serial.print("Abweichung vom Sollwert in %: ");
  Serial.println(O2Dev);
  
  if (O2Dev >= 10)
    Serial.print("Error, Abweichung zu groß ");
    
  return;
}

// Frage: brauchen wir eine 2te measureO2 Funktion speziell für die Messung bei höheren Umgebungsdrücken?
//  current_O2Percent = ((mV_OOM202 / mV_baselineO2) * ambientAir_O2Percent); // Convert analog voltage to percentage
// calculate Dev 2bar
void calculateDev2bar()
{
  float O2Dev2 = 0.00;
  float O2Dev2bar = 0.00;
  if (current_O2Percent != ambientAir_O2Percent_2bar)  
  O2Dev2 = ((current_O2Percent - ambientAir_O2Percent_2bar) / ambientAir_O2Percent_2bar) * 100;   //Deviation from 41.8 in %
  O2Dev2bar = abs(O2Dev2);
  Serial.print("Abweichung vom Sollwert 2bar in %: ");
  Serial.println(O2Dev2bar);
  
  if (O2Dev2bar >= 10)
    Serial.print("Error, Abweichung zu groß ");
    
  return;
}

/*
float measureO2()
{ 
  
  adc_OOM202 = ads.readADC_SingleEnded(OOM202); // Read the ADC Value of OOM202 Sensor
  mV_OOM202 = ads.computeVolts(adc_OOM202);     // Compute the voltage Input from OOM202 Sensor
  mV_OOM202 = mV_OOM202 * 1000;                 // Convert volatge computed to millivolt (mV)
  
return mV_OOM202;
}
*/

/* 
void storeO2Value()
{
  int currentIndex = 0;   //Current index for saving the next value in the array.

  if (currentIndex < ARRAY_SIZE_O2) 
  {
    O2Values[currentIndex] = current_O2Percent;
    currentIndex++;
  } else 
    {
    for (int i = 0; i < ARRAY_SIZE_O2 - 1; i++)
      {
      O2Values[i] = O2Values[i + 1];
      }
    O2Values[ARRAY_SIZE_O2 - 1] = current_O2Percent;
    }
  Serial.println("O2Values[currentIndex]");
  return;
}

*/




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


