#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h> 
// use pins 10 and 11 instead of Tx and Rx pins
SoftwareSerial mySerial(10, 11);

void cactiTask(void *pvParameters);
void LCDTask(void *pvParameters);
void buttonTask(void *pvParameters);

TaskHandle_t xHandle;

int x1 = random(8, 15), x2 = random(8, 15), num1, num2, pos_dinosaur = 0, score = 0;
byte dinosaur[8]  = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
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
  xTaskCreate(buttonTask, "keypad", 64, NULL, 1, NULL);
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
    vTaskDelay(50);
  }
}

void LCDTask(void *pvParameters){
  (void) pvParameters;
  const TickType_t xDelay = 2000/portTICK_PERIOD_MS;

  for( ;; ){
    if(x1 == 0 && pos_dinosaur == 0 || x2 == 0 && pos_dinosaur == 1){
      mySerial.print('2');
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
      mySerial.print('3');
      lcd.setCursor(x1, 0); lcd.write(1);
      lcd.setCursor(x2, 1); lcd.write(1);
      lcd.setCursor(0, pos_dinosaur); lcd.write(0);
      vTaskDelay(20); lcd.clear();
    }
  }
}

void buttonTask(void *pvParameters){
  (void) pvParameters;

  char data;
  for( ;; ){
    if(mySerial.available()){
      data = mySerial.read();
      if(data == '1' && pos_dinosaur == 0){
        pos_dinosaur = 1;
      }
      else if(data == '1' && pos_dinosaur == 1){
        pos_dinosaur = 0;
      }
    }
  }
}

void loop() {
}
