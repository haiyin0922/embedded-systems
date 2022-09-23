#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void cactiTask(void *pvParameters);
void LCDTask(void *pvParameters);
void leftTask(void *pvParameters);
void rightTask(void *pvParameters);
TaskHandle_t xHandle;

int threshold = 100, SPEED = 0;
int flagLeft = 0, flagRight = 0;
int x1 = 0, x2 = 0, num1, num2;
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, cactus);

  xTaskCreate(cactiTask, "cacti", 128, NULL, 1, &xHandle);
  xTaskCreate(LCDTask, "LCD", 128, NULL, 1, NULL);
  xTaskCreate(leftTask, "left", 128, NULL, 1, NULL);
  xTaskCreate(rightTask, "right", 128, NULL, 1, NULL);
}

void cactiTask(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    num1 = random(8, 15);
    num2 = random(8, 15);
    if(x1 < 1) x1 = num1;
    else x1 -= 1;
    if(x2 < 1) x2 = num2;
    else x2 -= 1;
    vTaskDelay(100 - SPEED*10);
  }
}

void LCDTask(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    lcd.setCursor(x1, 0); lcd.write(0);
    lcd.setCursor(x2, 1); lcd.write(0);
    if(flagLeft && flagRight) vTaskSuspend(xHandle);
    else vTaskResume(xHandle);
    vTaskDelay(10); lcd.clear();
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

void loop() {
}
