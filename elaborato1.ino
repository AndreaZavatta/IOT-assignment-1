// C++ code
//

#define EI_ARDUINO_INTERRUPTED_PIN // to enable pin states functionality 
#include <EnableInterrupt.h>


int difficulty;
int brightness = 0;
int fadeamount = 5;
bool gamestart = false;

void game()
{
  gamestart = true;
}

void func()
{
  while (!gamestart){
    long startSec = millis();
    Serial.println("Ciao!");
    
    while(millis() - startSec < 10000 && !gamestart)
    {
      analogWrite(3, brightness);
      brightness += fadeamount;
      if (brightness == 0 || brightness == 255){
        fadeamount = -fadeamount;
      }
      delay(15);
    }
    if(!gamestart) {
      //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      //sleep_enable(); 
      //sleep_mode();
      Serial.println("dormendo");
    }
  }
}

void setup()
{
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);

  pinMode(A5, OUTPUT);

  enableInterrupt(11, game, CHANGE);
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  func();
  analogWrite(3,0);
  //read potenziometro.
  difficulty = analogRead(A5);
  Serial.println(difficulty);
}

void loop()
{ 
  
}