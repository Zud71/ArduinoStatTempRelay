#include <main.h>
#include <data.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#include <DHT.h>

#include <GSMMod.h>
//#include <GSMSim.h>

#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>

#include <Adafruit_Sensor.h>

#include <SoftwareSerial.h>
#include "Arduino.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

#define BUTTONPIN 10
#define RELEPINA 2
#define RELEPINB 3

#define TIMERDISPLAY 60

#define GSMPINRX 5
#define GSMPINTX 8
#define GSMPINRESET 9
#define BAUD 19200
#define AM2320POWER 6
#define AM2320PIN 7
#define DHTTYPE DHT22
#define CHREESTART 5 //счетчик сброса температурного модуля

#define COUNTSEND 900    //счетчик отсылки данных
#define COUNT_GET_TEMP 30 //счетчик получения температуры
#define COUNTINCORRECT 3

GSMMod gsm(GSMPINRX, GSMPINTX, GSMPINRESET);

SSD1306AsciiAvrI2c oled;
DHT dht(AM2320PIN, DHTTYPE);

///---------------
// Data wire is plugged into port 10
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer;
///---------------

uint8_t col0 = 0; // First value column
uint8_t col1 = 0; // Last value column.
uint8_t col2 = 0;
uint8_t col3 = 0;
uint8_t ColStatusGSM = 0;
uint8_t ColStatusGPRS = 0;
uint8_t ColStatusTime = 0;
uint8_t IndRows = 0;

uint8_t ch_round = 3;
uint8_t ch_getTemp = COUNT_GET_TEMP;
uint8_t col_space = 0;
uint8_t col_space1 = 0;

//enum  TypeStatus { Temperatura = 0, Status = 1 };

const char *label[] = {"Temp:", "Gsm:", "Gprs:", "Temp:"};
const char *units[] = {"C | ", "%"};
const char *indic[] = {"-", "\\", "|", "/"};

float STemp;
float SHum;

//-------------
int count;
int adresscount = 0;
int lastsensor = 129;
float t[] = {0, 0, 0, 0, 0, 0};
//-----------

//*******************************************************************************
void setup()
{
  pinMode(BUTTONPIN, INPUT_PULLUP);

  pinMode(RELEPINA, OUTPUT);
  pinMode(RELEPINB, OUTPUT);

  digitalWrite(RELEPINA, HIGH);
  digitalWrite(RELEPINB, HIGH);

  pinMode(AM2320POWER, OUTPUT);
  digitalWrite(AM2320POWER, LOW);

  Serial.begin(9600);
  SetupAM2320();
  SetupDs18b20();
  SetupLCD();
  SetupGSM();
}

void SetupDs18b20()
{

  Serial.println("Init Ds18b20");
  sensors.begin();
  count = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" sensors");
  if (count == 0)
  {
    Serial.print("Error, no sensors found, please check");
  }
  else
  {

    Serial.print("Requiest temperatures...");
    sensors.requestTemperatures();
    Serial.println();

    for (int i = 0; i < count; i++)
    {
      sensors.getAddress(Thermometer, i);
      t[i] = sensors.getTempCByIndex(i);
      Serial.print(t[i]);
      Serial.println(" oC");
    }
  }
}

void SetupAM2320()
{

  Serial.println("AM2320 Start");
  ResetAM2320();

  dht.begin();
  delay(3000);
  GetTemperature();
}

/*
void printAddress(DeviceAddress deviceAddress)
{
	Serial.print("    {");
	for (uint8_t i = 0; i < 8; i++)
	{
		Serial.print("0x");
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
		if (i != 7) Serial.print(", ");
	}
	Serial.print("}");
	adresscount++;
	if (adresscount == count) {
		Serial.println(); Serial.println("};");
		Serial.println(); Serial.print("DONE!!!"); while (1);
	}
	else {
		Serial.println(',');
	}
}*/

void ResetAM2320()
{
  Serial.println("Restart power AM2320");
  digitalWrite(AM2320POWER, LOW);
  delay(1000);
  digitalWrite(AM2320POWER, HIGH);
}

void SetupGSM()
{

  IndicateGSMStatusS("GSMSim init");
  IndicateGPRSStatusS("--------");
  Serial.println("");
  delay(1000);
  gsm.start(BAUD);

  IndicateGSMStatusS("Reset GSMSim");
  gsm.reset();
  delay(10000);

  IndicateGSMStatusS("Done Reset");
  // GSMConnect();
}

