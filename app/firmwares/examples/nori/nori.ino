// 서보 라이브러리
#include <Servo.h>
#include "protocol.h"
#include "TM1637Display.h"
#include "LCD1602.h"
#include "SimpleDHT.h"
#include "Adafruit_NeoPixel.h"

// noricoding 핀 설정
#define PORT1D 5
#define PORT1A A0
#define PORT2D 6
#define PORT2A A1
#define PORT3D 9
#define PORT3A A2
#define PORT4D 10
#define PORT4A A3

#define NORI_PORT_CNT  4

const int ANALOG_PINS[NORI_PORT_CNT] = {PORT1A, PORT2A, PORT3A, PORT4A};
const int DIGITAL_PINS[NORI_PORT_CNT] = {PORT1D, PORT2D, PORT3D, PORT4D};

//
#define MINIMUM_LOOP_CYCLE  25  // millis
#define CHECK_PHRASE "HiNori!!"

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
#define TEXTLCD     14
#define SEGMENT     15

// 전역변수 선언 시작
typedef struct tagPort {
    // device classes pointers
    Servo *devServo = NULL;
    TM1637Display *devSegment = NULL;
    LCD1602 *devLcd = NULL;
    SimpleDHT11* devDHT = NULL; // Temp. Humid. sensor
    Adafruit_NeoPixel *devPixels = NULL;

    // device status
    int ultrasonic = 0;
    int lastSegment = 0;

    float lastTemperature = 0;
    float lastHumidity = 0;

    // port mode
    int status = ALIVE;
    int index = 0;

    // port settting(RO)
    int analog_pin = 0;
    int digital_pin = 0;
} Port;

Port ports[NORI_PORT_CNT];

// 전역변수 선언 종료

void actionGet(int idx, int port_idx, int device);

bool actionSet(int idx, int port_idx, int device);

void actionReset(int idx, int port_idx, int device);

void setup() {
    Serial.begin(115200);
    initPorts();
    setActionCallback(actionGet, actionSet, actionReset);
    delay(200);
}

void resetPort(Port &port, int analog = OUTPUT, int digital = OUTPUT) {
    pinMode(port.analog_pin, analog);
    if (analog == OUTPUT) digitalWrite(port.analog_pin, LOW);

    pinMode(port.digital_pin, OUTPUT);
    if (digital == OUTPUT) digitalWrite(port.digital_pin, LOW);
}

void initPorts() {
    for (int i = 0; i < NORI_PORT_CNT; i++) {
        ports[i].index = i;

        ports[i].analog_pin = ANALOG_PINS[i];
        ports[i].digital_pin = DIGITAL_PINS[i];

        resetPort(ports[i], OUTPUT, OUTPUT);
    }
}

void sendModuleValues() {
    for (int i = 0; i < NORI_PORT_CNT; i++) {
        sendModuleValue(ports[i]);
        callOK();
    }
}

unsigned long started_time = 0;
void loop() {
    started_time = millis();
    processPacket();
    sendModuleValues();

    while(millis() - started_time <= MINIMUM_LOOP_CYCLE) delay(1);
}

void actionGet(int idx, int port_idx, int device) {
    Port& port = ports[port_idx];
    changeModule(port, device);
}

bool actionSet(int idx, int port_idx, int device) {
    Port& port = ports[port_idx];

    changeModule(port, device);
    setModule(port);

    return true;
}

void actionReset(int idx, int port_idx, int device) {
    Port& port = ports[port_idx];
    changeModule(port, device);
    changeModule(port, ALIVE);
}

void initModule(Port& port, int device) {
    switch (device) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
            resetPort(port, OUTPUT, OUTPUT);
            break;

        case VOLUME:
        case SOUND:
        case BUTTON:
        case AMBIENT:
        case IRRANGE:
        case TOUCH:
            resetPort(port, INPUT, INPUT);
            break;

        case SERVO:
            if (port.devServo != NULL) {
                delete port.devServo;
            }
            port.devServo = new Servo();
            port.devServo->attach(port.digital_pin);
            break;

        case TONE:
            break;
        case MOTOR:
            break;
        case NEOPIXEL:
            if (port.devPixels != NULL) {
                delete port.devPixels;
            }

            port.devPixels = new Adafruit_NeoPixel(10, port.digital_pin, NEO_RGB + NEO_KHZ800);
            port.devPixels->begin();
            port.devPixels->clear();
            port.devPixels->show();
            break;
        case TEMPER:
            if (port.devDHT != NULL) {
                delete port.devDHT;
            }

            resetPort(port, INPUT, INPUT);
            port.devDHT = new SimpleDHT11(port.digital_pin);
            port.devDHT->read2(&port.lastTemperature, &port.lastHumidity, NULL);
            break;

        case ULTRASONIC:
            resetPort(port, INPUT, OUTPUT);
            port.ultrasonic = 0;
            break;

        case TEXTLCD:
            if (port.devLcd != NULL) {
                delete port.devLcd;
            }

            port.devLcd = new LCD1602(port.analog_pin, port.digital_pin);
            port.devLcd->begin();
            port.devLcd->setBacklight(HIGH);
            port.devLcd->home();
            break;

        case SEGMENT:
            if (port.devSegment != NULL) {
                delete port.devSegment;
            }

            port.devSegment = new TM1637Display(port.digital_pin, port.analog_pin);
            port.devSegment->setBrightness(0x0F);
            port.devSegment->clear();
            break;
    }

    port.status = device;
}

