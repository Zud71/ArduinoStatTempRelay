#ifndef GSMMod_h
#define GSMMod_h
#include <SoftwareSerial.h>
#include "Arduino.h"

#define DEFAULT_BAUD_RATE 9600
#define BUFFER_RESERVE_MEMORY 255
#define TIME_OUT_READ_SERIAL 5000


class GSMMod : public SoftwareSerial
{
private:
    uint32_t _baud;
    uint8_t _timeout;
    String _buffer;
    String _readSerial();
    String _readSerial(uint32_t timeout);

public:
    uint8_t RX_PIN;
    uint8_t TX_PIN;
    uint8_t RESET_PIN;
    uint32_t BAUDRATE;

    GSMMod(uint8_t rx, uint8_t tx, uint8_t rst);

    void start(uint32_t baud);

    void reset();

    bool isRegistered();

    bool gprsConnectBearer(const char* apn);

    bool gprsIsConnected();

    bool gprsCloseConn();

    String gprsHTTPSGet(const char* url, const String &data, bool read);

    uint8_t signalQuality();
};

#endif