// C++ code
//

#define EI_ARDUINO_INTERRUPTED_PIN  // to enable pin states functionality
#include <EnableInterrupt.h>
#include <avr/sleep.h>
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
  //func();
}

void abc(){
  Serial.println("abc");
}

void setup() {
  sleeping = false;
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN_YELLOW, INPUT);
  pinMode(BTN_ORANGE, INPUT);
  pinMode(BTN_GREEN, INPUT);
  pinMode(BTN_BLUE, INPUT);

  pinMode(A5, OUTPUT);

  //enableInterrupt(BTN_BLUE, wakeUp, RISING);
  enableInterrupt(BTN_YELLOW, changeSleep, CHANGE);
  enableInterrupt(BTN_ORANGE, changeSleep, CHANGE);
  enableInterrupt(BTN_GREEN, changeSleep, CHANGE);

  enableInterrupt(BTN_BLUE, game, CHANGE);
  disableInterrupt(BTN_GREEN);
  Serial.begin(PORT);
  pinMode(LED_WHITE, OUTPUT);
  func();
  digitalWrite(LED_WHITE, LOW);
  //read potenziometro.
  difficulty = analogRead(A5);
  Serial.println(difficulty);
  Serial.println("difficlolta printata");
}

void loop() {
}

void fading() {
  analogWrite(LED_WHITE, brightness);
  brightness += fadeamount;
  if (brightness == MIN_BRIGHTNESS || brightness == MAX_BRIGHTNESS) {
    fadeamount = -fadeamount;
  }
  delay(15);
}

void func() {
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
  Serial.println("sto giocando!!");
}

void changeSleep()
{
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