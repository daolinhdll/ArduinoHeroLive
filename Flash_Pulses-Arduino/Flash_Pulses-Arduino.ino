#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // or 0x3F

//encoder
static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
static int enSW = 4; //The select switch for our encoder.

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
int encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
int oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
//---------

int ledPin = 13; // set the signal pin out here.
int flashButton = 5; //
int buzzer = 6;
int pulseNum = 40; // set the Pulse number here.
int onTime = 5; // set the Pulse width here.
int offTime = 5;

#define STATE_STARTUP 0
#define STATE_MAINMENU 1
#define STATE_SETTING 2
#define STATE_SETPARA 3
#define STATE_WAITTORUN 4
#define STATE_FLASH 5

byte currentState = STATE_STARTUP;

//------
int mainMenuCnt = 1;
int settingMenuCnt = 1;
int numOfSetting = 2;


void setup()
{
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(flashButton, INPUT_PULLUP);
  pinMode(buzzer,OUTPUT);
  
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzer, LOW);
  

  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(enSW, INPUT_PULLUP);
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0); //frint from column 3, row 0
  lcd.print("Hero Live");
  lcd.setCursor(0, 1);
  lcd.print("Xin chao cac ban");
  delay(2000);
  lcd.clear();

  updateState(STATE_MAINMENU);
}

void loop()
{
  //  if (oldEncPos != encoderPos) {
  //    Serial.println(encoderPos);
  //    oldEncPos = encoderPos;
  //  }
}




// ---------------------------
void updateState(byte aState)
{
  if (aState == currentState)
  {
    return;
  }

  // do state change
  switch (aState)
  {
    case STATE_STARTUP:
      Serial.println("STATE_STARTUP");
      delay(1000);
      break;
    case STATE_MAINMENU:
      Serial.println("STATE_MAINMENU");
      mainMenu();
      break;
    case STATE_SETTING:
      setting();
      break;
    case STATE_SETPARA:
      subSettingMenu();
      break;
    case STATE_WAITTORUN:
      waitToRun();
      break;
    case STATE_FLASH:
      delay(100);
      Serial.println("STATE_FLASH");
      flash(pulseNum, onTime, offTime);
      break;
  }

  currentState = aState;
}
//--------------------------------
void mainMenu() {
  //display line 0
  int currentEncoderPos = encoderPos;
  updateMainMenu();
  while (true) {
    if (currentEncoderPos != encoderPos) {
      mainMenuCnt = mainMenuCnt + (encoderPos - currentEncoderPos);
      currentEncoderPos = encoderPos;
      updateMainMenu();
    }
    if (digitalRead(enSW) == 0) {
      while (digitalRead(enSW) == 0);
      if (mainMenuCnt == 1) {
        updateState(STATE_SETTING);
      } else if (mainMenuCnt == 2) {
        updateState(STATE_WAITTORUN);
      }
      break;
    }
  }

}
// ---------------------------
void setting() {
  Serial.println("STATE_SETTING");
  int currentEncoderPos = encoderPos;
  updateSettingMenu();
  while (true) {
    if (currentEncoderPos != encoderPos) {
      settingMenuCnt = settingMenuCnt + (encoderPos - currentEncoderPos);
      currentEncoderPos = encoderPos;
      updateSettingMenu();
    }
    if (digitalRead(enSW) == 0) {
      while (digitalRead(enSW) == 0);
      if (settingMenuCnt == 4) {
        updateState(STATE_MAINMENU);
      } else {
        updateState(STATE_SETPARA);
      }
      break;
    }
  }
}

void waitToRun() {
  long timewait = millis();
  Serial.println("STATE_WAITTORUN");
  lcd.clear();
  lcd.setCursor(5, 0); //frint from column 1, row 0
  lcd.print("FLASH");
  lcd.setCursor(1, 1);
  lcd.print("Push to run");
  while (true) {
    //    Serial.println(digitalRead(flashButton));
    if (digitalRead(flashButton) == 0) {
      while (digitalRead(flashButton) == 0);
      updateState(STATE_FLASH);
    }
    if (millis() - timewait > 10000) {
      updateState(STATE_MAINMENU);
      break;
    }
    if (digitalRead(enSW) == 0) {
      while (digitalRead(enSW) == 0);
      updateState(STATE_MAINMENU);
      break;
    }
  }
}

