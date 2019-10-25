#include "protocol.h"

// 버퍼
char buffer[52];
unsigned char prevc = 0;
byte dataLen = 0;
bool isStart = false;
unsigned char packetOffset = 0;

uint8_t command_index = 0;

ActionGetCallback actionGetCallback = NULL;
ActionSetCallback actionSetCallback = NULL;
ActionResetCallback actionResetCallback = NULL;

void setActionCallback(ActionGetCallback getCallback, ActionSetCallback setCallback, ActionResetCallback resetCallback) {
    actionGetCallback = getCallback;
    actionSetCallback = setCallback;
    actionResetCallback = resetCallback;
}

void processPacket() {
    while (Serial.available() > 0) {
        char serialRead = Serial.read();
        stackData(serialRead & 0xff);
    }
}

void stackData(unsigned char c) {
    if (c == 0x2D && !isStart) {
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
            if (actionResetCallback) {
                actionResetCallback(idx, port, device);
                callOK();
            }
        }
            break;
    }
}


void writeBuffer(int index, unsigned char c) {
    buffer[index] = c;
}

void writeHead() {
    writeSerial(0xff);
    writeSerial(0x2D);
}

void writeEnd() {
    Serial.println();
}

void writeSerial(unsigned char c) {
    Serial.write(c);
}

void sendText(String s) {
    int l = s.length();
    writeSerial(TYPE_TEXT);
    writeSerial(l);
    for (int i = 0; i < l; i++) {
        writeSerial(s.charAt(i));
    }
}

void sendFloat(float value) {
    writeSerial(TYPE_FLOAT);
    val.floatVal = value;
    writeSerial(val.byteVal[0]);
    writeSerial(val.byteVal[1]);
    writeSerial(val.byteVal[2]);
    writeSerial(val.byteVal[3]);
}

void sendTwinFloat(float value1, float value2){
    writeSerial(TYPE_TWIN);
    val.floatVal = value1;
    writeSerial(val.byteVal[0]);
    writeSerial(val.byteVal[1]);
    writeSerial(val.byteVal[2]);
    writeSerial(val.byteVal[3]);
    val.floatVal = value2;
    writeSerial(val.byteVal[0]);
    writeSerial(val.byteVal[1]);
    writeSerial(val.byteVal[2]);
    writeSerial(val.byteVal[3]);
}

void sendShort(short value) {
    writeSerial(TYPE_SHORT);
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

char* readString() {
    int len =  readBuffer();
    char* buf = (char* )malloc(len+1);

    memset(buf, 0, len+1);
    memcpy(buf, &buffer[readIndex], len);
    readIndex += len;

    return buf;
}

void callOK() {
    writeSerial(0xff);
    writeSerial(0x2D);
    writeEnd();
}

void callDebug(char c) {
    writeSerial(0xff);
    writeSerial(0x2D);
    writeSerial(c);
    writeEnd();
}
