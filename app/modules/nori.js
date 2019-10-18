function Module() {
    this.sp = null;
    this.sensorTypes = {
        ALIVE: 0,

        BUZZER: 1,
        VOLUME: 2,
        SOUND: 3,
        BUTTON: 4,
        AMBIENT: 5,
        SERVO: 6,

        TONE: 7,
        MOTOR: 8,
        NEOPIXEL: 9,
        TEMPER: 10,
        ULTRASONIC: 11,
        IRRANGE: 12,
        TOUCH: 13,
        TEXTLCD: 14,
        SEGMENT: 15,
    };

    this.actionTypes = {
        GET: 1,
        SET: 2,
        RESET: 3,
        CFG: 4,
    };

    this.sensorValueFormat = {
        INT8: 1,
        FLOAT: 2,
        SHORT: 3,
        TEXT: 4,
    };

    this.magicCode = new Buffer([255, 45]);

    this.digitalPortTimeList = [0, 0, 0, 0];

    this.sensorData = {
        PORT: {
            '0': 0,
            '1': 0,
            '2': 0,
            '3': 0,
        },
        DEBUG: {
            'type': 0,
            'port': 0,
            'value': 0,
        },
    };

    this.checkPhrase = 'HiNori!';

    this.defaultOutput = {};

    this.recentCheckData = {};

    this.sendBuffers = [];

    this.lastTime = 0;
    this.lastSendTime = 0;
    this.isDraing = false;
}

let packetIdx = 0;

Module.prototype.init = function(handler, config) {
};

Module.prototype.setSerialPort = function(sp) {
    this.sp = sp;
};

Module.prototype.requestInitialData = function() {
    return this.makeSensorReadBuffer(this.sensorTypes.ALIVE, 0);
};

Module.prototype.checkInitialData = function(data, _) {
    const datas = this.getDataByBuffer(data);

    return datas.some((data) => {
        const result = this.parsingPacket(data);
        if (!result) {
            return false;
        }

        return (
            result.port === 0 &&
            result.type === this.sensorTypes.ALIVE &&
            result.value === this.checkPhrase
        );
    });
};

Module.prototype.afterConnect = function(that, cb) {
    that.connected = true;
    if (cb) {
        cb('connected');
    }
};

Module.prototype.validateLocalData = function(_) {
    return true;
};

Module.prototype.requestRemoteData = function(handler) {
    const self = this;
    if (!self.sensorData) {
        return;
    }
    Object.keys(this.sensorData).forEach((key) => {
        if (self.sensorData[key] !== undefined) {
            handler.write(key, self.sensorData[key]);
        }
    });
};

Module.prototype.handleRemoteData = function(handler) {
    const self = this;
    const getDatas = handler.read('GET');
    const setDatas = handler.read('SET') || this.defaultOutput;
    let buffer = new Buffer([]);

    if (getDatas) {
        const keys = Object.keys(getDatas);
        keys.forEach((key) => {
            let isSend = false;
            const dataObj = getDatas[key];
            if (
                typeof dataObj.port === 'string' ||
                typeof dataObj.port === 'number'
            ) {
                const time = self.digitalPortTimeList[dataObj.port];
                if (dataObj.time > time) {
                    isSend = true;
                    self.digitalPortTimeList[dataObj.port] = dataObj.time;
                }
            } else if (Array.isArray(dataObj.port)) {
                isSend = dataObj.port.every((port) => {
                    const time = self.digitalPortTimeList[port];
                    return dataObj.time > time;
                });

                if (isSend) {
                    dataObj.port.forEach((port) => {
                        self.digitalPortTimeList[port] = dataObj.time;
                    });
                }
            }

            if (isSend) {
                if (!self.isRecentData(dataObj.port, key, dataObj.data)) {
                    self.recentCheckData[dataObj.port] = {
                        type: key,
                        data: dataObj.data,
                    };
                    buffer = Buffer.concat([
                        buffer,
                        self.makeSensorReadBuffer(
                            key,
                            dataObj.port,
                            dataObj.data,
                        ),
                    ]);
                }
            }
        });
    }

    if (setDatas) {
        const setKeys = Object.keys(setDatas);
        setKeys.forEach((port) => {
            const data = setDatas[port];
            if (data) {
                if (self.digitalPortTimeList[port] < data.time) {
                    self.digitalPortTimeList[port] = data.time;

                    if (!self.isRecentData(port, data.type, data.data)) {
                        self.recentCheckData[port] = {
                            type: data.type,
                            data: data.data,
                        };
                        buffer = Buffer.concat([
                            buffer,
                            self.makeOutputBuffer(data.type, port, data.data),
                        ]);

                        console.log(buffer);
                    }
                }
            }
        });
    }

    if (buffer.length) {
        this.sendBuffers.push(buffer);
    }
};

Module.prototype.isRecentData = function(port, type, data) {
    let isRecent = false;

    if (port in this.recentCheckData) {
        if (
            this.recentCheckData[port].type === type &&
            this.recentCheckData[port].data === data
        ) {
            isRecent = true;
        }
    }

    return isRecent;
};

