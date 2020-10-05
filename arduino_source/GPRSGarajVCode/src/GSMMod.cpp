#include "Arduino.h"
#include "GSMMod.h"
#include <SoftwareSerial.h>

GSMMod::GSMMod(uint8_t rx, uint8_t tx, uint8_t rst) : SoftwareSerial(rx, tx)
{
    RX_PIN = rx;
    TX_PIN = tx;
    RESET_PIN = rst;

}

void GSMMod::start(uint32_t baud)
{
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, HIGH);

    _baud = baud;

    this->begin(_baud);

    _buffer.reserve(BUFFER_RESERVE_MEMORY);
}

// Reset GMS Module
void GSMMod::reset()
{
    digitalWrite(RESET_PIN, LOW);
    delay(1000);
    digitalWrite(RESET_PIN, HIGH);
    delay(1000);

    this->print(F("AT\r"));
    while (_readSerial().indexOf("OK") == -1)
    {
        this->print(F("AT\r"));
    }

}

// IS Module connected to the operator?
bool GSMMod::isRegistered()
{
    this->print(F("AT+CREG?\r"));
    _buffer = _readSerial(100000);

   // Serial.println("isRegistered -" + _buffer);

    if ((_buffer.indexOf("+CREG: 0,1")) != -1 || (_buffer.indexOf("+CREG: 0,5")) != -1 || (_buffer.indexOf("+CREG: 1,1")) != -1 || (_buffer.indexOf("+CREG: 1,5")) != -1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool GSMMod::gprsConnectBearer(const char *apn)
{

    this->print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));
    _buffer = _readSerial();

    if (_buffer.indexOf("OK") != -1)
    {

        delay(100);
        this->print(F("AT+SAPBR=3,1,\"APN\",\""));
        this->print(apn);
        this->print(F("\"\r"));
        _buffer = _readSerial();

        if (_buffer.indexOf("OK") != -1)
        {
            delay(100);

            this->print(F("AT+SAPBR=3,1,\"USER\",\"\"\r"));
            _buffer = _readSerial();

            if (_buffer.indexOf("OK") != -1)
            {

                delay(100);

                this->print(F("AT+SAPBR=3,1,\"PWD\",\"\"\r"));
                _buffer = _readSerial();

                if (_buffer.indexOf("OK") != -1)
                {

                    delay(100);
                    this->print("AT+SAPBR=1,1\r");
                    _buffer = _readSerial();
                    delay(50);
                    _buffer += _readSerial();
                    if (_buffer.indexOf("OK") != -1)
                    {

                        this->print("AT+SAPBR=2,1\r");
                        _buffer = _readSerial();

                        if (_buffer.indexOf("\"0.0.0.0\"") != -1 || _buffer.indexOf("ERR") != -1)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

// Check is GPRS connected?
bool GSMMod::gprsIsConnected()
{
    this->print(F("AT+SAPBR=2,1\r"));
    _buffer = _readSerial();
    delay(50);
    _buffer += _readSerial();

    if (_buffer.indexOf("ERR") != -1 || _buffer.indexOf("\"0.0.0.0\"") != -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool GSMMod::gprsCloseConn()
{
    this->print(F("AT+SAPBR=0,1\r"));
    _buffer = _readSerial();
    delay(50);
    _buffer = _readSerial();

    if (_buffer.indexOf("OK") != -1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
String GSMMod::gprsHTTPSGet(const char *url, const String &data, bool read)
{
    if (gprsIsConnected())
    {
        // Terminate http connection, if it opened before!
        this->print(F("AT+HTTPTERM\r"));
        _buffer = _readSerial();

        this->print(F("AT+HTTPINIT\r\n"));
        _buffer = _readSerial();

        //return _buffer;
        if (_buffer.indexOf("OK") != -1)
        {
            this->print(F("AT+HTTPPARA=\"CID\",1\r"));
            _buffer = _readSerial();

            if (_buffer.indexOf("OK") != -1)
            {
                
                this->print(F("AT+HTTPPARA=\"URL\",\""));
                this->print(url);
                this->print(data);
                this->print(F("\"\r"));
                _buffer = _readSerial();

                Serial.println("URL ->" + _buffer);

                if (_buffer.indexOf("OK") != -1)
                {
                  /*  this->print(F("AT+HTTPSSL=1\r"));
                    _buffer = _readSerial();

                    if (_buffer.indexOf("OK") == -1)
                    {
                        return "ERROR:HTTPS_ENABLE_ERROR";
                    }*/

                    this->print(F("AT+HTTPACTION=0\r"));
                    _buffer = _readSerial();

                    if (_buffer.indexOf("OK") != -1)
                    {
                        delay(100);
                        _buffer = _readSerial(1000000);

                        Serial.println("read ->" + _buffer);

                        if (_buffer.indexOf("+HTTPACTION: 0,") != -1)
                        {
                            String httpCod = _buffer.substring(_buffer.indexOf(",") + 1, _buffer.lastIndexOf(","));
                            String len = _buffer.substring(_buffer.lastIndexOf(",") + 1);
                            httpCod.trim();
                            len.trim();

                            this->print(F("AT+HTTPREAD\r"));
                            _buffer = _readSerial(10000);

                            Serial.println("data ->" + _buffer);

                            String okuma = "";

                            if (_buffer.indexOf("+HTTPREAD:") != -1)
                            {

                                String kriter = "+HTTPREAD: " + len;
                                okuma = _buffer.substring(_buffer.indexOf(kriter) + kriter.length(), _buffer.lastIndexOf("OK"));
                                okuma.trim();

                            }
                            else
                            {
                                return "ERROR:HTTP_READ_ERROR";
                            }

                            String sonuc = "HTTPCODE:";
                            sonuc += httpCod;
                            sonuc += "|LENGTH:";
                            sonuc += len;
                            sonuc += "|DATA:";
                            sonuc += okuma;

                            this->print(F("AT+HTTPTERM\r"));
                            _buffer = _readSerial();

                            sonuc.trim();

                            return sonuc;
                        }
                        else
                        {
                            return "ERROR:HTTP_ACTION_READ_ERROR";
                        }
                    }
                    else
                    {
                        return "ERROR:HTTP_ACTION_ERROR";
                    }
                }
                else
                {
                    return "ERROR:HTTP_PARAMETER_ERROR 1";
                }
            }
            else
            {
                return "ERROR:HTTP_PARAMETER_ERROR 2";
            }
        }
        else
        {
            return "ERROR:HTTP_INIT_ERROR";
        }
    }
    else
    {
        return "ERROR:GPRS_NOT_CONNECTED";
    }
}

// SIGNAL QUALTY - 0-31 | 0-> poor | 31 - Full | 99 -> Unknown
uint8_t GSMMod::signalQuality()
{

    this->print(F("AT+CSQ\r"));
    _buffer = _readSerial(100000);

  //  Serial.println("signalQuality - " + _buffer);

    if ((_buffer.indexOf("+CSQ:")) != -1)
    {
        return _buffer.substring(_buffer.indexOf("+CSQ: ") + 6, _buffer.indexOf(",")).toInt();
    }
    else
    {
        return 99;
    }
}

//////////////////////////////////////
//			PRIVATE METHODS			//
//////////////////////////////////////

// READ FROM SERIAL
String GSMMod::_readSerial()
{

    uint64_t timeOld = millis();

    while (!this->available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL))
    {
        delay(13);
    }

    String str = "";

    while (this->available())
    {
        if (this->available())
        {
            str += (char)this->read();
        }
    }

    return str;
}

String GSMMod::_readSerial(uint32_t timeout)
{

    uint64_t timeOld = millis();

    while (!this->available() && !(millis() > timeOld + timeout))
    {
        delay(13);
    }

    String str = "";

    while (this->available())
    {
        if (this->available())
        {
            str += (char)this->read();
        }
    }

    return str;
}