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

void enableInterruptForSequence1() {
  enableInterrupt(BTN_GREEN, setWrongButton, CHANGE);
  enableInterrupt(BTN_YELLOW, setWrongButton, CHANGE);
  enableInterrupt(BTN_BLUE, setWrongButton, CHANGE);
  enableInterrupt(BTN_ORANGE, setWrongButton, CHANGE);
}

bool wrongButtonPressed() {
  return wrongButton;
}

void setup() {
  Serial.begin(PORT);
  setPin();
}

void loop() {
  switch (phase) {
    case 0:
      {
        enableInterruptForStartingGame();
        T1 = 2000;
        T2 = 5000;
        T3 = 10000;
        factor = 2;
        life = 1;
        points = 0;
        sleeping = false;
        phase = 1;
        gamestart = false;
        randomSeed(analogRead(A0));
        break;
      }
    case 1:
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
        disableAllInterrupts();
        enableInterruptForSequence();
        //resetSeq();
        phase = 2;
        break;
      }
    case 2:
      {
        //show pattern, life -1 if button is clicked missing
        resetSeq();
        Serial.println("GO!");
        delay(T1);
        int num = randomSeq();
        for (int i = 0; i < num; i++) {
          digitalWrite(generated[i], HIGH);
        }
        //testing
        /*for (int i = 0; i < num; i++) {
          Serial.println(generated[i]);
        }*/
        delay(T2);
        lightOut();
        phase = 3;
        break;
      }
    case 3:
      {
        long time = millis();
        //testing
        Serial.println("schiaccia");
        //
        do {
          //modifica array risultati.
          if (wrongButtonPressed()) {
            wrongButton = false;
            phase = 6;
            break;
          }
        } while (millis() - time < T3 && !checkWin());
        lightOut();
        phase = 4;
        break;
      }
    case 4:
      {
        //check if win or not.
        if (checkWin()) {
          phase = 5;
        } else {
          phase = 6;
        }
        break;
      }
    case 5:
      {
        //win
        points++;
        //T2 /= factor;
        //T3 /= factor;
        Serial.print("New point! Score: ");
        Serial.println(points);
        phase = 2;
        break;
      }
    case 6:
      {
        lightOut();
        //lost
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

/* Nella phase 1 vengono generati casualmente 
  i led che si devono illuminare e vengono accesi.*/


/*
    Nella phase 2 non si possono premere i bottoni
    perchè il gioco sta ancora facendo vedere i led
    illuminati casualmente.

    Basta mettere un interrupt al bottone che quando schiacciato
    va in una phase in cui hai perso la partita.
    */


/*
    nella phase 3 vengono schiacciati i bottoni e viene controllata
    che la sequenza dei bottoni premuti sia la stessa dei led accesi.
    */

/*
  if (!checkWin()) {
    lightOut();
    digitalWrite(LED_WHITE, HIGH);
    delay(1000);
    digitalWrite(LED_WHITE, LOW);
    life--;
    delay(1000);
    Serial.println("hai perso una vita, ti sono rimaste " + (String)life + " vite");
    delay(1000);
  }
  if (life == 0) {
    Serial.println("Game Over. Final Score: " + (String)points);
    delay(10000);
    phase = 0;
  }
  lightOut();
*/
//testing
/*Serial.println("fuori");
  for ( int i = 0; i < num; i++) {
    Serial.println(generated[i]);
  }*/

//la reset va messa perche senno i led rimangono accesi.
//resetSeq();

//