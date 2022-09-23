#include <IRremote.h>
const int redPin = 3, greenPin = 5, bluePin = 6, servoPin = 9, remotePin = 10, echoPin = 11, trigPin = 12;
long duration;  int distance;
// This is the time since the last rising edge in units of 0.5us.
// This is the pulse width we want in units of 0.5us.
uint16_t volatile servoTime = 0, servoHighTime = 3000;
// This is true if the servo pin is currently high.
boolean volatile servoHigh = false;
IRrecv irrecv(remotePin);
decode_results results;

void setup() {
  Serial.begin(9600);
  servoInit();
  servoSetPosition(0);
  irrecv.enableIRIn();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);  
}

void servoInit() {
  digitalWrite(servoPin, LOW);
  pinMode(servoPin, OUTPUT);   
  // Turn on CTC mode.  Timer 1 will count up to OCR1A, then
  // reset to 0 and cause an interrupt.
  TCCR1A = (1 << WGM21);
  // Set a 1:8 prescaler.  This gives us 0.5us resolution.
  TCCR1B = (1 << CS21);   
  // Put the timer in a good default state.
  TCNT1 = 0;
  OCR1A = 255;   
  TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt.
  sei();   // Enable interrupts.
}
 
void servoSetPosition(uint16_t highTimeMicroseconds) {
  TIMSK1 &= ~(1 << OCIE1A); // disable timer compare interrupt
  servoHighTime = highTimeMicroseconds * 2;
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
}

ISR(TIMER1_COMPA_vect) {
  // The time that passed since the last interrupt is OCR1A + 1
  // because the timer value will equal OCR1A before going to 0.
  servoTime += OCR1A + 1;
  static uint16_t highTimeCopy = 3000;
  static uint8_t interruptCount = 0;   
  if(servoHigh) {
    if(++interruptCount == 2) {
      OCR1A = 255;
    } 
    // The servo pin is currently high.
    // Check to see if is time for a falling edge.
    // Note: We could == instead of >=.
    if(servoTime >= highTimeCopy) {
      // The pin has been high enough, so do a falling edge.
      digitalWrite(servoPin, LOW);
      servoHigh = false;
      interruptCount = 0;
    }
  } 
  else {
    // The servo pin is currently low.     
    if(servoTime >= 40000) {
      // We've hit the end of the period (20 ms),
      // so do a rising edge.
      highTimeCopy = servoHighTime;
      digitalWrite(servoPin, HIGH);
      servoHigh = true;
      servoTime = 0;
      interruptCount = 0;
      OCR1A = ((highTimeCopy % 256) + 256)/2 - 1;
    }
  }
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2; 

  if (irrecv.decode(&results) && results.value == 0xFF02FD || distance < 10) {
    analogWrite(greenPin, 255);
    delay(1000);
    servoSetPosition(1000);
    delay(4500);
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    delay(500);
    servoSetPosition(0);
  }
  analogWrite(redPin, 0);  
  irrecv.resume();

  delay(500);
}
