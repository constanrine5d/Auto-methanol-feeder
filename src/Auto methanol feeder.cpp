#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h> //Include the Arduino stepper library https://github.com/SimpleElectronicsYT/Stepper-Examples

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
const int stepsPerRevolution = 2048; //Set how many steps it takes to make a full revolution. Divide the degrees per step by 360 to get the steps

// pushbutton digital input pin https://docs.arduino.cc/built-in-examples/digital/Button/

const int8_t buttonInjectPin = 2;
const int8_t buttonRefillPin = 3;

const int8_t buttonUpPin = 4;
const int8_t buttonDownPin = 5;
const int8_t buttonLeftPin = 6;
const int8_t buttonRightPin = 7;
const int8_t buttonOKPin = 12;

// Button states

int8_t buttonInjectState = LOW;                 // variable for reading the pushbutton status
int8_t buttonRefillState = LOW;

int8_t buttonUpState = LOW;
int8_t buttonDownState = LOW;
int8_t buttonLeftState = LOW;
int8_t buttonRightState = LOW;
int8_t buttonOKState = LOW;

// Button bounce time

unsigned long buttonBounceTime = 500;       //milliseconds
unsigned long buttonBounceTimeUpDown = 100; //milliseconds
unsigned long buttonBounceTimeOK = 1000;    //milliseconds
bool buttonStopPress = false;
bool buttonStopPressUpDown = false;
bool buttonStopPressOK = false;


// Setup stepper

Stepper stepperName = Stepper(stepsPerRevolution, 8, 10, 9, 11); //Use pin 8-11 to IN1-IN4. Correct order: 8, 10, 9, 11.


//LCD Menu Logic https://github.com/VRomanov89/EEEnthusiast/blob/master/03.%20Arduino%20Tutorials/02.%20LCDMenuTutorial/LCDMenuProgramming/LCDMenuProgramming.ino
const int numOfScreens = 10;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"Syringe pump by", "Con"}, {"uL/injection", "uL"}, {"Calibrate w/2048", "steps. Press OK"}, {"How manySteps/mL", " steps/mL"}, {"Start w/ inject", " "}, {"TimePerInjection", "Hours"}, {"# of Injections", "Injections"}, {"Please charge an", "d bleed syringe now"}, {"Recap:", " "}, {"Running...", " "}};

//Menu options' variables

double volumePerInjection = 2500;
int stepsPermL = 771;
bool startWithInjection = false;
int timePerInjection = 24; //hours
int numberOfInjections = 3;
int injectionsCompleted = 0;

bool programStarted = false;

bool updateScreen = false;

unsigned long previousTime = 0;
unsigned long previousButtonTime = 0;
unsigned long previousButtonUpDownTime = 0;
unsigned long previousButtonOKTime = 0;

unsigned long eventInterval = 24 * 3600000; //24 hours to milliseconds
////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {

  Serial.begin(9600);

  stepperName.setSpeed(5); //Set the RPM of the stepper motor
  pinMode(buttonInjectPin, INPUT);
  pinMode(buttonRefillPin, INPUT);

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(buttonLeftPin, INPUT);
  pinMode(buttonRightPin, INPUT);
  pinMode(buttonOKPin, INPUT);

  /*lcd.init();                      // initialize the lcd
    lcd.backlight();
    lcd.setCursor(2,0);
    lcd.print("Hello, world!");      // Print a message to the LCD.
    lcd.setCursor(0,1);
    lcd.print("Ywrobot Arduino!"); */

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(6, 1);
  lcd.print(screens[currentScreen][1]);

}

