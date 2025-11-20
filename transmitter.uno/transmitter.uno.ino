// ------------------- Arduino A -------------------
#include <IRremote.h>
#include <Keypad.h>
#include <Servo.h>

#define PIN_RECV 12
#define PIN_SEND 11
#define SERVO_PIN A0

// ------------------- KEYPAD -------------------
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

// ------------------- SERVO -------------------
Servo myservo;

// ------------------- STATE VARIABLES -------------------
unsigned long lastNonSamsungTime = 0;
bool waitingForNEC = false;
bool handshakeDone = false;

uint8_t messageHEX = 0xFF;

// ------------------- MAP KEYPAD KEY → HEX -------------------
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
    default: return 0xFF;
  }
}

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(PIN_RECV);
  IrSender.begin(PIN_SEND);

  myservo.attach(SERVO_PIN);
  myservo.write(0);

  Serial.println("Arduino A Ready.");
}

void loop() {

  // ------------------- WAIT FOR KEYPAD PRESS -------------------
  if (!handshakeDone) {
    char key = keypad.getKey();
    if (key) {
      Serial.print("Key pressed: ");
      Serial.println(key);

      messageHEX = mapKeyToHEX(key);
      Serial.print("Mapped HEX: ");
      Serial.println(messageHEX, HEX);

      startScanningAndHandshake();   // <-- MAIN FUNCTION
    }
  }
}

// ==========================================================
//                   MAIN LOGIC FUNCTION
// ==========================================================
void startScanningAndHandshake() {

  Serial.println("Starting scanning + Samsung burst...");

  waitingForNEC = false;
  handshakeDone = false;

  // ---------------------------------------------------------
  // SCAN FROM 0 TO 90 DEGREES
  // ---------------------------------------------------------
  for (int angle = 0; angle <= 180; angle += 5) {

    myservo.write(angle);
    delay(300);

    if (handshakeDone) break;

    // Only send Samsung if NOT waiting for NEC
    if (!waitingForNEC) {
      IrSender.sendSamsung(0x0708, 0x55, 0);
      delay(200);
    }

    // ---------------- READ SIGNALS ----------------
    if (IrReceiver.decode()) {

      uint8_t proto = IrReceiver.decodedIRData.protocol;

      // ---------------- BEFORE NEC MODE ----------------
      if (!waitingForNEC) {

        // NOT Samsung → go into NEC waiting mode
        if (proto != SAMSUNG) {
          Serial.println("Non-Samsung detected → Waiting for NEC");
          waitingForNEC = true;
          lastNonSamsungTime = millis();
        }
      }

      // ---------------- WAITING FOR NEC ----------------
      else {
        if (proto == NEC) {
          Serial.println("NEC detected → Handshake Complete!");

          handshakeDone = true;

          // Send message to Arduino B
          IrSender.sendNEC(0x0102, messageHEX, 0);
          Serial.print("Sent message HEX: ");
          Serial.println(messageHEX, HEX);

          break;
        }
      }

      IrReceiver.resume();
    }

    // ---------------- NEC TIMEOUT ----------------
    if (waitingForNEC) {
      if (millis() - lastNonSamsungTime > 1000) {
        Serial.println("NEC timeout → resuming Samsung + rotation");
        waitingForNEC = false;
      }
      continue;
    }
  }

  // =====================================================
  // AFTER MESSAGE SENT → RESET SYSTEM
  // =====================================================
  if (handshakeDone) {
    Serial.println("Returning to home position...");

    myservo.write(0);
    delay(500);

    // Reset states for next key
    waitingForNEC = false;
    handshakeDone = false;
    messageHEX = 0xFF;

    Serial.println("Ready for next key!");
  }
}
