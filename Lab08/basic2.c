#define TIMER1_COMPA_vect _VECTOR(11) /* Timer/Counter1 Compare Match A */
#define INT0_vect _VECTOR(1) /* External Interrupt Request 0 */
#define INT1_vect _VECTOR(2) /* External Interrupt Request 1 */

// constants won't change. They're used here to set pin numbers:
const int ledPin_R = 4;      // the number of the LED pin
const int button_int_R = 0;
const int button_int_RGB = 1;
const int redPin = 9, greenPin = 10, bluePin = 11;

// Variables will change:
int ledState_R = LOW;         // the current state of the output pin
int r = 0, g = 0, b = 0;         // the current state of the output pin
int toggle_on_R = LOW;
int toggle_on_RGB = LOW;
float press_R = 0, press_RGB = 0;
unsigned long long cnt_R = 0, cnt_RGB = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin_R, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  

  // set initial LED state
  digitalWrite(ledPin_R, ledState_R);
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);  

  // initialize timer1 
  cli(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 155;  // give 0.5 sec at 16 MHz/1024
  TIMSK1 |= (1<<OCIE1A);
  
  attachInterrupt(button_int_R, handle_click_R, CHANGE);
  attachInterrupt(button_int_RGB, handle_click_RGB, CHANGE);
  sei(); // enable all interrupts
}

ISR(TIMER1_COMPA_vect) { // wait for time up
  cnt_R++;
  if(toggle_on_R == HIGH){
    press_R += 0.01;
  }
  else if(toggle_on_R == LOW && press_R < 0.5 && cnt_R%50 == 0){
    press_R = 0;
    ledState_R = LOW;
  }
  else if(toggle_on_R == LOW && press_R >= 0.5 && cnt_R%50 == 0){
    press_R -= 0.5;
    ledState_R = !ledState_R;
  }

  cnt_RGB++;
  if(toggle_on_RGB == HIGH){
    press_RGB += 0.01;
  }
  else if(toggle_on_RGB == LOW && press_RGB < 0.5 && cnt_RGB%50 == 0){
    press_RGB = 0;
    r = 0;  g = 0;  b = 0;
  }
  else if(toggle_on_RGB == LOW && press_RGB >= 0.5 && cnt_RGB%50 == 0){
    press_RGB -= 0.5;
    if(r == 255){
      r = 0;  g = 0;  b = 0;
    }
    else{
      r = 255;  g = 255;  b = 0;
    }
  }
}

void handle_click_R() { // button debouncing, toggle LED
  static unsigned long last_int_time_R = 0;
  unsigned long int_time_R = millis(); // Read the clock

  if (int_time_R-last_int_time_R > 200) {  
    // Ignore when < 200 msec
    toggle_on_R = !toggle_on_R;  // switch LED
    cnt_R = 0;
    if(toggle_on_R){
      press_R = 0;
      ledState_R = HIGH;    
    }
    else{
      Serial.print("Red : ");
      Serial.print(press_R);
      Serial.println(" sec");
      ledState_R = LOW;
      press_R -= 0.5;
    }
  }
  
  last_int_time_R = int_time_R;
}

void handle_click_RGB() { // button debouncing, toggle LED
  static unsigned long last_int_time_RGB = 0;
  unsigned long int_time_RGB = millis(); // Read the clock

  if (int_time_RGB - last_int_time_RGB > 200 ) {  
    // Ignore when < 200 msec
    toggle_on_RGB = !toggle_on_RGB;  // switch LED
    cnt_RGB = 0;
    if(toggle_on_RGB){
      r = 255;  g = 0;  b = 0;
      press_RGB = 0;    
    }
    else{
      Serial.print("RGB : ");
      Serial.print(press_RGB);
      Serial.println(" sec");
      r = 0;  g = 0;  b = 0;
      press_RGB -= 0.5;
    }
  }
  
  last_int_time_RGB = int_time_RGB;
}

void loop() {
  digitalWrite(ledPin_R, ledState_R);

  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);  
}
