#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEGatt.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Adafruit_BLEGatt gatt(ble);

const int PIN_TRIG = 2;
const int PIN_ECHO = 3;
const int PIN_BUZZER = 5;

const int PIN_SWITCH = 6;

const int PIN_1A = A0;
const int PIN_2A = A1;
const int PIN_ENABLE_1_2 = 11;
const int PIN_3A = A2;
const int PIN_4A = A3;
const int PIN_ENABLE_3_4 = 10;

const int OBSTACLES_THRESHOLD = 30; // cm

int32_t serviceId;
int32_t characterId;

void setup(void) {
  Serial.begin(9600);
  setupPin();
  setupBLE();
}

void loop(void) {
  int switchValue = digitalRead(PIN_SWITCH);
  //Serial.println(switchValue);
  if (switchValue == HIGH) {
    ble.update(200);
  } else {
    autoPlay();
  }
}

void setupPin() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_SWITCH, INPUT);
  
  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_ENABLE_1_2, OUTPUT);
  pinMode(PIN_3A, OUTPUT);
  pinMode(PIN_4A, OUTPUT);
  pinMode(PIN_ENABLE_3_4, OUTPUT);  
}

void setupBLE() {
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if (!ble.begin(VERBOSE_MODE)) {
    error(F("Couldn't find Bluefruit, make sure it's in Command mode & check wiring?"));
  }
  if (!ble.factoryReset()){
       error(F("Couldn't factory reset"));
  }

  ble.echo(false);
  
  if (!ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Bluefruit"))) {
    error(F("Could not set device name?"));
  }

  boolean success;
  success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID128=EE-02-AC-5B-32-A0-0C-DD-DB-39-5D-3A-B4-33-6C-6D"), &serviceId);
  if (!success) {
    error(F("Could not add Custom Service"));
  }

  // 0x02 : Read
  // 0x04 : Write without a response
  // 0x08 : Write with a response
  // 0x10 : Notifications
  Serial.println(F("Adding the Custom Characteristic: "));
  success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID128=EE-02-34-A7-32-A0-0C-DD-DB-39-5D-3A-B4-33-6C-6D,PROPERTIES=0x08,MIN_LEN=5,MAX_LEN=5,DATATYPE=INTEGER,DESCRIPTION=number,VALUE=0"), &characterId);
  if (!success) {
    error(F("Could not add Custom Characteristic"));
  }

  uint8_t advdata[] = {0x03, 0x01, 0x06, 0x05, 0x02, 0x09, 0x18, 0x0a, 0x18};
  ble.setAdvData(advdata, sizeof(advdata));
  ble.sendCommandCheckOK(F("AT+GAPSETADVDATA=03-01-06-05-02-0d-18-0a-18"));

  ble.reset();
  
  ble.setBleGattRxCallback(characterId, BleGattRX);
  Serial.println("OK");
}

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void BleGattRX(int32_t chars_id, uint8_t data[], uint16_t len) {
//  Serial.print( F("[BLE GATT RX] (" ) );
//  Serial.print(chars_id);
//  Serial.print(") ");

  // 0x01FF00EE > left: 0xFF, right: 0xEE 
  if (chars_id == characterId) {
    if (data[4] > 0) { // buzzer
      tone(PIN_BUZZER, 442, 200);
      delay(400);
      tone(PIN_BUZZER, 442, 400);
      return;
    }
    int left = data[1];
    int right = data[3];
    if (data[0] == 0) {
      left = left * -1;
    }
    if (data[2] == 0) {
      right = right * -1;
    }
    Serial.println(left);
    Serial.println(right);
    rotate(left, right);
  }
}

bool isInitialized = false;
void autoPlay() {
  if (!isInitialized) {
    isInitialized = true;
    delay(3000);
    tone(PIN_BUZZER, 659, 500);
    delay(1000);
    tone(PIN_BUZZER, 659, 500);
    delay(1000);
    tone(PIN_BUZZER, 659, 500);
    delay(1000);
    tone(PIN_BUZZER, 1318, 1000);
    delay(1000);
  }
  
  if (isNearObstacles()) {
    avoidObstacles();
  } else {
    rotate(255, 255);
  }
  delay(100);
}

bool isNearObstacles() {
  int distance = getDistance();
  Serial.println(distance);
  return OBSTACLES_THRESHOLD > distance && distance > 0;
}

// return cm
int getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  float cm = pulseIn(PIN_ECHO, HIGH) / 58.0; // The echo time is converted into cm
  cm = (int(cm * 100.0)) / 100.0;     // Keep two decimal places
  // Serial.print("Distance\t=\t");
  // Serial.print(cm);
  // Serial.print("cm");
  // Serial.println();
  return cm;
}

void avoidObstacles() {
  rotate(-255, 255);
  tone(PIN_BUZZER, 442, 400);
  delay(500);
}

void rotate(int left, int right) {
  rotate(PIN_1A, PIN_2A, PIN_ENABLE_1_2, left);
  rotate(PIN_3A, PIN_4A, PIN_ENABLE_3_4, right);
}

void rotate(int pinX, int pinY, int pinEnable, int value) {
  if (value > 0) {
    digitalWrite(pinX, HIGH);
    digitalWrite(pinY, LOW);
  } else if (value < 0) {
    digitalWrite(pinX, LOW);
    digitalWrite(pinY, HIGH);
  }
  analogWrite(pinEnable, abs(value));
}