void SetupLCD()
{
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);

  // Increase space between letters.
  oled.setLetterSpacing(1);
  oled.clear();

  oled.println(label[0]);
  col0 = oled.strWidth(label[0]);
  oled.println(label[1]);
  ColStatusGSM = oled.strWidth(label[1]) + 4;
  oled.println(label[2]);
  ColStatusGPRS = oled.strWidth(label[2]) + 3;
  oled.println(label[3]);
  ColStatusTime = oled.strWidth(label[3]) + 3;

  col0 += 3;
  col_space = oled.strWidth(units[0]) + 4;
  col_space1 = oled.strWidth(units[1]) + 4;

  col1 = col0 + oled.strWidth("99.9");

  col2 = col1 + oled.strWidth("99.9") + col_space;
  col3 = col2 + oled.strWidth(indic[0]) + col_space1;

  oled.setCursor(col1 + 1, 0);
  oled.print(units[0]);

  oled.setCursor(col2 + 1, 0);
  oled.print(units[1]);
}
//*******************************************************
uint8_t resetChPower = CHREESTART;
uint8_t countIncorectT = COUNTINCORRECT;
uint8_t countIncorectH = COUNTINCORRECT;

void GetTemperature()
{
  Serial.println("Get Temperature");

  float STemp1;
  float SHum1;
  static bool firstStart;

  STemp1 = dht.readTemperature();
  SHum1 = dht.readHumidity();

    Serial.println(STemp);
    Serial.println(SHum);

    if (isnan(STemp1))
    {
      Serial.println(F("Error reading temperature!"));
      
      resetChPower--;
    }
    else
    {
      if (abs(STemp - STemp1) > 4 && firstStart && countIncorectT)
      {
        Serial.print(F("incorrect reading temperature! - "));
        Serial.println(STemp1);
        countIncorectT--;

            }else{

              countIncorectT = COUNTINCORRECT;
              resetChPower = CHREESTART;
              STemp = STemp1;
              Serial.print(F("Temperature: "));
              Serial.print(STemp);
              Serial.println(F("�C"));
      }
    }

  if (isnan(SHum1))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    if (abs(SHum - SHum1) > 40 && firstStart && countIncorectH)
    {
      Serial.print(F("incorrect reading humidity! - "));
      Serial.println(SHum1);
      countIncorectH--;
    }
    else
    {
      firstStart = true;
      countIncorectH = COUNTINCORRECT;
      SHum = SHum1;
      Serial.print(F("Humidity: "));
      Serial.print(SHum);
      Serial.println(F("%"));
    }
  }

  if (!resetChPower)
  {
    resetChPower = CHREESTART;
    ResetAM2320();
    //	GPRSHttpSendData("����� AM2320", STATUS_S);
  }

  Serial.println(resetChPower);
}

//-----------------------------------
void GPRSConnect()
{
  /*bool reg = false;

  do
  {
    reg = gsm.isRegistered();
    IndicateGSMStatus(reg);
    delay(300);
  } while (!reg);

  delay(300);*/

  do
  {

    IndicateGPRSStatusS("Connecting");
    gsm.gprsConnectBearer(Apn);
    delay(5000);
  } while (!gsm.gprsIsConnected());

  // Serial.println(gsm.gprsGetIP());

  IndicateGPRSStatusS("Conected");
  delay(3000);
}

void GPRSCloseConnect()
{
  bool cls = false;
  bool clsCn = false;

  IndicateGPRSStatusS("Close conect");

  do
  {
    clsCn = gsm.gprsIsConnected();
    if (clsCn)
    {
      cls = gsm.gprsCloseConn();
    }
    else
    {
      break;
    }
    delay(250);

  } while (!cls);

  IndicateGPRSStatusS("Conect closed");
  delay(3000);
}

String GPRSHttpSendData(const String &str)
{
  delay(1000);
 
  IndicateGPRSStatusS("Send data");

  String data = gsm.gprsHTTPSGet(QUrl, str, true);

  Serial.println("Return: " + data);
 // IndicateGPRSStatusS("R: " + data);
  delay(1000);

return data;

}


void IndicateTemp()
{

  oled.clear(col0, col1, 0, 0);
  oled.print(STemp, 1);
  oled.clear(col1 + col_space, col2, 0, 0);
  oled.print(SHum, 1);

  oled.clear(col2 + col_space1, col3, 0, 0);
  oled.print(indic[ch_round]);

  if (ch_round)
  {
    ch_round--;
  }
  else
  {
    ch_round = 3;
  }

  if (ch_getTemp)
  {
    ch_getTemp--;
  }
  else
  {
    ch_getTemp = COUNT_GET_TEMP;
    GetTemperature();
    IndicateTemp2();
  }
}
//---------------------------------------
/*void IndicateTime()
{
	//oled.clear(ColStatusTime, oled.displayWidth(), 3, 3);
	Serial.println(gsm.timeGetRaw());
	//oled.print(gsm.timeGetRaw());
}*/

