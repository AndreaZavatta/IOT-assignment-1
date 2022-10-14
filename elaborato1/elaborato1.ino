// C++ code
#include <EnableInterrupt.h>
#include <avr/sleep.h>
#include"./header.h"
#define BTN_BLUE 11
#define BTN_GREEN 10
#define BTN_ORANGE 9
#define BTN_YELLOW 8
#define LED_BLUE 7
#define LED_GREEN 6
#define LED_ORANGE 5
#define LED_YELLOW 4
#define LED_WHITE 3
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 255
#define PORT 9600

bool sleeping;
int difficulty;
int brightness = 0;
int fadeamount = 5;
bool gamestart = false;
long prevts = 0;
int generated[4];
int T1 = 2000;
int T2 = 5000;
int T3 = 10000;

int life;
int points;

void disableAllInterrupts(){
  disableInterrupt(BTN_GREEN);
  disableInterrupt(BTN_YELLOW);
  disableInterrupt(BTN_ORANGE);
  disableInterrupt(BTN_BLUE);
}

void sleep() {
  sleeping = true;
  Serial.println("dormendo");
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  Serial.println("sveglio");
  sleep_disable();
}

int checkLed(int led) {
  for (int i = 0; i < 4; i++) {
    if (generated[i] == led){
      return i;
    }
  }
  return -1;
}

bool checkWin() {
  for (int i = 0; i < 4; i++) {
    if (generated[i] != 0) {
      return false;
    }
  }
  return true;
}

void pressBtn(int led) {
  long ts = micros();
  if (ts - prevts > 200000) {
    int i = checkLed(led);
    if (i != -1) {
      generated[i] = 0;
      digitalWrite(led, HIGH);
      /*if (checkWin()) {
        Serial.println("Win");
        points++;
      }*/
    }

    prevts = ts;
  }
}

void pressGreen(){
  pressBtn(LED_GREEN);
}

void pressYellow(){
  pressBtn(LED_YELLOW);
}

void pressOrange(){
  pressBtn(LED_ORANGE);
}

void pressBlue(){
  pressBtn(LED_BLUE);
}

void lightOut() {
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_ORANGE, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void fading() {
  analogWrite(LED_WHITE, brightness);
  brightness += fadeamount;
  if (brightness == MIN_BRIGHTNESS || brightness == MAX_BRIGHTNESS) {
    fadeamount = -fadeamount;
  }
  delay(15);
}

void changeSleep() {
  sleeping = false;
}

void game() {
  long ts = micros();
  if (ts - prevts > 200000) {
    if (sleeping) {
      changeSleep();
    } else {
      gamestart = true;
    }
    prevts = ts;
  }
}

bool checkGenerated(int pin) {
  for (int i = 0; i < 4; i++) {
    if (generated[i] == pin) {
      return true;
    }
  }
  return false;
}

//not working, you cant put more than one time a number (4,4,6,7 is possible).
int randomSeq() {
  int num = random(1,5);
  for (int i = 0; i < num; i++) {
    int pin = random(LED_YELLOW, LED_BLUE + 1);
    if (!checkGenerated(pin)) {
      generated[i] = pin;
    } else {
      i--;
    }
  }
  return num;
}

void resetSeq() {
  for (int i = 0; i < 4; i++) {
    generated[i] = 0;
  }
}

void setPin(){
  pinMode(LED_WHITE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN_YELLOW, INPUT);
  pinMode(BTN_ORANGE, INPUT);
  pinMode(BTN_GREEN, INPUT);
  pinMode(BTN_BLUE, INPUT);
  pinMode(A5, OUTPUT);
}

void enableInterruptForStartingGame(){
  enableInterrupt(BTN_YELLOW, changeSleep, CHANGE);
  enableInterrupt(BTN_ORANGE, changeSleep, CHANGE);
  enableInterrupt(BTN_GREEN, changeSleep, CHANGE);
  enableInterrupt(BTN_BLUE, game, CHANGE);
}

void enableInterruptForSequence(){
  enableInterrupt(BTN_GREEN, pressGreen, CHANGE);
  enableInterrupt(BTN_YELLOW, pressYellow, CHANGE);
  enableInterrupt(BTN_BLUE, pressBlue, CHANGE);
  enableInterrupt(BTN_ORANGE, pressOrange, CHANGE);
}

void setup() {
  Serial.begin(PORT);
  life = 3;
  points = 0;
  sleeping = false;
  setPin();
  enableInterruptForStartingGame();
  while (!gamestart) {
    long startSec = millis();
    Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
    do {
      fading();
    } while (millis() - startSec < 10000 && !gamestart);
    if (!gamestart) {
      sleep();
    }
  }
  //Serial.println("sto giocando!!");
  digitalWrite(LED_WHITE, LOW);
  //read potenziometro.
  difficulty = analogRead(A5);
  //Serial.println(difficulty);
  //Serial.println("difficlolta printata");
  disableAllInterrupts();
  enableInterruptForSequence();
  resetSeq();
}

void loop() {
  Serial.println("GO!");  
  delay(T1);
  int num = randomSeq();
  for ( int i = 0; i < num; i++) {
    digitalWrite(generated[i], HIGH);
  }
  //testing
  for ( int i = 0; i < num; i++) {
    Serial.println(generated[i]);
  }
  //
  delay(T2);
  lightOut();

  long time = millis();
  //testing
  Serial.println("schiaccia");
  //

  do{
    if(checkWin()){
      Serial.println("YOU WON");
      break;
    }
  } while(millis() - time < T3);
  lightOut();
  //testing
  Serial.println("fuori");
  for ( int i = 0; i < num; i++) {
    Serial.println(generated[i]);
  }
  //la reset va messa perche senno i led rimangono accesi.
  resetSeq();
  delay(5000);
  //
}
//missing -life, +points, printing points.
//missing random times, difficulty.