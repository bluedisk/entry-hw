#include "protocol.h"

// 버퍼
char buffer[52];
unsigned char prevc = 0;
byte dataLen;
bool isStart = false;
unsigned char packetOffset = 0;

uint8_t command_index = 0;

ActionGetCallback actionGetCallback = NULL;
ActionSetCallback actionSetCallback = NULL;


void setActionCallback(ActionGetCallback getCallback, ActionSetCallback setCallback) {
    actionGetCallback = getCallback;
    actionSetCallback = setCallback;
}

void processPacket() {
    while (Serial.available() > 0) {
        char serialRead = Serial.read();
        stackData(serialRead & 0xff);
    }
}

void stackData(unsigned char c) {
    if (c == 0x55 && !isStart) {
        if (prevc == 0xff) {
            packetOffset = 1;
            isStart = true;
        }
    } else {
        prevc = c;
        if (isStart) {
            if (packetOffset == 2) {
                dataLen = c;
            } else if (packetOffset > 2) {
                dataLen--;
            }

            writeBuffer(packetOffset, c);
        }
    }

    packetOffset++;

    if (packetOffset > 51) {
        packetOffset = 0;
        isStart = false;
    }

    if (isStart && dataLen == 0 && packetOffset > 3) {
        isStart = false;
        dispatchPacket();
        packetOffset = 0;
    }
}

int readIndex = 0;
void seekBuffer(int seekOffset) {
    readIndex = seekOffset;
}

unsigned char readBuffer() {
    return buffer[readIndex++];
}

void dispatchPacket() {
    isStart = false;

    // don't change the order
    seekBuffer(3);
    int idx = readBuffer();
    int action = readBuffer();
    int device = readBuffer();
    int port = readBuffer();

    command_index = (uint8_t) idx; // ???

    switch (action) {
        case GET: 
            if (actionGetCallback)
                actionGetCallback(idx, port, device);
        
            break;
        case SET: {
            if (actionSetCallback)
                if (actionSetCallback(idx, port, device))
                    callOK();
        }
            break;
        case RESET: {
            callOK();
        }
            break;
    }
}


void writeBuffer(int index, unsigned char c) {
    buffer[index] = c;
}

void writeHead() {
    writeSerial(0xff);
    writeSerial(0x55);
}

void writeEnd() {
    Serial.println();
}

void writeSerial(unsigned char c) {
    Serial.write(c);
}

void sendString(String s) {
    int l = s.length();
    writeSerial(4);
    writeSerial(l);
    for (int i = 0; i < l; i++) {
        writeSerial(s.charAt(i));
    }
}

void sendFloat(float value) {
    writeSerial(2);
    val.floatVal = value;
    writeSerial(val.byteVal[0]);
    writeSerial(val.byteVal[1]);
    writeSerial(val.byteVal[2]);
    writeSerial(val.byteVal[3]);
}

void sendShort(double value) {
    writeSerial(3);
    valShort.shortVal = value;
    writeSerial(valShort.byteVal[0]);
    writeSerial(valShort.byteVal[1]);
}

short readShort() {
    valShort.byteVal[0] = readBuffer();
    valShort.byteVal[1] = readBuffer();
    return valShort.shortVal;
}

float readFloat() {
    val.byteVal[0] = readBuffer();
    val.byteVal[1] = readBuffer();
    val.byteVal[2] = readBuffer();
    val.byteVal[3] = readBuffer();
    return val.floatVal;
}

long readLong() {
    val.byteVal[0] = readBuffer();
    val.byteVal[1] = readBuffer();
    val.byteVal[2] = readBuffer();
    val.byteVal[3] = readBuffer();
    return val.longVal;
}

void callOK() {
    writeSerial(0xff);
    writeSerial(0x55);
    writeEnd();
}

void callDebug(char c) {
    writeSerial(0xff);
    writeSerial(0x55);
    writeSerial(c);
    writeEnd();
}
