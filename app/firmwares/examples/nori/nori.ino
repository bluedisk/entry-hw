// 서보 라이브러리
#include <Servo.h>
#include "protocol.h"
#include "TM1637Display.h"

// noricoding 핀 설정
#define PORT1D 5
#define PORT1A A0
#define PORT2D 6
#define PORT2A A1
#define PORT3D 9
#define PORT3A A2
#define PORT4D 10
#define PORT4A A3

#define NORI_PORTS_CNT  4

const int APINS[NORI_PORTS_CNT] = {PORT1A, PORT2A, PORT3A, PORT4A};
const int DPINS[NORI_PORTS_CNT] = {PORT1D, PORT2D, PORT3D, PORT4D};

#define CHECK_PHRASE "Hello Nori!"

// 동작 상수
#define ALIVE 0

#define BUZZER  1
#define VOLUME  2
#define SOUND   3
#define BUTTON  4
#define AMBIENT 5
#define SERVO   6

#define TONE        7
#define MOTOR       8
#define NEOPIXEL    9
#define TEMPER      10
#define ULTRASONIC  11
#define IRRANGE     12
#define TOUCH       13
#define LCD         14
#define SEGMENT     15

// 전역변수 선언 시작
Servo servos[NORI_PORTS_CNT];
TM1637Display *segment[NORI_PORTS_CNT] = {NULL,NULL,NULL,NULL};

int ports[NORI_PORTS_CNT] = {ALIVE, ALIVE, ALIVE, ALIVE};


// 전역변수 선언 종료

void actionGet(int idx, int port, int device);
bool actionSet(int idx, int port, int device);
void actionReset(int idx, int port, int device);

void setup() {
    Serial.begin(115200);
    initPorts();
    setActionCallback(actionGet, actionSet, actionReset);
    delay(200);
}

void initPorts() {
    for (int i = 0; i < NORI_PORTS_CNT; i++) {
        pinMode(APINS[i], OUTPUT);
        digitalWrite(APINS[i], LOW);

        pinMode(DPINS[i], OUTPUT);
        digitalWrite(DPINS[i], LOW);
    }
}

void sendModuleValues() {
    for (int i = 0; i < NORI_PORTS_CNT; i++) {
        sendModuleValue(i);
        callOK();
    }
}

void loop() {
    processPacket();
    delay(15);
    sendModuleValues();
    delay(10);
}

void actionGet(int idx, int port, int device) {
    changeModule(port, device);
}

bool actionSet(int idx, int port, int device) {
    changeModule(port, device);
    runModule(port, device);

    return true;
}

void actionReset(int idx, int port, int device) {
    changeModule(port, ALIVE);
}

void initModule(int port, int device) {
    switch (device) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
            pinMode(DPINS[port], OUTPUT);
            digitalWrite(DPINS[port], LOW);
            break;

        case VOLUME:
        case SOUND:
        case BUTTON:
        case AMBIENT:
        case IRRANGE:
        case TOUCH:
            pinMode(DPINS[port], INPUT);
            pinMode(APINS[port], INPUT);
            break;

        case SERVO:
            servos[port].attach(DPINS[port]);
            break;

        case TONE:
            break;
        case MOTOR:
            break;
        case NEOPIXEL:
            break;
        case TEMPER:
            break;
        case ULTRASONIC:
            break;
        case LCD:
            break;
        case SEGMENT:
            if ( segment[port] != NULL ) {
              delete segment[port];
            }

            segment[port] = new TM1637Display(DPINS[port], APINS[port]);
            segment[port]->setBrightness(0x0F);
            segment[port]->clear();
            break;
    }
}

void delModule(int port) {
    switch (ports[port]) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
        case VOLUME:
        case SOUND:
        case BUTTON:
        case AMBIENT:
        case IRRANGE:
        case TOUCH:
        case TONE:
        case NEOPIXEL:
        case TEMPER:
        case ULTRASONIC:
            pinMode(DPINS[port], OUTPUT);
            digitalWrite(DPINS[port], LOW);

            pinMode(APINS[port], OUTPUT);
            digitalWrite(APINS[port], LOW);
            break;
            
        case SERVO:
            digitalWrite(DPINS[port], LOW);
            digitalWrite(APINS[port], LOW);
            servos[port].detach();
            break;

        case MOTOR:
            break;
        case LCD:
            break;
        case SEGMENT:
            if ( segment[port] == NULL ) break;
            
            segment[port]->clear();
            //segment[port]->showNumberDec(5555);
            delete segment[port];
            segment[port] = NULL;
            break;
    }
}

void runModule(int port, int device) {
    switch (device) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
            pinMode(DPINS[port], OUTPUT);
            digitalWrite(DPINS[port], readShort());
            break;

        case VOLUME:
            // Do Nothing
            break;
        case SOUND:
            // Do Nothing
            break;
        case BUTTON:
            // Do Nothing
            break;
        case AMBIENT:
            // Do Nothing
            break;
        case SERVO: {
                int v = readShort();
                if (v >= 0 && v <= 180) {
                    servos[port].write(v);
                }
            }
            break;

        case TONE:
            break;
        case MOTOR:
            break;
        case NEOPIXEL:
            break;
        case TEMPER:
            // Do Nothing
            break;
        case ULTRASONIC:
            // Do Nothing
            break;
        case IRRANGE:
            // Do Nothing
            break;
        case TOUCH:
            // Do Nothing
            break;
        case LCD:
            break;
        case SEGMENT: {
            if ( segment[port] == NULL ) break;
            int num = readShort();
            int colon = readShort() * SEG_G;
            segment[port]->showNumberDecEx(num, colon, false);
        }
            break;
    }
}

void sendDigitalStatus(int port) {
  writeHead();
  sendShort(digitalRead(DPINS[port]));
  writeSerial(port);
  writeSerial(ports[port]);
  writeEnd();
}

void sendAnalogStatus(int port) {
  writeHead();
  sendFloat(analogRead(APINS[port]));
  writeSerial(port);
  writeSerial(ports[port]);
  writeEnd();
}

void sendModuleValue(int port) {
    switch (ports[port]) {
        case ALIVE:
            writeHead();
            sendText(CHECK_PHRASE);
            writeSerial(port);
            writeSerial(ports[port]);
            writeEnd();
            break;

        case BUZZER:
            // do nothing
            break;
        case VOLUME:
            sendAnalogStatus(port);
            break;
        case SOUND:
            sendAnalogStatus(port);
            break;
        case BUTTON:
            sendDigitalStatus(port);
            break;
        case AMBIENT:
            sendAnalogStatus(port);
            break;
        case SERVO:
            // do nothing
            break;

        case TONE:
            // do nothing
            break;
        case MOTOR:
            // do nothing
            break;
        case NEOPIXEL:
            // do nothing
            break;
        case TEMPER:
            break;
        case ULTRASONIC:
            break;
        case IRRANGE:
            sendAnalogStatus(port);
            break;
        case TOUCH:
            sendDigitalStatus(port);
            break;
        case LCD:
            // do nothing
            break;
        case SEGMENT:
            // do nothing
            break;
    }

}

void changeModule(int port, int device) {
    if (ports[port] == device) return;

    delModule(port);
    initModule(port, device);

    ports[port] = device;
}
