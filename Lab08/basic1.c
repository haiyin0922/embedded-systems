#define TIMER1_COMPA_vect _VECTOR(11) /* Timer/Counter1 Compare Match A */
#define INT0_vect _VECTOR(1) /* External Interrupt Request 0 */
#define INT1_vect _VECTOR(2) /* External Interrupt Request 1 */

// constants won't change. They're used here to set pin numbers:
const int ledPin_R = 4;      // the number of the LED pin
const int ledPin_B = 5;      // the number of the LED pin
const int button_int_R = 0;
const int button_int_B = 1;

// Variables will change:
int ledState_R = LOW;         // the current state of the output pin
int ledState_B = LOW;         // the current state of the output pin
int toggle_on_R = LOW;
int toggle_on_B = LOW;
float press_R = 0, press_B = 0;
unsigned long long cnt_R = 0, cnt_B = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin_R, OUTPUT);
  pinMode(ledPin_B, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin_R, ledState_R);
  digitalWrite(ledPin_B, ledState_B);

  // initialize timer1 
  cli(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 155;  // give 0.5 sec at 16 MHz/1024
  TIMSK1 |= (1<<OCIE1A);
  
  attachInterrupt(button_int_R, handle_click_R, CHANGE);
  attachInterrupt(button_int_B, handle_click_B, CHANGE);
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

  cnt_B++;
  if(toggle_on_B == HIGH){
    press_B += 0.01;
  }
  else if(toggle_on_B == LOW && press_B < 0.5 && cnt_B%50 == 0){
    press_B = 0;
    ledState_B = LOW;
  }
  else if(toggle_on_B == LOW && press_B >= 0.5 && cnt_B%50 == 0){
    press_B -= 0.5;
    ledState_B = !ledState_B;
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

void handle_click_B() { // button debouncing, toggle LED
  static unsigned long last_int_time_B = 0;
  unsigned long int_time_B = millis(); // Read the clock

  if (int_time_B - last_int_time_B > 200 ) {  
    // Ignore when < 200 msec
    toggle_on_B = !toggle_on_B;  // switch LED
    cnt_B = 0;
    if(toggle_on_B){
      press_B = 0;
      ledState_B = HIGH;    
    }
    else{
      Serial.print("Blue : ");
      Serial.print(press_B);
      Serial.println(" sec");
      ledState_B = LOW;
      press_B -= 0.5;
    }
  }
  
  last_int_time_B = int_time_B;
}

void loop() {
  digitalWrite(ledPin_R, ledState_R);
  digitalWrite(ledPin_B, ledState_B);
}
