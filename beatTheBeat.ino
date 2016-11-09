#include <AsciiMassenger.h>
#include <Adafruit_NeoPixel.h>
#include <Chrono.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN         6

#define NUMPIXELS   60


AsciiMassenger msg;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsEnnemis = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//-----------Level-------------------------------------------------------------//
//--lvl.1------------------------------------------------------------------------//
int start1[] = {8, 16, 24, 32, 40, 48, 56};
int jumpDelay1 = 4;
int leap1 = 2;

//--lvl.2------------------------------------------------------------------------//
int start2[] = {10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
int jumpDelay2 = 3;
int leap2 = 3;

//--lvl.3--------------------------currently impossible--------------------------//
int start3[] = {8, 16, 20, 28, 32, 40, 44, 52, 56};
int jumpDelay3 = 2;
int leap3 = 3;
//-------------------------------------------------------------------------------//

//tempo

Chrono theTime;
Chrono respawn;

//btm data
float theBPM = 100;
float theBPS = 60 / theBPM;
float timeBang = theBPS * 1000;

//control of animation step
boolean beatOffBeat = false;
boolean etat = false;

//Player data
int characterPosition = 0;
int currentLevel = 1;
boolean hasMoved = false;
boolean isDeath = false;
float respawnDelay = 1000;

//vibration detection
const int LEFT = 1;
const int RIGHT = 0;
const int Treshold = 220;

//button detection
const int button = 7;

//controller data
int leftState = 0;
int rightState = 0;
boolean bang;

void setup() {
  // put your setup code here, to run once:
  theTime.start();

  //Button pinMode
  pinMode(7, INPUT_PULLUP);

  //for debuging only
  //Serial.begin(9600);
  //while (!Serial) {};

  //Message pour setter le tempo du son avec le code
  msg.sendBegin("setup");
  msg.sendInt(600);
  msg.sendEnd();

  //start main timer
  theTime.restart();

  pixels.begin();
  //  pinMode(LEFT, INPUT);
  //  pinMode(RIGHT, INPUT);
}

void loop() {

  unsigned long elapsed = theTime.elapsed();
  bang = false;

  leftState = analogRead(LEFT);
  rightState = analogRead(RIGHT);

  //loop trought all the pixel and apply black color for the reset
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }

  //light the character in white
  pixels.setPixelColor(characterPosition, pixels.Color(150, 150, 150));

  //call the function that need to check data every iteration
  checkTimer();
  checkRespawn();
  checkVictory();
  checkRestartGame();

  //bang if vibration are detected
  if (leftState > Treshold || rightState > Treshold) {
    bang = true;
  }

  //move if bang
  if (bang == true) {
    movement();
  }
  //calcul ennemis position
  ennemisData();

  //death animation
  if (isDeath == true) {
    if (beatOffBeat == true) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(150, 150, 150));
        beatOffBeat = false;
      }
    } else if (beatOffBeat == false) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        beatOffBeat = true;
      }
    }
  }
  pixels.show();
}

void checkTimer() {
  //check the tempo to restrain character movement and call the tempo function
  if ( theTime.hasPassed(timeBang) ) {
    theTime.restart();
    hasMoved = false;
    isDeath = false;
    if (beatOffBeat == false) {
      tempo();
      beatOffBeat = true;
    } else if (beatOffBeat == true) {
      beatOffBeat = false;
    }
  }
}

void checkRestartGame() {
  if(digitalRead(button) == 1) {
    currentLevel = 1;
    theBPM = 100;
    isDeath = true;
    theTime.stop();
    respawn.start();
    characterPosition = 0;
    msg.sendBegin("setup");
    msg.sendEnd();
  }
}

void checkRespawn() {
  //check the death time(respawn) and stop it then start the normal timer
  if (respawn.hasPassed(respawnDelay) && isDeath == true) {
    Serial.println("endTimer");
    respawn.stop();
    theTime.start();
    isDeath = false;
  }
}

void checkVictory() {
  //check if player is at the last light(he win) and the next level appear after the death animation
  if (characterPosition >= 59) {
    //NUMPIXELS - 1
    if (currentLevel == 3) {
      currentLevel = 1;
      theBPM = 100;
      msg.sendBegin("niveauReussi3");
      msg.sendInt(600);
      msg.sendEnd();
    } else {
      currentLevel = currentLevel + 1;
      theBPM = theBPM + 20;
      msg.sendBegin("niveauReussi");
      msg.sendInt(currentLevel);
      msg.sendEnd();
    }
    resetNiveau();
  }
}

void tempo() {
  //switch ennemis stats
  if (etat == false) {
    etat = true;
  } else {
    etat = false;
  }
}

void movement() {
  //move player
  if (hasMoved == false) {
    hasMoved = true;
    if (leftState > Treshold) {
      //if left detector is higher then treshold move left
      if (characterPosition > 0) {
        characterPosition = characterPosition - 1;
      }
    } else if (rightState > Treshold) {
      //if right detector is higher then treshold move right
      if (characterPosition < 60) {
        characterPosition = characterPosition + 1;
      }
    }
  }
}

void ennemisData() {
  //determine the position of ennemis depending on current level
  int theValue;
  //level 1 mecanique
  if (currentLevel == 1) {
    if (etat == false) {
      for (int i = 0; i < (sizeof(start1) / sizeof(int)); i++) {
        lightEnnemis(start1[i]);
      }
    } else {
      for (int i = 0; i < (sizeof(start1) / sizeof(int)); i++) {
        theValue = start1[i] - leap1;

        //Serial.println(theValue);
        lightEnnemis(theValue);
      }
    }
  }
  //level 2 mecanique
  else if (currentLevel == 2) {
    if (etat == false) {
      for (int i = 0; i < (sizeof(start2) / sizeof(int)); i++) {
        lightEnnemis(start2[i]);
      }
    } else {
      for (int i = 0; i < (sizeof(start2) / sizeof(int)); i++) {
        theValue = start2[i] - leap2;
        lightEnnemis(theValue);
      }
    }
  }
  //level 3 mecanique
  else if (currentLevel == 3) {
    if (etat == false) {
      for (int i = 0; i < (sizeof(start3) / sizeof(int)); i++) {
        lightEnnemis(start3[i]);
      }
    } else {
      for (int i = 0; i < (sizeof(start3) / sizeof(int)); i++) {
        theValue = start3[i] - leap3;
        lightEnnemis(theValue);
      }
    }
  }
}

void lightEnnemis(int thePosition) {
  //light ennemi on thePosition with red light
  if (thePosition == characterPosition) {
    death();
  }
  if(currentLevel == 1){
      pixels.setPixelColor(thePosition, pixels.Color(80, 150, 40));
  }else if(currentLevel == 2){
      pixels.setPixelColor(thePosition, pixels.Color(150, 40, 40));
  }else if(currentLevel == 3){
      pixels.setPixelColor(thePosition, pixels.Color(150, 0, 0));
  }
  

}

void death() {
  //stop normal timer start respawn timer reset player position
  isDeath = true;
  theTime.stop();
  respawn.start();
  characterPosition = 0;
  msg.sendBegin("mort");
  msg.sendInt("1");
  msg.sendEnd();
}

void resetNiveau() {
  //stop normal timer start respawn timer reset player position
  isDeath = true;
  theTime.stop();
  respawn.start();
  characterPosition = 0;
}

