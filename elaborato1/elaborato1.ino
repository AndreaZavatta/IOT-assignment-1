// C++ code
#include <EnableInterrupt.h>
#include <avr/sleep.h>
#include "./header.h"
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
#define LED_NUMBER 4
#define SETUP 0
#define LED_BLINKING 1
#define RANDOM_LED 2
#define CLICK_BUTTONS 3
#define WIN 4
#define LOSS 5
#define T1_MIN 500
#define T1_MAX 10000
#define T2_MIN 500
#define T2_MAX 10000
#define T3_MIN 2000
#define T3_MAX 20000
#define T_RANGE 5000


bool doDelay;
bool sleeping;
int difficulty;//the factor F
int brightness = 0;
int fadeamount = 5;
bool gamestart;
long prevts = 0;
int generated[LED_NUMBER];
int T1;
int T2;
int T3;
int phase;
int life;
int points;
bool wrongButton = false;

void setup() {
  Serial.begin(PORT);
  setPin();
  phase = SETUP;
}

void loop() {
  switch (phase) {
    //setup the game variables and all other things you will need to reset when you lose.
    //this case will be called when game over.
    case SETUP:
      {
        resetSeq();
        enableInterruptForStartingGame();
        //Randomly chooses the times between their max and their starting range.
        T1 = random(T1_MAX-T_RANGE, T1_MAX+1);
        T2 = random(T2_MAX-T_RANGE, T2_MAX+1);
        T3 = random(T3_MAX-T_RANGE, T3_MAX+1);
        life = 3;
        points = 0;
        sleeping = false;
        gamestart = false;
        randomSeed(analogRead(A0));
        //after setting up all variables i can go to the phase 1 of the game.
        phase = LED_BLINKING;
        break;
      }
    //this is the idle phase, when the led is blinking and can go sleep.
    case LED_BLINKING:
      {
        long startSec = millis();
        Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
        //blinking red led, waiting for start game.
        do {
          fading();
        } while (millis() - startSec < 10000 && !gamestart);
        if (!gamestart) {
          sleep();
        } else {
          digitalWrite(LED_WHITE, LOW);
          //read potenziometro.
          difficulty = (analogRead(A5) / 256) + 1;
          //if game starts, we go to phase 2, where the pattern will be shown.
          phase = RANDOM_LED;
        }
        break;
      }
    case RANDOM_LED:
      {
        resetSeq();
        disableAllInterrupts();
        Serial.println("GO!");
        delay(T1);
        int num = randomSeq();
        for (int i = 0; i < num; i++) {
          digitalWrite(generated[i], HIGH);
        }
        long int time = millis();
        phase = CLICK_BUTTONS;
        while (millis() - time < T2 && phase == CLICK_BUTTONS) {
          if (digitalRead(BTN_BLUE) == HIGH || digitalRead(BTN_GREEN) == HIGH || digitalRead(BTN_ORANGE) == HIGH || digitalRead(BTN_YELLOW) == HIGH) {
            phase = LOSS;
          }
        }
        lightOut();
        break;
      }
    //in this phase you will have to click the buttons. if you click the wrong button you will immediatly go to the lose section (6)
    //if you correctly recreate the pattern you will go to the win section (5)
    //if you dont do the pattern in time you will go to the lose section (6)
    case CLICK_BUTTONS:
      {
        disableAllInterrupts();
        enableInterruptForSequence();
        long time = millis();
        do {
          //if you click the wrong button the else in the pressBtn will be triggered.
          if (wrongButton) {
            wrongButton = false;
            phase = LOSS;
            disableAllInterrupts();
            break;
          }
        } while (millis() - time < T3 && !checkWin());
        disableAllInterrupts();
        delay(250);
        lightOut();
        if (checkWin()) {
          phase = WIN;
        } else {
          phase = LOSS;
        }
        break;
      }
    //win situation
    case WIN:
      {
        points++;
        // x% di T2
        // Reduces the times by a factor chosen with the difficulty. The times can't go lower than their minimum.
        T1 = T1 - (T1 * difficulty / 10);
        if (T1 < T1_MIN) {
          T1 = T1_MIN;
        }
        T2 = T2 - (T2 * difficulty / 10);
        if (T2 < T2_MIN) {
          T2 = T2_MIN;
        }
        T3 = T3 - (T3 * difficulty / 10);
        if (T3 < T3_MIN) {
          T3 = T3_MIN;
        }
        Serial.print("New point! Score: ");
        Serial.println(points);
        phase = RANDOM_LED;
        break;
      }
    //lose situation
    case LOSS:
      {
        lightOut();
        life--;
        Serial.println("You lost one Life! You have " + (String)life + " Lives left");
        digitalWrite(LED_WHITE, HIGH);
        delay(2000);
        digitalWrite(LED_WHITE, LOW);
        delay(1000);
        if (life == 0) {
          phase = SETUP;
          Serial.println("YOU LOST");
        } else {
          phase = RANDOM_LED;
        }
        break;
      }
    default:
      phase = SETUP;
      break;
  }
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
  Serial.println("dormendo");
  Serial.flush();
  digitalWrite(LED_WHITE, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  Serial.println("sveglio");
  sleep_disable();
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
  long ts = micros();
  if (ts - prevts > 200000) {
    int i = checkLed(led);
    digitalWrite(led, HIGH);
    if (i != -1) {
      generated[i] = 0;
    } else {
      setWrongButton();
    }
    prevts = ts;
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
  enableInterrupt(BTN_YELLOW, changeSleep, CHANGE);
  enableInterrupt(BTN_ORANGE, changeSleep, CHANGE);
  enableInterrupt(BTN_GREEN, changeSleep, CHANGE);
  enableInterrupt(BTN_BLUE, game, CHANGE);
}

void enableInterruptForSequence() {
  enableInterrupt(BTN_GREEN, pressGreen, FALLING);
  enableInterrupt(BTN_YELLOW, pressYellow, FALLING);
  enableInterrupt(BTN_BLUE, pressBlue, FALLING);
  enableInterrupt(BTN_ORANGE, pressOrange, FALLING);
}

/*la casualità del T1,T2,T3 e del factor (guardare la phase SETUP nel loop)
mancano poi le print giuste*/