void IndicateTemp2()
{
  sensors.requestTemperatures();

  for (int i = 0; i < count; i++)
  {
    sensors.getAddress(Thermometer, i);
    t[i] = sensors.getTempCByIndex(i);
    Serial.print(t[i]);
    Serial.println(" oC");
  }

  oled.clear(ColStatusTime, oled.displayWidth(), 3, 3);
  oled.print(t[0]);
}

//--------------------------------------
void IndicateGSMStatusS(const char* str)
{
  Serial.println(str);

  oled.clear(ColStatusGSM, oled.displayWidth(), 1, 1);

  oled.print(str);
}

void IndicateGSMStatus(bool IsRegistered)
{
  oled.clear(ColStatusGSM, oled.displayWidth(), 1, 1);

  if (IsRegistered)
  {

    uint8_t sg = gsm.signalQuality();
    Serial.print("Registered Q:\t\t");
    Serial.println(sg);

    oled.print("Registered Q:");
    oled.print(sg);
  }
  else
  {
    Serial.println("Not Registered");
    oled.print("Not Registered");
  }
}

//--------------------------------------

void IndicateGPRSStatusS(const char *str)
{
  Serial.println(str);

  oled.clear(ColStatusGPRS, oled.displayWidth(), 2, 2);

  oled.print(str);
}

void IndicateGPRSStatus(bool IsConnected)
{
  oled.clear(ColStatusGPRS, oled.displayWidth(), 2, 2);
  if (IsConnected)
  {
    Serial.println("Status Conected");
    oled.print("Status Conected");
  }
  else
  {
    Serial.println("Not Conected");
    oled.print("Not Conected");
  }
}

void RunCommad(String &cmd)
{
  if (cmd.indexOf("ERROR")==-1)
  {
    if(cmd.substring(9,cmd.indexOf("|"))=="200")
    {
      Serial.println("Command status 200");
      String run = cmd.substring(cmd.indexOf("|DATA:")+6);

      Serial.println("Command ->" + run);

if(run=="r1on")
{
  digitalWrite(RELEPINA, LOW);
}

if (run == "r2on")
{
  digitalWrite(RELEPINB, LOW);
}

if (run == "r1off")
{

  digitalWrite(RELEPINA, HIGH);
}

if (run == "r2off")
{

  digitalWrite(RELEPINB, HIGH);
}

if (run == "r1togle")
{
  digitalWrite(RELEPINA, HIGH);
  delay(500);
  digitalWrite(RELEPINA, LOW);
  delay(500);
  digitalWrite(RELEPINA, HIGH);
}

if (run == "r2togle")
{
  digitalWrite(RELEPINB, HIGH);
  delay(500);
  digitalWrite(RELEPINB, LOW);
  delay(500);
  digitalWrite(RELEPINB, HIGH);
}

if (run == "roff")
{
  digitalWrite(RELEPINA, HIGH);
  digitalWrite(RELEPINB, HIGH);

}
    }

  }else
  {
    Serial.println("Error read command");
  }
}

  //-------------------------------------------
  uint16_t CountToSend = COUNTSEND;
  uint8_t CounterDisplayShow = TIMERDISPLAY;
  bool DisplayOn = true;
  uint8_t cind=0;
  bool isReg=false;
  
  void loop()
  {

    IndicateTemp();

    IndicateGSMStatus(isReg);

    while (!isReg)
    {
      isReg = gsm.isRegistered();
      delay(3000);
    }
      

    if (isReg)
    {
      if (!CountToSend--)
      {
        CountToSend = COUNTSEND;

        bool isCon = gsm.gprsIsConnected();
        IndicateGPRSStatus(isCon);

        if (!isCon)
        {
          delay(1000);
          GPRSConnect();

          String data = "stg?t1=" + String(STemp) + "&t2=" + String(t[0]) + "&hum=" + String(SHum);

          if (GPRSHttpSendData(data).indexOf("ERROR") == -1)
          {
            String command = GPRSHttpSendData("gcmd?status=" + (String)cind);
            RunCommad(command);
            cind++;
            if (cind==6)
              cind=0;
          }

          GPRSCloseConnect();
        }
      }

      char buffer[4];
      char *p;
      p = itoa(CountToSend, buffer, 10);

      IndicateGPRSStatusS(p);
    }

    uint8_t btn = digitalRead(BUTTONPIN);

    if (btn == 0)
    {
      
      oled.ssd1306WriteCmd(SSD1306_DISPLAYON);
      CounterDisplayShow = TIMERDISPLAY;
      DisplayOn = true;
    }
    else
    {

      if (DisplayOn)
      {

        if (!CounterDisplayShow--)
        {

          oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
          DisplayOn = false;
        }
      }

    }

  delay(1000);
}
