#include <Stepper.h>
#define TIMER1_COMPA_vect _VECTOR(11) /* Timer/Counter1 Compare Match A */
#define INT0_vect _VECTOR(1) /* External Interrupt Request 0 */
#define INT1_vect _VECTOR(2) /* External Interrupt Request 1 */
// for 28BYJ-48
#define STEPS 2048

// constants won't change. They're used here to set pin numbers:
const int button_int_STEP = 0;
const int button_int_RGB = 1;
const int redPin = 9, greenPin = 10, bluePin = 11;
const int motor_pin_1 = 5, motor_pin_2 = 7, motor_pin_3 = 6, motor_pin_4 = 8;
Stepper stepper(STEPS, motor_pin_1, motor_pin_2, motor_pin_3, motor_pin_4);
int xAxis = A0, yAxis = A1;

// Variables will change:
int r = 0, g = 0, b = 0;
int toggle_on_STEP = LOW;
int toggle_on_RGB = LOW;
float press_STEP = 0, press_RGB = 0;
unsigned long long cnt_RGB = 0;
int step_val = 0, xVal = 0, yVal = 0;

void setup() {
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT_PULLUP);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  

  // set initial LED state
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);  

  stepper.setSpeed(5); // set the speed to 30 RPMs

  // initialize timer1 
  cli(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 155;  // give 0.5 sec at 16 MHz/1024
  TIMSK1 |= (1<<OCIE1A);
  
  attachInterrupt(button_int_STEP, handle_click_STEP, CHANGE);
  attachInterrupt(button_int_RGB, handle_click_RGB, CHANGE);
  sei(); // enable all interrupts
}

ISR(TIMER1_COMPA_vect) { // wait for time up
  if(toggle_on_STEP == HIGH){
    press_STEP += 0.01;
  }
  else if(toggle_on_STEP == LOW && press_STEP > 0){
    press_STEP -= 0.01;
  }
   else if(toggle_on_STEP == LOW && press_STEP <= 0){
    step_val = 0;
    press_STEP = 0;
  }

  cnt_RGB++;
  if(toggle_on_RGB == HIGH){
    press_RGB += 0.01;
    r = xVal/4;  g = yVal/4;  b = 0;
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

void handle_click_STEP() { // button debouncing
  static unsigned long last_int_time_STEP = 0;
  unsigned long int_time_STEP = millis(); // Read the clock

  if (int_time_STEP-last_int_time_STEP > 200) {  
    // Ignore when < 200 msec
    toggle_on_STEP = !toggle_on_STEP;
    if(toggle_on_STEP){
      press_STEP = 0;
      step_val = 1;    
    }
    else{
      Serial.print("STEP : ");
      Serial.print(press_STEP);
      Serial.println(" sec");
      step_val = -1;
    }
  }
  
  last_int_time_STEP = int_time_STEP;
}

void handle_click_RGB() { // button debouncing, toggle LED
  static unsigned long last_int_time_RGB = 0;
  unsigned long int_time_RGB = millis(); // Read the clock

  if (int_time_RGB - last_int_time_RGB > 200 ) {  
    // Ignore when < 200 msec
    toggle_on_RGB = !toggle_on_RGB;  // switch LED
    cnt_RGB = 0;
    if(toggle_on_RGB){
      //r = 255;  g = 0;  b = 0;
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
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);

  stepper.step(step_val);

  xVal = analogRead(xAxis);
  yVal = analogRead(yAxis);
}
