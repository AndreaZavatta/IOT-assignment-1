// C++ code
//

#define EI_ARDUINO_INTERRUPTED_PIN  // to enable pin states functionality
#include <EnableInterrupt.h>
#define BTN_BLUE 11
#define BTN_GREEN 10
#define BTN_ORANGE 9
#define BTN_YELLOW 8
#define LED_BLUE 7
#define LED_GREEN 6
#define LED_ORANGE 5
#define LED_YELLOW 4
#define LED_WHITE 3



int difficulty;
int brightness = 0;
int fadeamount = 5;
bool gamestart = false;


void setup() {
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN_YELLOW, INPUT);
  pinMode(BTN_ORANGE, INPUT);
  pinMode(BTN_GREEN, INPUT);
  pinMode(BTN_BLUE, INPUT);

  pinMode(A5, OUTPUT);

  enableInterrupt(11, game, CHANGE);
  Serial.begin(9600);
  pinMode(LED_WHITE, OUTPUT);
  func();
  digitalWrite(LED_WHITE, LOW);
  //read potenziometro.
  difficulty = analogRead(A5);
  Serial.println(difficulty);
}

void loop() {
}

void func() {
  while (!gamestart) {
    long startSec = millis();
    Serial.println("Ciao!");
    do {
      analogWrite(LED_WHITE, brightness);
      brightness += fadeamount;
      if (brightness == 0 || brightness == 255) {
        fadeamount = -fadeamount;
      }
      delay(15);
    } while (millis() - startSec < 10000 && !gamestart);
    if (!gamestart) {
      //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      //sleep_enable();
      //sleep_mode();
      Serial.println("dormendo");
    }
  }
}

void game() {
  gamestart = true;
}