void delModule(Port& port) {
    switch (port.status) {
        case ALIVE:
        case BUZZER:
        case VOLUME:
        case SOUND:
        case BUTTON:
        case AMBIENT:
        case IRRANGE:
        case TOUCH:
        case TONE:
        case ULTRASONIC:
            // do nothing
            break;

        case NEOPIXEL:
            if (port.devPixels != NULL) {
                port.devPixels->clear();
                port.devPixels->show();
                delete port.devPixels;
                port.devPixels = NULL;
            }
            break;

        case SERVO:
            if (port.devServo != NULL) {
                port.devServo->detach();
                delete port.devServo;
                port.devServo = NULL;
            }
            break;

        case MOTOR:
            break;

        case TEMPER:
            if (port.devDHT != NULL) {
                delete port.devDHT;
                port.devDHT = NULL;
            }

        case TEXTLCD:
            if (port.devLcd != NULL) {
                //port.devLcd->setBacklight(LOW);
                port.devLcd->clear();
                delete port.devLcd;
                port.devLcd = NULL;
            }
            break;

        case SEGMENT:
            if (port.devSegment != NULL) {
                port.devSegment->clear();
                delete port.devSegment;
                port.devSegment = NULL;
            }
            break;
    }

    resetPort(port);
    port.status = ALIVE;
}

void setModule(Port& port) {
    switch (port.status) {
        case ALIVE:
            // do nothing
            break;

        case BUZZER:
            pinMode(port.digital_pin, OUTPUT);
            digitalWrite(port.digital_pin, readShort());
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
        case SERVO:
            if (port.devServo != NULL) {
                int v = readShort();
                if (v >= 0 && v <= 180) {
                    port.devServo->write(v);
                }
            }
            break;

        case TONE:
            break;
        case MOTOR:
            break;
        case NEOPIXEL:
            if (port.devPixels != NULL) {
                const byte i = readBuffer();
                const byte r = readBuffer();
                const byte g = readBuffer();
                const byte b = readBuffer();
                port.devPixels->setPixelColor(i, port.devPixels->Color(r, g, b));
                port.devPixels->show();
            }
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
        case TEXTLCD:
            if (port.devLcd != NULL) {
                const int line = readBuffer();
                char* text = readString();
                port.devLcd->setCursor(0, line);
                port.devLcd->print(text);
                free(text);
            }
            break;

        case SEGMENT:
            if (port.devSegment != NULL) {
                int num = readShort();
                int colon = readShort() * SEG_G;
                int val = num + colon * 10000;

                if (val != port.lastSegment) {
                    port.devSegment->showNumberDecEx(num, colon, false);
                    port.lastSegment = val;
                }
            }

            break;
    }
}

void sendDigitalStatus(Port& port) {
    writeHead();
    sendShort(digitalRead(port.digital_pin));
    writeSerial(port.index);
    writeSerial(port.status);
    writeEnd();
}

void sendAnalogStatus(Port& port) {
    writeHead();
    sendFloat(analogRead(port.analog_pin));
    writeSerial(port.index);
    writeSerial(port.status);
    writeEnd();
}

void sendUltrasonic(Port& port) {
    const int trigPin = port.digital_pin;
    const int echoPin = port.analog_pin;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin, LOW);

    float value = pulseIn(echoPin, HIGH) / 29.0 / 2.0;

    if (value == 0) {
        value = port.ultrasonic;
    } else {
        port.ultrasonic = value;
    }

    writeHead();
    sendShort(value);
    writeSerial(port.index);
    writeSerial(port.status);
    writeEnd();
}

void sendDHT11(Port& port) {
    port.devDHT->read2(&port.lastTemperature, &port.lastHumidity, NULL);

    writeHead();
    sendTwinFloat(port.lastTemperature, port.lastHumidity);
    writeSerial(port.index);
    writeSerial(port.status);
    writeEnd();
}

void sendModuleValue(Port& port) {
    switch (port.status) {
        case ALIVE:
            writeHead();
            sendText(CHECK_PHRASE);
            writeSerial(port.index);
            writeSerial(port.status);
            writeEnd();
            break;

        case BUZZER:
            // do nothing
            break;
        case VOLUME:
            sendAnalogStatus(port);
            break;
        case SOUND:
            sendDigitalStatus(port);
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
            sendDHT11(port);
            break;
        case ULTRASONIC:
            sendUltrasonic(port);
            break;
        case IRRANGE:
            sendAnalogStatus(port);
            break;
        case TOUCH:
            sendDigitalStatus(port);
            break;
        case TEXTLCD:
            // do nothing
            break;
        case SEGMENT:
            port.lastSegment = 0;
            break;
    }

}

void changeModule(Port& port, int device) {
    if (port.status == device) return;

    delModule(port);
    initModule(port, device);
}
