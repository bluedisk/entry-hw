// 서보 라이브러리
#include <Servo.h>
#include "protocol.h"

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
int ports[NORI_PORTS_CNT] = {ALIVE, ALIVE, ALIVE, ALIVE};


// 전역변수 선언 종료

void actionGet(int idx, int port, int device);

bool actionSet(int idx, int port, int device);

void setup() {
    Serial.begin(115200);
    initPorts();
    setActionCallback(actionGet, actionSet);
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
        case IRRANGE:
            break;
        case TOUCH:
            break;
        case LCD:
            break;
        case SEGMENT:
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
        case IRRANGE:
            break;
        case TOUCH:
            break;
        case LCD:
            break;
        case SEGMENT:
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
            digitalWrite(DPINS[port], HIGH);

            delay(readShort());

            digitalWrite(DPINS[port], LOW);
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
        case SEGMENT:
            break;
    }
}

void sendModuleValue(int port) {
    switch (ports[port]) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
            // do nothing
            break;
        case VOLUME:
            writeHead();
            sendFloat(analogRead(APINS[port]));
            writeEnd();
            break;
        case SOUND:
            writeHead();
            sendFloat(analogRead(DPINS[port]));
            writeEnd();
            break;
        case BUTTON:
            writeHead();
            sendFloat(analogRead(DPINS[port]));
            writeEnd();
            break;
        case AMBIENT:
            writeHead();
            sendFloat(analogRead(APINS[port]));
            writeEnd();
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
            break;
        case TOUCH:
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
