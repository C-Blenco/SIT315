// Define pins
const byte led_pin = 13;
const byte pir_pin = 2;

void setup()
{
  // Output LED
  pinMode(led_pin, OUTPUT);
  
  // Attach interrupt for PIR sensor
  attachInterrupt(digitalPinToInterrupt(pir_pin), motion_detected, CHANGE);
  
  // Initialise serial monitor
  Serial.begin(9600);
}

// Initialise motion variable as state 0 (LOW)
int motion = 0;

void loop() {
  // Write led pin (if 0, off, if 1, on)
  digitalWrite(led_pin, motion);
  
  // Print state
  Serial.println("Motion: " + String(motion));
}

void motion_detected() {
  // Flip motion variable
  motion = !motion;
}