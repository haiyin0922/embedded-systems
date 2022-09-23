#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <semphr.h>

void displayTask(void *p);
void dinoTask(void *p);
void cactusTask(void *p);

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};
const int xAxis = A0, yAxis = A1, button = 3;
char buf[2][16];
LiquidCrystal_I2C lcd(0x27, 16, 2);
SemaphoreHandle_t gatekeeper = 0;
SemaphoreHandle_t binary_sem;

int xVal, yVal, isPress;
int dino_x, dino_y, cact_x, cact_y;
int egg1_x, egg1_y, egg2_x, egg2_y, egg3_x, egg3_y;
int lay = 0, gameover = 0;

void setup() {
  Serial.begin(9600);
  pinMode(button, INPUT_PULLUP);
  randomSeed(analogRead(0));
  
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);

  gatekeeper = xSemaphoreCreateMutex();
  binary_sem = xSemaphoreCreateBinary();
  xTaskCreate(displayTask, "display", 128, NULL, 1, NULL);
  xTaskCreate(dinoTask, "dino", 128, NULL, 1, NULL);
  xTaskCreate(cactusTask, "cactus", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}

void displayTask(void *p){
  const TickType_t xDelay = 1000/portTICK_PERIOD_MS;
  
  while(1){
    lcd.clear();
    for(int i=0; i<2; i++){
      for(int j=0; j<16; j++){
        lcd.setCursor(j, i);
        if(gameover){
          lcd.clear();
          lcd.setCursor(0, 0);
          if(gameover == 1) lcd.print(F("Succeed!"));
          else if(gameover == 2) lcd.print(F("Game over"));
          vTaskDelay(xDelay);
          break;
        }        
        else if(buf[i][j] == 'd'){
          lcd.write(0);
        }
        else if(buf[i][j] == 'c'){
          lcd.write(1);
        }
        else if(buf[i][j] == 'e'){
          lcd.write(2);
        }
        else if(buf[i][j] == 'b'){
          lcd.write(3);
        }
      }
      if(gameover){
        gameover = 0;
        break;
      }
    }
    vTaskDelay(8);
  }
}

void dinoTask(void *p){
  const TickType_t xDelay = 1000/portTICK_PERIOD_MS;
  
  dino_x = random(0, 16);
  dino_y = random(0, 2);
  buf[dino_y][dino_x] = 'd';
  
  while(1){
    xVal = analogRead(xAxis);
    yVal = analogRead(yAxis);
    isPress = digitalRead(button);
    
    if(xSemaphoreTake(gatekeeper, 100)){
      if(isPress == 1 && buf[dino_y][dino_x] != 'e' && buf[dino_y][dino_x] != 'c' && !gameover){
        buf[dino_y][dino_x] = 'e';
        if(lay == 0){
          lay++;
          egg1_x = dino_x;
          egg1_y = dino_y;
        }
        else if(lay == 1){
          lay++;
          egg2_x = dino_x;
          egg2_y = dino_y;
        }
        else if(lay == 2){
          lay = 0;
          egg3_x = dino_x;
          egg3_y = dino_y;
          vTaskDelay(xDelay);
          gameover = 1; //win
          buf[dino_y][dino_x] = '0';
          buf[egg1_y][egg1_x] = '0';
          buf[egg2_y][egg2_x] = '0';
          buf[egg3_y][egg3_x] = '0';
          buf[cact_y][cact_x] = '0';
          dino_x = random(0, 16);
          dino_y = random(0, 2);
          buf[dino_y][dino_x] = 'd';
          cact_x = random(0, 16);
          cact_y = random(0, 2);
          buf[cact_y][cact_x] = 'c';
        }
      }
      else if(yVal < 100 && dino_y == 1 && !gameover){  //up
        if(buf[dino_y][dino_x] == 'd'){
          buf[dino_y][dino_x] = '0';
        }
        dino_y = 0;
        if(buf[dino_y][dino_x] != 'e' && buf[dino_y][dino_x] != 'c'){
          buf[dino_y][dino_x] = 'd';
        }
        xSemaphoreGiveFromISR(binary_sem, NULL);
      }
      else if(yVal > 900 && dino_y == 0 && !gameover){  //down
        if(buf[dino_y][dino_x] == 'd'){
          buf[dino_y][dino_x] = '0';
        }
        dino_y = 1;
        if(buf[dino_y][dino_x] != 'e' && buf[dino_y][dino_x] != 'c'){
          buf[dino_y][dino_x] = 'd';
        }
        xSemaphoreGiveFromISR(binary_sem, NULL);
      }
      else if(xVal < 100 && dino_x > 0 && !gameover){  //left
        if(buf[dino_y][dino_x] == 'd'){
          buf[dino_y][dino_x] = '0';
        }
        dino_x--;
        if(buf[dino_y][dino_x] != 'e' && buf[dino_y][dino_x] != 'c'){
          buf[dino_y][dino_x] = 'd';
        }
        xSemaphoreGiveFromISR(binary_sem, NULL);
      }
      else if(xVal > 900 && dino_x < 15 && !gameover){ //right
        if(buf[dino_y][dino_x] == 'd'){
          buf[dino_y][dino_x] = '0';
        }
        dino_x++;
        if(buf[dino_y][dino_x] != 'e' && buf[dino_y][dino_x] != 'c'){
          buf[dino_y][dino_x] = 'd';
        }
        xSemaphoreGiveFromISR(binary_sem, NULL);
      }
      
      xSemaphoreGive(gatekeeper); 
      vTaskDelay(30);
    }
    else{
      
    }
  }
}


void cactusTask(void *p){
  const TickType_t xDelay = 1000/portTICK_PERIOD_MS;
  
  cact_x = random(0, 16);
  cact_y = random(0, 2);
  buf[cact_y][cact_x] = 'c';
  
  while(1){
    if(xSemaphoreTake(binary_sem, portMAX_DELAY)){
      if(xSemaphoreTake(gatekeeper, 100)){
        if(buf[cact_y][cact_x] == 'c'){
          buf[cact_y][cact_x] = '0';
        }
        int r = random(0, 3);
        if(r == 0){
          cact_y = !cact_y; 
        }
        else if(r == 1){
          if(cact_x < 15) cact_x++;
          else cact_x--;
        }
        else if(r == 2){
          if(cact_x > 0) cact_x--;
          else cact_x++;
        }
        if(buf[cact_y][cact_x] == 'e'){
          buf[cact_y][cact_x] = 'b';
          vTaskDelay(xDelay);
          gameover = 2; //lose
          lay = 0;
          buf[dino_y][dino_x] = '0';
          buf[egg1_y][egg1_x] = '0';
          buf[egg2_y][egg2_x] = '0';
          buf[egg3_y][egg3_x] = '0';
          buf[cact_y][cact_x] = '0';
          cact_x = random(0, 16);
          cact_y = random(0, 2);
          dino_x = random(0, 16);
          dino_y = random(0, 2);
          buf[dino_y][dino_x] = 'd';
        }
        buf[cact_y][cact_x] = 'c';
        
        xSemaphoreGive(gatekeeper); 
      }
      else{
        
      }
       vTaskDelay(30);
    }               
  }
}

void loop(){
}
