
void setup()
{
  // Output LED
  pinMode(13, OUTPUT);
  // Input from PIR sensor
  pinMode(4, INPUT);
  
  // Initialise serial monitor
  Serial.begin(9600);
}

// Initialise motion variable
int motion = 0;
int read;

void loop()
{ 
  // Read motion sensor state
  read = digitalRead(4);
  
  // Detect motion change to only print state when motion changes
  if (motion != read) {
    motion = read;
    Serial.println(motion);
  }
  
  // If motion is detected, set LED to on (HIGH) and print
  if (motion == HIGH) {
    Serial.println("Motion detected.");
    digitalWrite(13, HIGH);
  }
  // Otherwise set LED off (LOW)
  else {
    digitalWrite(13, LOW);
  }
  
  // Delay
  delay(100);
}
