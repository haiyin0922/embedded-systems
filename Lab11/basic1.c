#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <queue.h>
#define KEY_ROWS 4
#define KEY_COLS 4

void controlTask(void *p);
void displayTask(void *p);
QueueHandle_t Global_Queue_Handle = 0; //Global Handler

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};
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

  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, err_dragon);

  typedef struct {
    int x = 0;   // a data value
    int y = 0; // data source
  } pos;
  // Create a queue of 3 int
  Global_Queue_Handle = xQueueCreate(3, sizeof(pos));
  // Create tasks with priority 1 //
  xTaskCreate(controlTask,"control", 128, NULL, 1, NULL);
  xTaskCreate(displayTask,"display", 128, NULL, 1, NULL);

  vTaskStartScheduler();
}

void controlTask(void *p){
  typedef struct {
    int x = 0;   // a data value
    int y = 0; // data source
  } pos;
  pos dinosaur;
  
  while(1){
    char key = myKeypad.getKey();
    if(key == '2'){
      dinosaur.y = 0;
    }
    else if(key == '8'){
      dinosaur.y = 1;
    }
    else if(key == '4' && dinosaur.x > 0){
      dinosaur.x--;      
    }
    else if(key == '6' && dinosaur.x < 15){
      dinosaur.x++;
    }
    
    if(!xQueueSend(Global_Queue_Handle, &dinosaur, 1000)){
      Serial.println("Failed to send to queue");    
    }
  }
}

void displayTask(void *p){
  typedef struct {
    int x = 0;   // a data value
    int y = 0; // data source
  } pos;
  pos dinosaur;

  while(1){
    if(xQueueReceive(Global_Queue_Handle, &dinosaur, 1000)){
      lcd.clear();
      lcd.setCursor(dinosaur.x, dinosaur.y);
      lcd.write(0);
      vTaskDelay(2);
    }
    else{
      Serial.println("Failed to receive from queue");
    }
  }
}

void loop(){
}