Module.prototype.requestLocalData = function() {
    const self = this;

    if (!this.isDraing && this.sendBuffers.length > 0) {
        this.isDraing = true;
        this.sp.write(this.sendBuffers.shift(), () => {
            if (self.sp) {
                self.sp.drain(() => {
                    self.isDraing = false;
                });
            }
        });
    }

    return null;
};

// internal
Module.prototype.parsingPacket = function(data) {
    if (data.length <= 4 || data[0] !== this.magicCode[0] || data[1] !== this.magicCode[1]) {
        return null;
    }

    const readData = data.subarray(2, data.length);
    let value = 0;

    switch (readData[0]) {
        case this.sensorValueFormat.INT8: {
            value = new Buffer(readData.subarray(1, 2)).readInt8(0);
            value = Math.round(value * 100) / 100;
            break;
        }
        case this.sensorValueFormat.FLOAT: {
            value = new Buffer(readData.subarray(1, 5)).readFloatLE(0);
            value = Math.round(value * 100) / 100;
            break;
        }
        case this.sensorValueFormat.SHORT: {
            value = new Buffer(readData.subarray(1, 3)).readInt16LE(0);
            break;
        }
        case this.sensorValueFormat.TEXT: {
            const len = readData[1];
            value = new Buffer(readData.subarray(2, 2 + len)).toString();
            break;
        }
        default: {
            value = 0;
            break;
        }
    }

    const type = readData[readData.length - 1];
    const port = readData[readData.length - 2];

    return {
        value,
        type,
        port,
    };
};

/*
ff 2D idx size data a
*/
Module.prototype.handleLocalData = function(data) {
    const self = this;
    const datas = this.getDataByBuffer(data);

    datas.forEach((data) => {
        const result = self.parsingPacket(data);
        if (!result) {
            return;
        }

        if (result.type !== self.sensorTypes.ALIVE) {
            self.sensorData.PORT[result.port] = result.value;
        }

        // self.sensorData.DEBUG.type = result.type;
        // self.sensorData.DEBUG.port = result.port;
        // self.sensorData.DEBUG.value = result.value;
        //
        // if (result.port === 0) {
        //     console.log(self.sensorData.DEBUG);
        // }
    });
};


Module.prototype.makePacket = function(device, port, action, payload) {
    const dummy = new Buffer([10]);

    if (!payload) {
        return Buffer.concat([
            this.magicCode,
            new Buffer([
                5,
                packetIdx,
                action,
                device,
                port,
            ]),
            dummy,
        ]);
    } else {
        return Buffer.concat([
            this.magicCode,
            new Buffer([
                payload.length + 5,
                packetIdx,
                action,
                device,
                port,
            ]),
            payload,
            dummy,
        ]);
    }
};
/*
ff 2D len idx action device port  slot  data a
0  1  2   3   4      5      6     7     8
*/

Module.prototype.makeSensorReadBuffer = function(device, port, data) {
    let packet;

    if (data) {
        const value = new Buffer(2);
        value.writeInt16LE(data, 0);
        packet = this.makePacket(device, port, this.actionTypes.GET, value);
    } else {
        packet = this.makePacket(device, port, this.actionTypes.GET);
    }

    packetIdx++;
    if (packetIdx > 254) {
        packetIdx = 0;
    }
    //console.log(`read ${port} ${device} ${packet}`);

    return packet;
};

//0xff 0x55 0x6 0x0 0x1 0xa 0x9 0x0 0x0 0xa
//0xff 0x55 0x6 0x0 0x1 0xa 0x9 0x0 0x0 0xa
Module.prototype.makeOutputBuffer = function(device, port, data) {
    let payload = null;

    switch (device) {
        case this.sensorTypes.SEGMENT: {
            payload = new Buffer(4);
            if ($.isPlainObject(data)) {
                payload.writeInt16LE(data.value, 0);
                payload.writeInt16LE(data.colon, 2);
            } else {
                return this.makePacket(device, port, this.actionTypes.RESET);
            }
            break;
        }
        case this.sensorTypes.TEXTLCD: {
            if ($.isPlainObject(data)) {
                const textline = data.line || 0;
                const textdata = data.value || "";

                payload = Buffer.concat([
                    new Buffer([
                        textline,
                        textdata.length,
                    ]),
                    Buffer.from(textdata, 'utf8'),
                ]);
            } else {
                return this.makePacket(device, port, this.actionTypes.RESET);
            }
            break;
        }
        default: {
            payload = new Buffer(2);
            payload.writeInt16LE(data || 0, 0);
            break;
        }
    }
    console.log(`output ${port} ${device} ${payload.length}`);

    return this.makePacket(device, port, this.actionTypes.SET, payload);
};

Module.prototype.getDataByBuffer = function(buffer) {
    const datas = [];
    let lastIndex = 0;
    buffer.forEach((value, idx) => {
        if (value === 13 && buffer[idx + 1] === 10) {
            datas.push(buffer.subarray(lastIndex, idx));
            lastIndex = idx + 2;
        }
    });

    return datas;
};

Module.prototype.disconnect = function(connect) {
    const self = this;
    connect.close();
    if (self.sp) {
        delete self.sp;
    }
};

Module.prototype.reset = function() {
    this.lastTime = 0;
    this.lastSendTime = 0;

    this.sensorData.PORT = {
        '0': 0,
        '1': 0,
        '2': 0,
        '3': 0,
    };
};

module.exports = new Module();