void subSettingMenu() {
  Serial.println("STATE_SETPARA");
  int currentEncoderPos = encoderPos;
  updateSubSettingMenu();
  while (true) {
    if (digitalRead(enSW) == 0) {
      while (digitalRead(enSW) == 0);
      updateState(STATE_SETTING);
      break;
    }
    if (currentEncoderPos != encoderPos) {
      switch (settingMenuCnt) {
        case 1:
          pulseNum = pulseNum + (encoderPos - currentEncoderPos);
          if (pulseNum < 0) {
            pulseNum = 0;
          }
          break;
        case 2:
          onTime = onTime + (encoderPos - currentEncoderPos);
          if (onTime < 0) {
            onTime = 0;
          }
          break;
        case 3:
          offTime = offTime + (encoderPos - currentEncoderPos);
          if (onTime < 0) {
            offTime = 0;
          }
          break;
      }
      currentEncoderPos = encoderPos;
      updateSubSettingMenu();
    }
  }

}

void flash(int repeats, int onTime, int offTime)
{
  lcd.clear();
  lcd.setCursor(4, 0); //frint from column 1, row 0
  lcd.print("FLASHING");
  lcd.setCursor(0, 1);
  lcd.print("..._|_|_|_|_|_|_");
  digitalWrite(buzzer,HIGH);
  for (int i = 0; i < repeats; i++)
  {
    digitalWrite(ledPin, LOW);
    delay(offTime);
    digitalWrite(ledPin, HIGH);
    delay(onTime);
  } 
  lcd.setCursor(0, 1);
  lcd.print("................");
  delay(2000);
  digitalWrite(buzzer,LOW);
  updateState(STATE_WAITTORUN);
}

//--------------------
void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge

    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}
//-------------------
void updateMainMenu() {
  switch (mainMenuCnt) {
    case 0:
      mainMenuCnt = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print(">Setting");
      lcd.setCursor(1, 1);
      lcd.print("Flash");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(1, 0); //frint from column 0, row 0
      lcd.print("Setting");
      lcd.setCursor(0, 1);
      lcd.print(">Flash");
      break;
    case 3:
      mainMenuCnt = 2;
      break;
  }
}
//-------------------
void updateSettingMenu() {
  switch (settingMenuCnt) {
    case 0:
      settingMenuCnt = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print(">PulseNum");
      lcd.setCursor(11, 0);
      lcd.print(pulseNum);
      lcd.setCursor(1, 1);
      lcd.print("ON Time");
      lcd.setCursor(11, 1);
      lcd.print(onTime);
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(1, 0); //frint from column 1, row 0
      lcd.print("PulseNum");
      lcd.setCursor(11, 0);
      lcd.print(pulseNum);
      lcd.setCursor(0, 1);
      lcd.print(">ON Time");
      lcd.setCursor(11, 1);
      lcd.print(onTime);
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print(">OFF Time");
      lcd.setCursor(11, 0);
      lcd.print(offTime);
      lcd.setCursor(1, 1); //frint from column 1, row 0
      lcd.print("Main menu");
      break;
    case 4:
      lcd.clear();
      lcd.setCursor(1, 0); //frint from column 1, row 0
      lcd.print("OFF Time");
      lcd.setCursor(11, 0);
      lcd.print(offTime);
      lcd.setCursor(0, 1); //frint from column 1, row 0
      lcd.print(">Main menu");
      break;
    case 5:
      settingMenuCnt = 4;
      break;
  }
}
//-------------------
void updateSubSettingMenu() {
  switch (settingMenuCnt) {
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print("Num of pulses");
      lcd.setCursor(0, 1);
      lcd.print(pulseNum);
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print("ON Time");
      lcd.setCursor(0, 1);
      lcd.print(onTime);
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0, 0); //frint from column 1, row 0
      lcd.print("OFF Time");
      lcd.setCursor(0, 1);
      lcd.print(offTime);
      break;
  }
}
//-------------------
