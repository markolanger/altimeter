
// ToDo: RTC, Werte Speichern, Audio?, DeepSleep.

#include <Adafruit_BMP280.h>
#include "M5StickCPlus.h"
#include <Wire.h>  // The BMP uses I2C PINout: G0 - SDA | G26 - SCL
#include "Adafruit_Sensor.h"
Adafruit_BMP280 bme;
float pressureOld        = 0;
float tmp                = 0;
float pressure           = 0;
float alt                = 0;
float altOld             = 0;
float delta              = 0.09;//2.5;
float altGND             = 0;
float altMAX             = 0;
float altDEPLOY          = 0;
float Ding               = 0;
int FlightFlag           = 0;
float NearGroundOffset   = 0.20;
float FreefallDetection  = -0.8;//-50;
int FreefallGNDDetection = 5;
int Freefall             = 0;
int i                    = 0;
int Jump                 = 0;
TFT_eSprite tftSprite    = TFT_eSprite(&M5.Lcd);


void setup() {
  M5.begin();
  M5.lcd.setRotation(3);
  tftSprite.createSprite(240, 135);
  tftSprite.setRotation(3);
  //  M5.Axp.EnableCoulombcounter();
  Wire.begin(0, 26, 100000UL);

  tftSprite.fillSprite(BLACK);
  tftSprite.setCursor(0, 0, 1);
  tftSprite.setTextSize(2);
  tftSprite.printf("ENV Unit(DHT12 and BMP280) test...\r\n");
  if (!bme.begin(0x76)) {
    tftSprite.printf("Could not find a valid BMP280 sensor, check wiring!\r\n");
    while (1);
  }
  tftSprite.pushSprite(5, 5);
  delay(500);
  pressure = bme.readPressure() / 100;
  pressureOld = pressure;
  M5.Beep.tone(4000);
  delay(100);
  M5.Beep.mute();
}

void loop() {
  altOld      = alt;
  pressureOld = pressure;
  tmp         = bme.readTemperature();
  pressure    = bme.readPressure() / 100;  //alt       = bme.readAltitude();

  // FlightFlag 1=Steigen; 2=Freifall; 3=Schirm; 0=Boden

  if ((pressureOld - pressure) > delta && FlightFlag == 0)
  {
    //Steigen
    altGND = pressure;
    FlightFlag = 1;
    alt = bme.readAltitude(altGND);
  }

  if (FlightFlag == 1 && Freefall == 1)
  {
    //    Steigen
    alt = bme.readAltitude(altGND);
    FlightFlag = 2;
  }

  if (FlightFlag == 1)
  {
    //Steigmodus
    alt = bme.readAltitude(altGND);
    FlightFlag = 1;
    Ding = (alt - altOld);
    if (alt > altMAX)
    {
      altMAX = bme.readAltitude(altGND);
    }
    if (Ding < FreefallDetection)
    {
      Serial.printf("1Temperatur: %2.2f% °C  Altitude: %0.1f% m/GND  Pressure: %0.2f hPa  Flight: %1d  A-AO: %0.1f \r\n",
                    tmp, alt, pressure, FlightFlag, Ding);
      Freefall = 1;
    }
  }
  if (FlightFlag == 2)
  {
    //FreifallCode
    alt = bme.readAltitude(altGND);
    FlightFlag = 2;
    Ding = (alt - altOld);
    if (Ding > FreefallDetection)
    {
      Serial.printf("2Temperatur: %2.2f% °C  Altitude: %0.1f% m/GND  Pressure: %0.2f hPa  Flight: %1d  A-AO: %0.1f \r\n",
                    tmp, alt, pressure, FlightFlag, Ding);
      Freefall = 1;
      FlightFlag = 3;
      altDEPLOY = alt;
    }
  }
  if (FlightFlag == 3)
  {
    alt = bme.readAltitude(altGND);
    FlightFlag = 3;
    Ding = (alt - altOld);
    if (Ding > FreefallDetection)
    {
      Serial.printf("3Temperatur: %2.2f% °C  Altitude: %0.1f% m/GND  Pressure: %0.2f hPa  Flight: %1d  A-AO: %0.1f \r\n",
                    tmp, alt, pressure, FlightFlag, Ding);
      Freefall = 1;
    }
    if (alt <= NearGroundOffset)
    {
      if (i == 10) //100
      {
        Jump++;
        // Sprung Speichern?
        //Werte auf 0
        alt        = 0;
        altMAX     = 0;
        altDEPLOY  = 0;
        Freefall   = 0;
        i          = 0;
        FlightFlag = 0;
      }
      else
      {
        i++;
      }
    }
  }
  Serial.printf("Temperatur: %2.2f% °C  Altitude: %0.1f% m/GND  Pressure: %0.2f hPa  Flight: %1d  A-AO: %0.1f \r\n",
                tmp, alt, pressure, FlightFlag, (alt - altOld));

  tftSprite.fillSprite(BLACK);
  tftSprite.setCursor(0, 0, 1);
  tftSprite.setTextSize(2);
  tftSprite.printf("Flight: %1d  \r\n", FlightFlag);
  tftSprite.printf("Alti: %0.1f m/GND \r\nA: %0.1f% \r\n", alt, (alt - altOld));
  tftSprite.printf("Max: %0.1f \r\n", altMAX);
  tftSprite.printf("Dep: %0.2f \r\n", altDEPLOY);
  tftSprite.setTextSize(1);
  tftSprite.printf("P: %0.2f hPa  I=%3d \r\n", pressure, i);
  tftSprite.printf("Bat: V: %.3fv  I: %.3fma\r\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
  tftSprite.printf("Bat power %.3fmw", M5.Axp.GetBatPower());
  tftSprite.pushSprite(5, 5);
  delay(1000);
}
