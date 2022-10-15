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


bool doDelay;
bool sleeping;
int difficulty;
int brightness = 0;
int fadeamount = 5;
bool gamestart;
long prevts = 0;
int generated[LED_NUMBER];
int T1;
int T2;
int T3;
int phase = 0;
int life;
int points;
int factor;
bool wrongButton = false;

void disableAllInterrupts() {
  disableInterrupt(BTN_GREEN);
  disableInterrupt(BTN_YELLOW);
  disableInterrupt(BTN_ORANGE);
  disableInterrupt(BTN_BLUE);
}

void setWrongButton(){
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
void gameOver(){
  phase = LOSS;
  doDelay = false;
  
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
  enableInterrupt(BTN_GREEN, pressGreen, CHANGE);
  enableInterrupt(BTN_YELLOW, pressYellow, CHANGE);
  enableInterrupt(BTN_BLUE, pressBlue, CHANGE);
  enableInterrupt(BTN_ORANGE, pressOrange, CHANGE);
}
void enableInterruptForGameover(){
  enableInterrupt(BTN_GREEN, gameOver, CHANGE);
  enableInterrupt(BTN_YELLOW, gameOver, CHANGE);
  enableInterrupt(BTN_BLUE, gameOver, CHANGE);
  enableInterrupt(BTN_ORANGE, gameOver, CHANGE);
}

bool wrongButtonPressed() {
  return wrongButton;
}

void setup() {
  Serial.begin(PORT);
  setPin();
}

/*
#define SETUP 0;
#define LED_BLINKING 1;
#define RANDOM_LED 2;
#define CLICK_BUTTONS 3;
#define WIN 4;
#define LOSS 5;
*/
void loop() {
  switch (phase) {
    //setup the game variables and all other things you will need to reset when you lose.
    //this case will be called when game over.
    //should be complete.
    case SETUP:
      {
        enableInterruptForStartingGame();
        T1 = 2000;
        T2 = 5000;
        T3 = 10000;
        factor = 2;
        life = 3;
        points = 0;
        sleeping = false;
        gamestart = false;
        randomSeed(analogRead(A0));
        //after setting up all variables i can go to the phase 1 of the game.
        phase = 1;
        break;
      }
    //this is the idle phase, when the led is blinking and can go sleep.
    case LED_BLINKING:
      {
        //blinking red led, waiting for start game.
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
        digitalWrite(LED_WHITE, LOW);
        //read potenziometro.
        difficulty = analogRead(A5);
        
        
        //if game starts, we go to phase 2, where the pattern will be shown. 
        phase = 2;
        break;
      }
    case RANDOM_LED:
      {
        /*
          in questa fase vengono mostrati i led casuali
          quindi non possiamo schiacciare nessun bottone
          la variabile doDelay viene impostata qui a true,
          e viene modificata a false dall'interrupt in caso venga schiacciato
          un bottone (la funzione dell'interrupt si chiama gameOver())
        */
        doDelay = true;
        /*
          phase = 3 lo setto in questo punto, non si può settare sotto
          perchè la phase potrebbe essere modificata dall'interrupt
          in quanto, se viene schiacciato il bottone il programma deve andare
          nella phase 5
        */
        phase = 3;
        disableAllInterrupts();
        resetSeq();
        Serial.println("GO!");
        delay(T1);
        int num = randomSeq();
        for (int i = 0; i < num; i++) {
          digitalWrite(generated[i], HIGH);
        }
        
        enableInterruptForGameover();       
        long int time = millis();
        while(millis()-time<T2 && doDelay){} 
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
          if (wrongButtonPressed()) {
            wrongButton = false;
            phase = 5;
            break;
          }
        } while (millis() - time < T3 && !checkWin());
        delay(1000);
        lightOut();
        if (checkWin()) {
          phase = 4;
        } else {
          phase = 5;
        }
        break;
      }

    //win situation
    case WIN:
      {
        points++;
        //T2 /= factor;
        //T3 /= factor;
        Serial.print("New point! Score: ");
        Serial.println(points);
        phase = 2;
        break;
      }
    //lose situation
    case LOSS:
      {

        lightOut();
        life--;
        Serial.println("hai perso una vita, ti sono rimaste " + (String)life + " vite");
        digitalWrite(LED_WHITE, HIGH);
        delay(2000);
        digitalWrite(LED_WHITE, LOW);
        delay(1000);
        if (life == 0) {
          phase = 0;
          Serial.println("YOU LOST");
        } else {
          phase = 2;
        }
        break;
      }
    default:
      phase = 0;
      break;
  }
}
//manca il potenziometro, la questione del fattore che non ho ben capito cosa sia