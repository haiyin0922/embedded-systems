#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#define KEY_ROWS 4
#define KEY_COLS 4

void cactiTask(void *pvParameters);
void LCDTask(void *pvParameters);
void leftTask(void *pvParameters);
void rightTask(void *pvParameters);
void keypadTask(void *pvParameters);
TaskHandle_t xHandle;

int threshold = 100, SPEED = 0;
int flagLeft = 0, flagRight = 0, susp = 0;
int x1 = random(8, 15), x2 = random(8, 15), num1, num2, pos_dinosaur = 0, score = 0;
byte dinosaur[8]  = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
LiquidCrystal_I2C lcd(0x27, 16, 2);
char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte colPins[KEY_COLS] = {9, 8, 7, 6};
byte rowPins[KEY_ROWS] = {13, 12 ,11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, dinosaur);
  lcd.createChar(1, cactus);
  lcd.setCursor(0, 0);  lcd.write(0);
  x1 = random(8, 15);
  x2 = random(8, 15);
  
  xTaskCreate(cactiTask, "cacti", 64, NULL, 1, &xHandle);
  xTaskCreate(LCDTask, "LCD", 128, NULL, 1, NULL);
  xTaskCreate(leftTask, "left", 64, NULL, 1, NULL);
  xTaskCreate(rightTask, "right", 64, NULL, 1, NULL);
  xTaskCreate(keypadTask, "keypad", 64, NULL, 1, NULL);
}

void cactiTask(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    num1 = random(8, 15);
    num2 = random(8, 15);
    if(x1 < 1) {
      x1 = num1;
      score++;
    }
    else x1 -= 1;
    if(x2 < 1) {
      x2 = num2;
      score++;
    }
    else x2 -= 1;
    vTaskDelay(100 - SPEED*10);
  }
}

void LCDTask(void *pvParameters){
  (void) pvParameters;
  const TickType_t xDelay = 2000/portTICK_PERIOD_MS;

  for( ;; ){
    if(x1 == 0 && pos_dinosaur == 0 || x2 == 0 && pos_dinosaur == 1){
      lcd.setCursor(0, 0);
      lcd.print(F("Game Over"));
      lcd.setCursor(0, 1);
      lcd.print(F("Score: ")); lcd.print(score);
      vTaskDelay(xDelay); lcd.clear();
      pos_dinosaur = 0;
      x1 = random(8, 15); x2 = random(8, 15);
      score = 0;
    }
    else{
      lcd.setCursor(x1, 0); lcd.write(1);
      lcd.setCursor(x2, 1); lcd.write(1);
      lcd.setCursor(0, pos_dinosaur); lcd.write(0);
      if(flagLeft && flagRight && susp == 0){
        vTaskSuspend(xHandle);
        susp = 1;
      }
      else if((!flagLeft || !flagRight) && susp == 1){
        vTaskResume(xHandle);
        susp = 0;
      }
      vTaskDelay(20); lcd.clear();
    }
  }
}

void leftTask(void *pvParameters){
  (void) pvParameters;
  
  for( ;; ){
    int left = analogRead(A1);
    if(left < threshold){
      flagLeft = 1;
      if(flagRight == 0 && SPEED > 0){
        SPEED--;
        Serial.println(SPEED);
      }
    }
    else flagLeft = 0; 
    vTaskDelay(50);    
  }
}

void rightTask(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    int right = analogRead(A0);
    if(right < threshold){
      flagRight = 1;
      if(flagLeft == 0 && SPEED < 8 && flagLeft == 0){
        SPEED++;
        Serial.println(SPEED);
      }     
    }
    else flagRight = 0;
    vTaskDelay(50); 
  }
}

void keypadTask(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    char key = myKeypad.getKey();
    if(key == '2'){
      pos_dinosaur = 0;
      Serial.println(key);
    }
    else if(key == '8'){
      pos_dinosaur = 1;
      Serial.println(key);
    }
  }
}

void loop() {
}
