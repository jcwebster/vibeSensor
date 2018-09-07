/**
  ******************************************************************************
  * @file    vibeSensor/vibeSensor.c 
  * @author  John Webster | 07/09/18
  * @brief   this is the firmware for an electronic vibration sensor for longboards, 
  *          meant to give comparative readings when used with different setups.
  ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "MPU6050.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ------------------------------------------------------------*/
const int buttonPin = 2;    // the number of the footswitch pin
const int ledPin = 13;      // general purpose LED pin
const int accelPin = 11;    // the number of the accelerometer pin
const int accelAddress = 0x68; // may be pull-down resistor at AD0 (address = 0x68), others have a pull-up resistor (address = 0x69).

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
bool startup = true;         // set to true only on reset or new cycle
bool startRun = false;       // flag to signal start of DAQ
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

/* Private function prototypes -----------------------------------------------*/
void msgDisplay();
int checkForButtonPush();
void initializeRun();
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  setup peripherals
  * @param  None
  * @retval None
  */
void setup() {
  
/*  TODO:
 *  LED display
 *  accelerometer
 */

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin, ledState);

  //msgDisplay("Welcome");

}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void loop() {
  // put your main code here, to run repeatedly:

  if (startup){
    //msgDisplay("New run");

    //clear data
    startup = false;
  }

  startRun = checkForButtonPush();

  if (stage[0]){
    stage[0] = 0;

    //go DAQ!
    initializeRun(); //wait for board to be level

    //startTime = millis(); //set up timer

    stage[1] = 1;
  } else if (stage[1]){   
    stage[1] = 0;
   
    //DAQ

    //

    //is timer done?
    if (timeUp){
      stage[2] = 1;
    }
  } else if (stage[2]){
    stage[2] = 0;

   //Display results

   //

   
    //do a quick runtime analysis of this controller structure tmrw, 
    //i think it could be more efficient...
    //it does, however, allow for expansion for say, a bluetooth data download stage
    stage[0] = 1;
  } else {
    
    //msgDisplay("ERROR");
    digitalWrite(ledPin, HIGH);
    return;
  }
  
  
}

/**
  * @brief  Check if button has been pushed. 
  *         Adopted from D.A. Mellis Arduino Debounce sketch.
  * @param  None
  * @retval 1 if pushed
  */
int checkForButtonPush(){

  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

}

