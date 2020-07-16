const unsigned int t_load = 0;
const unsigned int t_compare = 100000000;

void setup()
{
  DDRB |=  (1 << PB0); // Led PIN 8 input
  DDRB |=  (1 << PB1); // Led PIN 9 input
  DDRB |=  (1 << PB2); // Led PIN 10 input 
  
  DDRD &= ~(1 << PD4); // Button 3 input
  DDRD &= ~(1 << PD5); // Button 2 input
  DDRD &= ~(1 << PD6); // Button 1 input
  
  // Setup timer1
  timer_setup();
  
  // Enable global interrupts
  SREG |= (1 << 7);
  // enable interrupts on D0-D7
  PCICR |= (1 << PCIE2);
  // enable interrupts on pins D4,5,6
  PCMSK2 |= (1 << PCINT20); 
  PCMSK2 |= (1 << PCINT21); 
  PCMSK2 |= (1 << PCINT22);
}

void timer_setup() {
  // Setup timer1
  TCCR1A = 0; //set TCCR1A register to 0
  
  // set prescaler
  TCCR1B |= (1 << CS12); 
  TCCR1B &= ~(1 << CS11); 
  TCCR1B &= ~(1 << CS10); 
  
  TCNT1 = t_load; // set counter to 0
  OCR1A = t_compare; // set comparison time
  
  // Enable compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void loop() 
{  
  delay(500);
}

// triggers when D0-D7 are triggered
ISR(PCINT2_vect)
{
  // If statements toggle LED's if the corresponding pin is HIGH
  
  // Button 3
  if((PIND & (1 << PD4))) {
    PORTB^= (1 << PB0);
  }
  // Button 2
  if((PIND & (1 << PD5))) {
    PORTB^= (1 << PB1);
  }
  // Button 1
  if((PIND & (1 << PD6))) {
    PORTB^= (1 << PB2);
  }
}

ISR(TIMER1_COMPA_vect)
{
  PORTB^= (1 << PB0); // toggle red led
  TCNT1 = t_load; // reset timer
}