////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  buttonInjectState = digitalRead(buttonInjectPin); // read the state of the pushbutton value:
  buttonRefillState = digitalRead(buttonRefillPin);

  buttonUpState = digitalRead(buttonUpPin);
  buttonDownState = digitalRead(buttonDownPin);
  buttonLeftState = digitalRead(buttonLeftPin);
  buttonRightState = digitalRead(buttonRightPin);
  buttonOKState = digitalRead(buttonOKPin);

  if (buttonInjectState == HIGH) { // check if the pushbutton is pressed. If it is, the buttonState is HIGH.
    stepperName.step(-stepsPerRevolution / 64); //Syringe injects methanol
  } else if (buttonRefillState == HIGH) {
    stepperName.step(stepsPerRevolution / 64); //Syringe refills
  } else if (programStarted == false) {

    if (buttonLeftState == HIGH && currentScreen >= 1 && buttonStopPress == false) {
      currentScreen = currentScreen - 1;
      updateScreen = true;
      previousButtonTime = millis();
      buttonStopPress = true;
    }

    if (buttonRightState == HIGH && currentScreen < numOfScreens - 2 && buttonStopPress == false) {
      currentScreen = currentScreen + 1;
      updateScreen = true;
      previousButtonTime = millis();
      buttonStopPress = true;
    }

    switch (currentScreen) {
      case 0:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(6, 1);
        lcd.print(screens[currentScreen][1]);
        break;
      case 1:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(9, 1);
        lcd.print(screens[currentScreen][1]);
        lcd.setCursor(0, 1);
        lcd.print("       ");
        lcd.setCursor(0, 1);
        lcd.print(volumePerInjection);

        if (buttonUpState == HIGH && volumePerInjection <= 10000 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          volumePerInjection = volumePerInjection + 10;

          previousButtonUpDownTime = millis();
        }

        if (buttonDownState == HIGH && volumePerInjection >= 10 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          volumePerInjection = volumePerInjection - 10;

          previousButtonUpDownTime = millis();
        }

        break;
      case 2:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0, 1);
        lcd.print(screens[currentScreen][1]);

        if (buttonOKState == HIGH && buttonStopPressOK == false) {
          buttonStopPressOK = true;

          //volumePerInjection = volumePerInjection + 10;
          stepperName.step(-2048); //Syringe injects methanol

          previousButtonOKTime = millis();
        }
        break;
      case 3:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(3, 1);
        lcd.print(screens[currentScreen][1]);
        lcd.setCursor(13, 1);
        lcd.print("OK?");
        lcd.setCursor(0, 1);
        lcd.print("    ");
        lcd.setCursor(0, 1);
        lcd.print(stepsPermL);

        if (buttonUpState == HIGH && stepsPermL <= 4 * 2048 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          stepsPermL = stepsPermL + 1;

          previousButtonUpDownTime = millis();
        }

        if (buttonDownState == HIGH && stepsPermL >= 1 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          stepsPermL = stepsPermL - 1;

          previousButtonUpDownTime = millis();
        }

        if (buttonOKState == HIGH && buttonStopPressOK == false) {
          buttonStopPressOK = true;

          //volumePerInjection = volumePerInjection + 10;
          stepperName.step(-stepsPermL); //Syringe injects methanol

          previousButtonOKTime = millis();
        }
        break;
      case 4:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0, 1);
        lcd.print("   ");
        if (startWithInjection == false) {
          lcd.setCursor(0, 1);
          lcd.print("No");
        } else if (startWithInjection == true) {
          lcd.setCursor(0, 1);
          lcd.print("Yes");
        }

        if (buttonUpState == HIGH && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          startWithInjection = !startWithInjection;

          previousButtonUpDownTime = millis();
        }

        if (buttonDownState == HIGH && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          startWithInjection = !startWithInjection;

          previousButtonUpDownTime = millis();
        }

        break;
      case 5:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(4, 1);
        lcd.print(screens[currentScreen][1]);
        lcd.setCursor(0, 1);
        lcd.print("   ");
        lcd.setCursor(0, 1);
        lcd.print(timePerInjection);

        if (buttonUpState == HIGH && timePerInjection <= (7 * 24) && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          timePerInjection = timePerInjection + 1;

          previousButtonUpDownTime = millis();
        }

        if (buttonDownState == HIGH && timePerInjection >= 2 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          timePerInjection = timePerInjection - 1;

          previousButtonUpDownTime = millis();
        }

        eventInterval = timePerInjection * 3600000;
        break;
      case 6:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(4, 1);
        lcd.print(screens[currentScreen][1]);
        lcd.setCursor(0, 1);
        lcd.print("   ");
        lcd.setCursor(0, 1);
        lcd.print(numberOfInjections);

        if (buttonUpState == HIGH && numberOfInjections <= 24 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          numberOfInjections = numberOfInjections + 1;

          previousButtonUpDownTime = millis();
        }

        if (buttonDownState == HIGH && numberOfInjections >= 2 && buttonStopPressUpDown == false) {
          buttonStopPressUpDown = true;

          numberOfInjections = numberOfInjections - 1;

          previousButtonUpDownTime = millis();
        }
        break;
      case 7:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0, 1);
        lcd.print(screens[currentScreen][1]);
        break;
      case 8:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        lcd.setCursor(0, 0);
        lcd.print(screens[currentScreen][0]);
        lcd.print(volumePerInjection);
        lcd.print("uL");
        lcd.print(numberOfInjections);
        lcd.print("inj");
        lcd.setCursor(0, 1);
        lcd.print(stepsPermL);
        lcd.print("stp");
        if (startWithInjection == true) {
          lcd.print("Start w/inj");
        } else if (startWithInjection == false) {
          lcd.print("Start w/out inj");
        }
        lcd.print(screens[currentScreen][1]);


        if (buttonOKState == HIGH && buttonStopPressOK == false) {
          buttonStopPressOK = true;

          programStarted = true;

          previousTime = millis();

          previousButtonOKTime = millis();
        }

      default:
        if (updateScreen == true) {
          updateScreen = false;
          lcd.clear();
        }
        break;
    }

    if (buttonStopPress == true && millis() - previousButtonTime >= buttonBounceTime) {
      buttonStopPress = false;
    }

    if (buttonStopPressUpDown == true && millis() - previousButtonUpDownTime >= buttonBounceTimeUpDown) {
      buttonStopPressUpDown = false;
    }

    if (buttonStopPressOK == true && millis() - previousButtonOKTime >= buttonBounceTimeOK) {
      buttonStopPressOK = false;
    }

  } else if (programStarted == true) {

    lcd.noBacklight();

    Serial.println(millis());
    Serial.println(previousTime);
    Serial.println(eventInterval);
    Serial.println(millis() - previousTime >= eventInterval);
    Serial.println("/n");

    if (startWithInjection == true) {
      stepperName.step(floor(-stepsPermL * volumePerInjection / 1000));

      startWithInjection = false;
      previousTime = millis();

      injectionsCompleted = injectionsCompleted + 1;
    }

    if (millis() - previousTime >= eventInterval && injectionsCompleted < numberOfInjections) {
      stepperName.step(floor(-stepsPermL * volumePerInjection / 1000));

      previousTime = millis();

      injectionsCompleted = injectionsCompleted + 1;
    }
  }
}
