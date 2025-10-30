// ----- IR Transmitter + Receiver Self-Test -----
// Transmitter Pin
const int IR_LED = 11;
// Receiver Pin
const int IR_RECV = 12;

void setup() {
  Serial.begin(115200);
  pinMode(IR_LED, OUTPUT);
  pinMode(IR_RECV, INPUT);
  Serial.println("Starting IR self-test...");
}

void loop() {
  // --- Step 1: Emit 38kHz signal for 100ms ---
  unsigned long start = millis();
  while (millis() - start < 100) {
    // 38 kHz carrier (approx)
    digitalWrite(IR_LED, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_LED, LOW);
    delayMicroseconds(13);
  }

  // --- Step 2: Read receiver output ---
  int sensorValue = digitalRead(IR_RECV);

  // On most IR receiver modules (e.g., HX1838, VS1838B):
  // LOW = signal detected, HIGH = no signal
  if (sensorValue == LOW) {
    Serial.println("✅ Signal detected!");
  } else {
    Serial.println("❌ No signal detected!");
  }

  delay(1000); // wait 1 second and repeat
}
