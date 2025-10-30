#include <Keypad.h>
#include <Servo.h>

// === Pin Assignments ===
const int IR_LED = 11;
const int IR_RECV = 12;
const int SERVO_PIN = A0;

// === Keypad Setup ===
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

// === Servo Setup ===
Servo myservo;

// === Function Prototypes ===
void send38kHzBurst(unsigned long durationMicros);
bool checkReceiver();
void sendCodeword(int bursts);

void setup() {
  Serial.begin(115200);
  pinMode(IR_LED, OUTPUT);
  pinMode(IR_RECV, INPUT);
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  Serial.println("Arduino 1 Ready - Waiting for key input...");
}

void loop() {
  // 1. Wait for keypad input
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);

    // Map key to number of frames (codewords)
    int frames = 0;
    if (key >= '0' && key <= '9') frames = (key - '0') + 5;
    else if (key == '*') frames = 15;
    else if (key == '#') frames = 16;
    else if (key == 'A') frames = 17;
    else if (key == 'B') frames = 18;
    else if (key == 'C') frames = 19;
    else if (key == 'D') frames = 20;

    // 2. Rotate servo to search for Arduino 2 signal
    bool found = false;
    for (int angle = 0; angle <= 90; angle += 5) {
      myservo.write(angle);
      delay(300);  // allow servo to move
      send38kHzBurst(100000); // send a 100 ms IR burst

      if (checkReceiver()) {
        Serial.print("Signal detected at ");
        Serial.print(angle);
        Serial.println(" degrees!");
        found = true;
        break;
      }
    }

    if (found) {
      // 3. Send the codeword as IR bursts

      delay(1000);
      Serial.print("Sending codeword (");
      Serial.print(frames);
      Serial.println(" frames)...");
      sendCodeword(frames);
      Serial.println("Codeword sent.");
    } else {
      Serial.println("No receiver detected during scan.");
    }

    // Reset servo
    myservo.write(0);
    delay(1000);
  }
}

// === Helper: Emit 38 kHz carrier for a duration (µs) ===
void send38kHzBurst(unsigned long durationMicros) {
  unsigned long endMicros = micros() + durationMicros;
  while (micros() < endMicros) {
    digitalWrite(IR_LED, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_LED, LOW);
    delayMicroseconds(13);
  }
}

// === Helper: Check if IR receiver detects signal ===
bool checkReceiver() {
  // LOW = detected for most HX1838/V1838 modules
  int val = digitalRead(IR_RECV);
  return (val == LOW);
}

// === Helper: Send codeword as N bursts ===
void sendCodeword(int bursts) {
  for (int i = 0; i < bursts; i++) {
    send38kHzBurst(100000);  // 100 ms burst
    delay(100);              // gap between frames
  }
}
