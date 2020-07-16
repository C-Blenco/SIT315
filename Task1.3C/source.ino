// Define pins
const byte motion_led_pin = 11;
const byte button_led_pin = 12;
const byte pir_pin = 2;
const byte button_pin = 3;

void setup()
{
  // Output LED
  pinMode(motion_led_pin, OUTPUT);
  pinMode(button_led_pin, OUTPUT);
  
  // Attach interrupt for PIR sensor
  attachInterrupt(digitalPinToInterrupt(pir_pin), motion_detected, CHANGE);
  attachInterrupt(digitalPinToInterrupt(button_pin), button_press, FALLING);
  
  // Initialise serial monitor
  Serial.begin(9600);
}

// State variables
int motion = 0;
int button = 0;
// Previous state variables
int motion_prev;
int button_prev;

void loop() {
  // Write led pins (if 0, off, if 1, on)
  digitalWrite(motion_led_pin, motion);
  digitalWrite(button_led_pin, button);
  
  // Print motion state if changed & motion detected
  if (motion_prev != motion && motion) {
    Serial.println("Motion Detected");
  }
  
  // Print button state if pressed
  if (button_prev != button) {
    Serial.println("Button pressed");
  }
 
  // Update previous states
  motion_prev = motion;
  button_prev = button;
  delay(100);
}


void motion_detected() {
  // Flip motion variable
  motion = !motion;
}

void button_press() {
  // Flip button variable
  button = !button;
}
