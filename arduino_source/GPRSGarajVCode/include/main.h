#include "Arduino.h"

#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

void SetupGSM();
void SetupDs18b20();
void SetupAM2320();
void ResetAM2320();
void SetupGSM();
void SetupLCD();
void GetTemperature();
void GPRSConnect();
void GPRSCloseConnect();
String GPRSHttpSendData(const String &str);
void IndicateTemp();
void IndicateTemp2();

void IndicateGSMStatusS(const char *str);
void IndicateGSMStatus(bool IsRegistered);

void IndicateGPRSStatusS(const char *str);
void IndicateGPRSStatus(bool IsConnected);

void RunCommad(String &cmd);