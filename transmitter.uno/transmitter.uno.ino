// Arduino A

#include <IRremote.h>
#include <Keypad.h>
#include <Servo.h>

#define PIN_RECV 12
#define PIN_SEND 11

const int SERVO_PIN = A0;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'},
  {'A','B','C','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Servo myservo;

unsigned long lastNonSamsungTime = 0;
bool waitingForNEC = false;

uint8_t mapKeyToHEX(char key);

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(PIN_RECV);
  IrSender.begin(PIN_SEND);
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  Serial.println("Arduino 1 Ready - Waiting for key input...");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);

    uint8_t message = mapKeyToHEX(key);
    Serial.print("Message: ");
    Serial.println(message);
    
    bool handshakeComplete = false;
    for (int angle = 0; angle <= 90; angle += 5) {
      myservo.write(angle);
      delay(300);

      // ---------------- SEND SAMSUNG CONTINUOUSLY ----------------
      IrSender.sendSamsung(0x0708, 0x55, 0);
      delay(200);

      if (IrReceiver.decode()) {

        uint8_t proto = IrReceiver.decodedIRData.protocol;

        if (!waitingForNEC) {
          // Normal Samsung sending mode
          if (proto != SAMSUNG) {
            Serial.println("Non-Samsung detected, stopping Samsung burst");
            waitingForNEC = true;
            lastNonSamsungTime = millis();
          }
        } 
        else {
          // Waiting for NEC
          if (proto == NEC) {
            Serial.println("NEC detected → Handshake complete!");
            handshakeComplete = true;   // << STOP everything
            IrSender.sendSamsung(0x0708, message,0);
            return;
          }
        }

        IrReceiver.resume();
      }

      // ---------------- WAIT FOR NEC TIMEOUT ----------------
      if (waitingForNEC) {
        if (millis() - lastNonSamsungTime > 1000) {
          Serial.println("No NEC detected → resuming Samsung burst");
          waitingForNEC = false;
        }
        return; // do NOT send Samsung while waiting
      }
    }
  }
}

uint8_t mapKeyToHEX(char key) {
  switch (key) {
    case '0': return 0x00;
    case '1': return 0x01;
    case '2': return 0x02;
    case '3': return 0x03;
    case '4': return 0x04;
    case '5': return 0x05;
    case '6': return 0x06;
    case '7': return 0x07;
    case '8': return 0x08;
    case '9': return 0x09;
    case '*': return 0x0A;
    case '#': return 0x0B;
    case 'A': return 0x0C;
    case 'B': return 0x0D;
    case 'C': return 0x0E;
    case 'D': return 0x0F;
    default:
      return 0xFF; // invalid character
  }
}
