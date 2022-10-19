/*authors: 
Lorenzo Tosi - lorenzo.tosi10@studio.unibo.it - 0000997569
Andrea Zavatta - andrea.zavatta3@studio.uninbo.it - 0000989124
Luca Pasini - luca.pasini9@studio.unibo.it - 0000987673
*/
#include "Arduino.h"
#include "functions.h"
#include <EnableInterrupt.h>
#include <avr/sleep.h>

int generated[LED_NUMBER];
bool sleeping;
bool gamestart;
long prevts = 0;
int brightness = 0;
int fadeamount = 5;
bool wrongButton = false;
int points;
int life;
int T2;
int T3;

void switchOnRandomLeds() {
  int num = randomSeq();
  for (int i = 0; i < num; i++) {
    digitalWrite(generated[i], HIGH);
  }
}
bool clickAButton() {
  return digitalRead(BTN_BLUE) == HIGH || digitalRead(BTN_GREEN) == HIGH || digitalRead(BTN_ORANGE) == HIGH || digitalRead(BTN_YELLOW) == HIGH;
}

// Reduces the times by a factor chosen with the difficulty. The times can't go lower than their minimum.
int reduceByFactor(long int temp, int difficulty, long int min_temp) {
  int temp2 = temp - (temp * difficulty / 10);
  if (temp2 < min_temp) {
    temp2 = min_temp;
  }
  return temp2;
}
void setVariables() {
  //Randomly chooses the times between their max and their starting range.
  T2 = 4000;
  T3 = 4000;
  life = 3;
  points = 0;
  sleeping = false;
  gamestart = false;
  randomSeed(analogRead(A0));
  //after setting up all variables i can go to the phase 1 of the game.
  //phase = FADING;
}
void disableAllInterrupts() {
  disableInterrupt(BTN_GREEN);
  disableInterrupt(BTN_YELLOW);
  disableInterrupt(BTN_ORANGE);
  disableInterrupt(BTN_BLUE);
}

void setWrongButton() {
  wrongButton = true;
}

void sleep() {
  sleeping = true;
  digitalWrite(LED_WHITE, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
  //startSec = millis();
}

bool checkWin() {
  for (int i = 0; i < LED_NUMBER; i++) {
    if (generated[i] != 0) {
      return false;
    }
  }
  return true;
}

int checkLed(int led) {
  for (int i = 0; i < LED_NUMBER; i++) {
    if (generated[i] == led) {
      return i;
    }
  }
  return -1;
}

void pressGreen() {
  pressBtn(LED_GREEN);
}

void pressYellow() {
  pressBtn(LED_YELLOW);
}

void pressOrange() {
  pressBtn(LED_ORANGE);
}

void pressBlue() {
  pressBtn(LED_BLUE);
}

void pressBtn(int led) {
  int i = checkLed(led);
  digitalWrite(led, HIGH);
  if (i != -1) {
    generated[i] = 0;
  } else {
    setWrongButton();
  }
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

void game() {
  long ts = micros();
  if (ts - prevts > 400000) {
    if (sleeping) {
      changeSleep();
    } else {
      gamestart = true;
    }
    prevts = ts;
  }
}

void changeSleep() {
  sleeping = false;
}

int randomSeq() {
  int num = random(1, 5);
  for (int i = 0; i < num; i++) {
    int led = random(LED_YELLOW, LED_BLUE + 1);
    if (checkLed(led) == -1) {
      generated[i] = led;
    } else {
      i--;
    }
  }
  return num;
}

void resetSeq() {
  for (int i = 0; i < LED_NUMBER; i++) {
    generated[i] = 0;
  }
}

void setPin() {
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

void enableInterruptForStartingGame() {
  enableInterrupt(BTN_YELLOW, game, FALLING);
  enableInterrupt(BTN_ORANGE, changeSleep, FALLING);
  enableInterrupt(BTN_GREEN, changeSleep, FALLING);
  enableInterrupt(BTN_BLUE, changeSleep, FALLING);
}

void enableInterruptForSequence() {
  enableInterrupt(BTN_GREEN, pressGreen, FALLING);
  enableInterrupt(BTN_YELLOW, pressYellow, FALLING);
  enableInterrupt(BTN_BLUE, pressBlue, FALLING);
  enableInterrupt(BTN_ORANGE, pressOrange, FALLING);
}