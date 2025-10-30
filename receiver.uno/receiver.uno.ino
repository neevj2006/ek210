#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Pins ===
const int IR_RECV = 12;   // IR receiver OUT (input)
const int IR_LED  = 11;   // IR transmitter SIG (output)

// === LCD Setup ===
LiquidCrystal_I2C lcd(0x27, 16, 2);  // change to 0x3F if needed

// === Timing (ms/us) - tweak in lab if needed ===
const unsigned long HANDSHAKE_BURST_US = 100000UL; // 100 ms handshake burst
const unsigned long IGNORE_AFTER_HANDSHAKE_MS = 200; // ignore own reflection after sending handshake
const unsigned long CODEWORD_GAP_MS = 600; // gap (silence) that indicates end of codeword

// === State variables ===
bool handshakeDone = false;     // we have sent handshake and now expect codeword
unsigned long lastEdgeMillis = 0; // last time we saw a falling edge (start of a burst)
unsigned long lastSeenMillis = 0; // last time we saw any LOW (within a burst)
unsigned long ignoreUntil = 0;    // don't count edges until this time (guard after sending handshake)
int frameCount = 0;               // counts bursts after handshake
bool currentlyLow = false;        // current level state to detect edges

void setup() {
  Serial.begin(9600);
  pinMode(IR_RECV, INPUT);
  pinMode(IR_LED, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("IR Decoder Ready");
  Serial.println("IR Decoder Ready");
}

// send 38kHz for given microseconds
void send38kHzBurst(unsigned long durationMicros) {
  unsigned long endMicros = micros() + durationMicros;
  while (micros() < endMicros) {
    digitalWrite(IR_LED, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_LED, LOW);
    delayMicroseconds(13);
  }
}

void loop() {
  int val = digitalRead(IR_RECV); // LOW => IR present
  unsigned long nowMs = millis();

  // detect falling edge (HIGH -> LOW) which marks start of a burst
  if (!currentlyLow && val == LOW) {
    // falling edge occurred
    currentlyLow = true;
    lastEdgeMillis = nowMs;
    lastSeenMillis = nowMs;
    // If we haven't handshaked yet -> this is the initial scanning beam
    if (!handshakeDone) {
      Serial.println("Initial beam detected -> sending handshake");
      send38kHzBurst(HANDSHAKE_BURST_US); // send single handshake burst back
      handshakeDone = true;
      frameCount = 0;                      // ensure we start fresh counting codeword
      ignoreUntil = nowMs + IGNORE_AFTER_HANDSHAKE_MS; // ignore reflections briefly
      // wait a tiny bit so we don't accidentally count the handshake as a frame
      delay(10);
    } else {
      // handshake already done -> this falling edge likely is start of a codeword burst
      // only count if we are past ignoreUntil guard
      if (nowMs >= ignoreUntil) {
        frameCount++;
        Serial.print("Burst counted. frameCount = ");
        Serial.println(frameCount);
      } else {
        Serial.println("Ignored edge (guard)");
      }
    }
  }

  // detect rising edge (LOW -> HIGH) marks end of a burst
  if (currentlyLow && val == HIGH) {
    currentlyLow = false;
    lastSeenMillis = nowMs;
  }

  // If we've handshakeDone and are counting bursts, wait for a silence long enough to
  // indicate the sender is finished. Then decode.
  if (handshakeDone && frameCount > 0) {
    if (nowMs - lastSeenMillis >= CODEWORD_GAP_MS) {
      // End of codeword detected
      Serial.print("End of codeword. frames = ");
      Serial.println(frameCount);
      char decoded = decodeFrameCount(frameCount);
      Serial.print("Decoded: "); Serial.println(decoded);

      // Display on LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Code Received:");
      lcd.setCursor(0,1);
      lcd.print(decoded);

      // Reset handshake for next round (as requested)
      handshakeDone = false;
      frameCount = 0;
      ignoreUntil = 0;
      Serial.println("Reset handshakeDone -> false. Ready for next cycle.");
    }
  }

  // Small debounce delay
  delay(2);
}

char decodeFrameCount(int frames) {
  // Map frames to symbol per your spec
  if (frames == 5) return '0';
  if (frames == 6) return '1';
  if (frames == 7) return '2';
  if (frames == 8) return '3';
  if (frames == 9) return '4';
  if (frames == 10) return '5';
  if (frames == 11) return '6';
  if (frames == 12) return '7';
  if (frames == 13) return '8';
  if (frames == 14) return '9';
  if (frames == 15) return '*';
  if (frames == 16) return '#';
  if (frames == 17) return 'A';
  if (frames == 18) return 'B';
  if (frames == 19) return 'C';
  if (frames == 20) return 'D';
  return '?';
}
