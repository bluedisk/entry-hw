#include <Arduino.h>

// Actions
#define GET     1
#define SET     2
#define RESET   3
#define CFG     4

// Types
#define TYPE_INT8    1
#define TYPE_FLOAT   2
#define TYPE_SHORT   3
#define TYPE_TEXT    4
        
// val Union
union {
    byte byteVal[4];
    float floatVal;
    long longVal;
} val;

// valShort Union
union {
    byte byteVal[2];
    short shortVal;
} valShort;

typedef void (*ActionGetCallback)(int idx, int port, int device);
typedef bool (*ActionSetCallback)(int idx, int port, int device);
typedef void (*ActionResetCallback)(int idx, int port, int device);


void setActionCallback(ActionGetCallback getCallback, ActionSetCallback setCallback, ActionResetCallback resetCallback);

void processPacket();
void stackData(unsigned char c);

unsigned char readBuffer();

void dispatchPacket();

void writeBuffer(int index, unsigned char c);

void writeHead();
void writeEnd();
void writeSerial(unsigned char c);

void sendText(String s);
void sendFloat(float value);
void sendShort(double value);

short readShort();
float readFloat();
long readLong();
String readString();

void callOK();
void callDebug(char c);
