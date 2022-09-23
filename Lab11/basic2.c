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
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);

  randomSeed(analogRead(0));

  typedef struct {
    int dino_x = -1;
    int dino_y = -1;
    int cact_x = -1;
    int cact_y = -1;
    int egg1_x = -1;
    int egg1_y = -1;
    int egg2_x = -1;
    int egg2_y = -1;
    int egg3_x = -1;
    int egg3_y = -1;
  } pos;
  // Create a queue of 3 int
  Global_Queue_Handle = xQueueCreate(10, sizeof(pos));
  // Create tasks with priority 1 //
  xTaskCreate(controlTask,"control", 128, NULL, 1, NULL);
  xTaskCreate(displayTask,"display", 128, NULL, 1, NULL);

  vTaskStartScheduler();
}

void controlTask(void *p){
  typedef struct {
    int dino_x = -1;
    int dino_y = -1;
    int cact_x = -1;
    int cact_y = -1;
    int egg1_x = -1;
    int egg1_y = -1;
    int egg2_x = -1;
    int egg2_y = -1;
    int egg3_x = -1;
    int egg3_y = -1;
  } pos;
  pos game;

  int gameover = 1;
  while(1){
    if(gameover){
      game.dino_x = random(0, 16);
      game.dino_y = random(0, 2);
      game.cact_x = random(0, 16);
      game.cact_y = random(0, 2);
      game.egg1_x = -1;
      game.egg1_y = -1;
      game.egg2_x = -1;
      game.egg2_y = -1;
      game.egg3_x = -1;
      game.egg3_y = -1;
      gameover = 0;
    }
    
    char key = myKeypad.getKey();
    if(key == '2' && game.dino_y == 1){
      game.dino_y = 0;
      int r = random(0, 3);
      if(r == 0){
        game.cact_y = !game.cact_y; 
      }
      else if(r == 1){
        if(game.cact_x < 15) game.cact_x++;
        else game.cact_x--;
      }
      else{
        if(game.cact_x > 0) game.cact_x--;
        else game.cact_x++;
      }
    }
    else if(key == '8' && game.dino_y == 0){
      game.dino_y = 1;
      int r = random(0, 3);
      if(r == 0){
        game.cact_y = !game.cact_y;
      }
      else if(r == 1){
        if(game.cact_x < 15) game.cact_x++;
        else game.cact_x--;
      }
      else{
        if(game.cact_x > 0) game.cact_x--;
        else game.cact_x++;
      }
    }
    else if(key == '4' && game.dino_x > 0){
      game.dino_x--;
      int r = random(0, 3);
      if(r == 0){
        game.cact_y = !game.cact_y;
      }
      else if(r == 1){
        if(game.cact_x < 15) game.cact_x++;
        else game.cact_x--;
      }
      else{
        if(game.cact_x > 0) game.cact_x--;
        else game.cact_x++;
      }    
    }
    else if(key == '6' && game.dino_x < 15){
      game.dino_x++;
      int r = random(0, 3);
       if(r == 0){
        game.cact_y = !game.cact_y;
      }
      else if(r == 1){
        if(game.cact_x < 15) game.cact_x++;
        else game.cact_x--;
      }
      else{
        if(game.cact_x > 0) game.cact_x--;
        else game.cact_x++;
      }
    }
    else if(key == '5'){
      if(game.egg1_x < 0){
        game.egg1_x = game.dino_x;
        game.egg1_y = game.dino_y;
      }
      else if(game.egg2_x < 0 && (game.dino_x != game.egg1_x || game.dino_y != game.egg1_y)){
        game.egg2_x = game.dino_x;
        game.egg2_y = game.dino_y;
      }
      else if(game.egg3_x < 0 && (game.dino_x != game.egg1_x || game.dino_y != game.egg1_y) 
                              && (game.dino_x != game.egg2_x || game.dino_y != game.egg2_y)){
        game.egg3_x = game.dino_x;
        game.egg3_y = game.dino_y;
        gameover = 1;
      }
    }
    if(game.cact_x == game.egg1_x && game.cact_y == game.egg1_y ||
       game.cact_x == game.egg2_x && game.cact_y == game.egg2_y){
      gameover = 1;  
    }
    
    if(!xQueueSend(Global_Queue_Handle, &game, 1000)){
      Serial.println("Failed to send to queue");    
    }    
  }
}

void displayTask(void *p){
  const TickType_t xDelay = 1000/portTICK_PERIOD_MS;
  typedef struct {
    int dino_x = -1;
    int dino_y = -1;
    int cact_x = -1;
    int cact_y = -1;
    int egg1_x = -1;
    int egg1_y = -1;
    int egg2_x = -1;
    int egg2_y = -1;
    int egg3_x = -1;
    int egg3_y = -1;
  } pos;
  pos game;

  while(1){
    if(xQueueReceive(Global_Queue_Handle, &game, 1000)){
      lcd.clear();
      lcd.setCursor(game.dino_x, game.dino_y);
      lcd.write(0);
      lcd.setCursor(game.cact_x, game.cact_y);
      lcd.write(1);
      if(game.cact_x == game.egg1_x && game.cact_y == game.egg1_y){
        lcd.setCursor(game.egg1_x, game.egg1_y);
        lcd.write(3);
        vTaskDelay(xDelay);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Game over"));
        vTaskDelay(xDelay);
      }
      else if(game.egg1_x >= 0){
        lcd.setCursor(game.egg1_x, game.egg1_y);
        lcd.write(2);
      }
      if(game.cact_x == game.egg2_x && game.cact_y == game.egg2_y){
        lcd.setCursor(game.egg2_x, game.egg2_y);
        lcd.write(3);
        vTaskDelay(xDelay);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Game over"));
        vTaskDelay(xDelay);
      }
      else if(game.egg2_x >= 0){
        lcd.setCursor(game.egg2_x, game.egg2_y);
        lcd.write(2);
      }
      if(game.egg3_x >= 0){
        lcd.setCursor(game.egg3_x, game.egg3_y);
        lcd.write(2);
        vTaskDelay(xDelay);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Succeed!"));
        vTaskDelay(xDelay);
      }
      
      vTaskDelay(3);
    }
    else{
      Serial.println("Failed to receive from queue");
    }
  }
}

void loop(){
}

