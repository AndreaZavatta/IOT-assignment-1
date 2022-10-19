/*authors: 
Lorenzo Tosi - lorenzo.tosi10@studio.unibo.it - 0000997569
Andrea Zavatta - andrea.zavatta3@studio.uninbo.it - 0000989124
Luca Pasini - luca.pasini9@studio.unibo.it - 0000987673
*/

#include <avr/sleep.h>
#include "functions.h"

extern bool gamestart;
extern int generated[LED_NUMBER];
extern int T2;
extern int T3;
extern int life;
extern int points;
extern bool wrongButton;
long startSec;
int T1;
int phase;
int difficulty;  //the factor F

void setup() {
  Serial.begin(PORT);
  setPin();
  phase = SETUP;
}

void loop() {
  switch (phase) {
    //setup the game variables and all other things you will need to reset when you lose.
    case SETUP:
      {
        resetSeq();
        setVariables();
        Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
        enableInterruptForStartingGame();
        startSec = millis();
        phase = FADING;
        break;
      }
    // Fading phase:
    //where the led keeps fading until the 10-second time runs out
    //or the player has clicked the button, to start the game
    case FADING:
      {
        if (millis() - startSec < 10000 && !gamestart) {
          fading();
        } else {
          phase = SLEEP;
        }
        break;
      }
    //Sleep phase:
    //if the player has not clicked button 1, then the game has not started
    //and we return to the Fading phase by resetting the timer
    //otherwise, turn off LED_WHITE and set the difficulty
    case SLEEP:
      {
        //blinking red led, waiting for start game.
        if (!gamestart) {
          sleep();
          phase = FADING;
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
    //Random phase:
    //displays the sequence of random LEDs to be executed
    case RANDOM_LED:
      {
        resetSeq();
        disableAllInterrupts();
        Serial.println("GO!");
        T1 = random(T1_MAX - T_RANGE, T1_MAX + 1);
        delay(T1);
        switchOnRandomLeds();
        startSec = millis();
        phase = WAIT_RANDOM_LED;
        break;
      }
      //WAIT_RANDOM_LED phase:
      //while you are viewing the random sequence,
      //if you click on a button, then you go to the phase loss
      //if not, we continue

    case WAIT_RANDOM_LED:
      {
        if (millis() - startSec < T2) {
          if (clickAButton()) {
            phase = LOSS;
          }
        } else {
          phase = CHECK_BUTTON;
          lightOut();
          disableAllInterrupts();
          enableInterruptForSequence();
          startSec = millis();
        }
        break;
      }
    //CHECK_BUTTON phase:
    //checks whether in the phase where buttons are to be typed, the player has pressed //a wrong button, in which case he is sent to the LOSS phase
    case CHECK_BUTTON:
      {
        if (millis() - startSec < T3 && !checkWin()) {
          if (wrongButton) {
            wrongButton = false;
            phase = LOSS;
            disableAllInterrupts();
          }
        } else {
          phase = CLICK_BUTTONS;
        }
        break;
      }
    //CLICK_BUTTON phase
    //if you correctly recreate the pattern you will go to the WIN phase
    //if you dont do the pattern in time you will go to the LOSS phase
    case CLICK_BUTTONS:
      {
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
    //WIN phase
    //Reduces T2 and T3 time to increase difficulty ,
    //increases the score
    //and returns to the stage devoted to random led generation.
    case WIN:
      {
        points++;
        T2 = reduceByFactor(T2, difficulty, T2_MIN);
        T3 = reduceByFactor(T3, difficulty, T3_MIN);
        Serial.print("New point! Score: ");
        Serial.println(points);
        phase = RANDOM_LED;
        break;
      }
    //LOSS phase
    //takes away one life
    //and checks whether the player has any life left or has lost.
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