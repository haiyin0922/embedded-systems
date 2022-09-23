// constants won't change. They're used here to set pin numbers:
const int buttonPin_G = 5;    // the number of the pushbutton pin
const int buttonPin_B = 6;    // the number of the pushbutton pin
const int buttonPin_R = 7;    // the number of the pushbutton pin
const int ledPin_G = 2;      // the number of the LED pin
const int ledPin_B = 3;      // the number of the LED pin
const int ledPin_R = 4;      // the number of the LED pin

// Variables will change:
int ledState_G = LOW;         // the current state of the output pin
int ledState_B = LOW;         // the current state of the output pin
int ledState_R = LOW;         // the current state of the output pin
int buttonState_G;             // the current reading from the input pin
int buttonState_B;             // the current reading from the input pin
int buttonState_R;             // the current reading from the input pin
int lastButtonState_G = LOW;   // the previous reading from the input pin
int lastButtonState_B = LOW;   // the previous reading from the input pin
int lastButtonState_R = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime_G = 0, lastDebounceTime_B = 0, lastDebounceTime_R = 0;  // the last time the output pin was toggled
float press_G = 0, press_B = 0, press_R = 0;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin_G, INPUT);
  pinMode(ledPin_G, OUTPUT);
  pinMode(buttonPin_B, INPUT);
  pinMode(ledPin_B, OUTPUT);
  pinMode(buttonPin_R, INPUT);
  pinMode(ledPin_R, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin_G, ledState_G);
  digitalWrite(ledPin_B, ledState_B);
  digitalWrite(ledPin_R, ledState_R);

  // initialize timer1 
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // give 0.5 sec at 16 MHz/1024
  interrupts(); // enable all interrupts
}

void loop() {
  // read the state of the switch into a local variable:
  int reading_G = digitalRead(buttonPin_G);
  int reading_B = digitalRead(buttonPin_B);
  int reading_R = digitalRead(buttonPin_R);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading_G != lastButtonState_G) {
    // reset the debouncing timer
    lastDebounceTime_G = millis();
  }
  if (reading_B != lastButtonState_B) {
    // reset the debouncing timer
    lastDebounceTime_B = millis();
  }
  if (reading_R != lastButtonState_R) {
    // reset the debouncing timer
    lastDebounceTime_R = millis();
  }

  if ((millis() - lastDebounceTime_G) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
   
    // if the button state has changed:
    if (reading_G != buttonState_G) {
      buttonState_G = reading_G;

      // only toggle the LED if the new button state is HIGH
      if (buttonState_G == HIGH) {
        ledState_G = HIGH;
        press_G = 0;
      }
      else{
        Serial.print("Green : ");
        Serial.print(press_G);
        Serial.println(" sec");
      }
    }
  }

  if ((millis() - lastDebounceTime_B) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading_B != buttonState_B) {
      buttonState_B = reading_B;

      // only toggle the LED if the new button state is HIGH
      if (buttonState_B == HIGH) {
        ledState_B = HIGH;
        press_B = 0;
      }
      else{
        Serial.print("Blue : ");
        Serial.print(press_B);
        Serial.println(" sec");
      }
    }
  }

  if ((millis() - lastDebounceTime_R) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading_R != buttonState_R) {
      buttonState_R = reading_R;

      // only toggle the LED if the new button state is HIGH
      if (buttonState_R == HIGH) {
        ledState_R = HIGH;
        press_R = 0;
      }
      else{
        Serial.print("Red : ");
        Serial.print(press_R);
        Serial.println(" sec");
      }
    }
  }
  
  if(TCNT1 % 156 == 0){
    if(buttonState_G == HIGH) press_G += 0.01;
    if(buttonState_B == HIGH) press_B += 0.01;
    if(buttonState_R == HIGH) press_R += 0.01;
  }
  
  if (TIFR1 & (1 << OCF1A)) { // wait for time up
    if(buttonState_G == LOW && press_G > 0){
      if(ledState_G == HIGH) ledState_G = LOW;
      else ledState_G = HIGH;
      press_G -= 0.5;
    }
    else if(buttonState_G == LOW && press_G <= 0){
      ledState_G = LOW;
    }
    if(buttonState_B == LOW && press_B > 0){
      if(ledState_B == HIGH) ledState_B = LOW;
      else ledState_B = HIGH;
      press_B -= 0.5;
    }
    else if(buttonState_B == LOW && press_G <= 0){
      ledState_B = LOW;
    }
    if(buttonState_R == LOW && press_R > 0){
      if(ledState_R == HIGH) ledState_R = LOW;
      else ledState_R = HIGH;
      press_R -= 0.5;
    }
    else if(buttonState_R == LOW && press_R <= 0){
      ledState_R = LOW;
    }
    TIFR1 = (1<<OCF1A);
  } // clear overflow flag

    // set the LED:
  digitalWrite(ledPin_G, ledState_G);
  digitalWrite(ledPin_B, ledState_B);
  digitalWrite(ledPin_R, ledState_R);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState_G = reading_G;
  lastButtonState_B = reading_B;
  lastButtonState_R = reading_R;  
}
