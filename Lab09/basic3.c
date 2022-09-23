#include <Arduino_FreeRTOS.h>

void TaskUltra(void *pvParameters);
void TaskLeft(void *pvParameters);
void TaskRight(void *pvParameters);
void Task7seg(void *pvParameters);

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[10][8] = { 
  {true, true, true, true, true, true, false, false}, //0
  {false, true, true, false, false, false, false, false}, //1
  {true, true, false, true, true, false, true, false}, //2
  {true, true, true, true, false, false, true, false}, //3
  {false, true, true, false, false, true, true, false}, //4
  {true, false, true, true, false, true, true, false}, //5
  {true, false, true, true, true, true, true, false}, //6
  {true, true, true, false, false, false, false, false}, //7
  {true, true, true, true, true, true, true, false}, //8
  {true, true, true, true, false, true, true, false} //9
};

int threshold = 100, SPEED = 0;
int flagLeft = 0, flagRight = 0;
const int redPin = 17, greenPin = 18, bluePin = 19, trigPin = 11, echoPin = 10;
int r = 0, g = 0, b = 0;
long duration;  int distance;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  xTaskCreate(TaskUltra, "Ultra", 128, NULL, 1, NULL);
  xTaskCreate(TaskLeft, "Left", 128, NULL, 2, NULL);
  xTaskCreate(TaskRight, "Right", 128, NULL, 2, NULL);
  xTaskCreate(Task7seg, "7seg", 128, NULL, 1, NULL);
}

void TaskUltra(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration*0.034 / 2;
    Serial.println(distance);
  }
}

void TaskLeft(void *pvParameters){
  (void) pvParameters;
  
  for( ;; ){
    int left = analogRead(A0);
    if(left < threshold && SPEED > 0 && flagRight == 0){
        flagLeft = 1;
        SPEED--;
    }
    flagLeft = 0; 
    vTaskDelay(50);    
  }
}

void TaskRight(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    int right = analogRead(A1);
     if(right < threshold && SPEED < 9 && flagLeft == 0){
      flagRight = 1;
      SPEED++;
    }
    flagRight = 0;
    vTaskDelay(50); 
  }
}

void Task7seg(void *pvParameters){
  (void) pvParameters;

  for( ;; ){
    if(distance > 15){
      SPEED = 0;
    }
    for(int i=0; i<8; i++){
      digitalWrite(pins[i], data[SPEED][i]==true ? HIGH : LOW);
    }
    if(SPEED < 4){
      r = 0;  g = 255;  b = 0;
    }
    else if(SPEED >= 7){
      r = 255;  g = 0;  b = 0;
    }
    else{
      r = 255;  g = 255;  b = 0;
    }
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
  }
}

void loop() {
  // Empty! Things are done in Tasks.
}
