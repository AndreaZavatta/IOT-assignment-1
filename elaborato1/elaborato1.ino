/*authors: 
Lorenzo Tosi - lorenzo.tosi10@studio.unibo.it - 0000997569
Andrea Zavatta - andrea.zavatta3@studio.uninbo.it - 0000989124
Luca Pasini - luca.pasini9@studio.unibo.it - 0000987673
*/
#include <EnableInterrupt.h>
#include <avr/sleep.h>
#include "./header.h"

bool sleeping;
int difficulty;  //the factor F
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
long startSec;

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
        setVariables();
        Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
        enableInterruptForStartingGame();
        //delay(500);
        startSec = millis();
        break;
      }
    case SLEEP:
      {
        if (millis() - startSec < 10000 && !gamestart) {
          fading();
        } else {
          phase = LED_BLINKING;
        }
        break;
      }
    //this is the idle phase, when the led is blinking and can go sleep.
    case LED_BLINKING:
      {
        //blinking red led, waiting for start game.
        if (!gamestart) {
          sleep();
          phase = SLEEP;
          startSec = millis();
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

        T1 = reduceByFactor(T1, difficulty, T1_MIN);
        T2 = reduceByFactor(T2, difficulty, T2_MIN);
        T3 = reduceByFactor(T3, difficulty, T3_MIN);
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
        Serial.println("Penality!");
        digitalWrite(LED_WHITE, HIGH);
        delay(2000);
        digitalWrite(LED_WHITE, LOW);
        if (life == 0) {
          phase = SETUP;
          Serial.print("Game Over. Final Score: ");
          Serial.println(points);
          delay(10000);
        } else {
          delay(1000);
          phase = RANDOM_LED;
        }
        break;
      }
    default:
      phase = SETUP;
      break;
  }
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
  T1 = random(T1_MAX - T_RANGE, T1_MAX + 1);
  T2 = 4000;
  T3 = 4000;
  life = 3;
  points = 0;
  sleeping = false;
  gamestart = false;
  randomSeed(analogRead(A0));
  //after setting up all variables i can go to the phase 1 of the game.
  phase = SLEEP;
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