/**
  ******************************************************************************
  * @file    vibeSensor/vibeSensor.c 
  * @author  John Webster | 07/09/18
  * @brief   Firmware for an electronic vibration sensor for longboards, 
  *          meant to give comparative readings when used with different setups.
  ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <MPU6050_tockn.h>
#include <LedControl.h>
#include <Wire.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LCD_DELAY    50   //250ms delay at the end of each loop (for lcd display)
#define HEARTBEAT_DELAY 1000  //2s period heartbeat
/* Private constants ------------------------------------------------------------*/
const int buttonPin = 2;    // the number of the footswitch pin
const int ledPin = 7;      // general purpose LED pin
const int heartbeat = 13;   //use D13 LED as heartbeat for debugging
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

uint32_t mainTimer = 0;
uint32_t mainElapsed = 0;
/*
 * stage 0= after initialization finishes the first time
 * stage 1=set after start button push, waits for board to be levelled
 * stage 2=set after board is levelled, runs DAQ loop
 * stage 3=set after DAQ finishes, displays results and resets data
 */
uint8_t stage=0;
MPU6050 mpu6050(Wire);        //vibration sensor
/*
 * Pin12 to DataIn
 * Pin11 to CLK
 * Pin10 to LOAD
 * 2, number of MAX72x devices
 */
LedControl lc=LedControl(12,11,10,2); 


/* Private function prototypes -----------------------------------------------*/
void hello();
void msgDisplay();
bool checkForButtonPush();
bool boardLevelled();
void displayGO();

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Setup peripherals
  * @param  None
  * @retval None
  */
void setup() {
  
/*  TODO:
 *  LED display
 *  accelerometer
 */
  Serial.begin(9600);
  Wire.begin();

  mpu6050.begin();

  lc.shutdown(0,false);     //wakeup call
  lc.setIntensity(0,8);     //lvl 0-15
  lc.clearDisplay(0);
  
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(heartbeat, OUTPUT);

  
  // set initial LED state
  digitalWrite(ledPin, ledState);

  hello();  
  stage[0] = 1; //go!
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void loop() {

  mainElapsed = HAL_GetTick();

  if (mainElapsed - mainTimer > HEARTBEAT_DELAY){
    if (digitalRead(heartbeat) == HIGH){
      digitalWrite(heartbeat,LOW);
    } else {
      digitalWrite(heartbeat, HIGH);
    }

    mainTimer = mainElapsed;
  }
/*
  if (startup){
    //msgDisplay("New run");

    //clear data
    startup = false;
  }
*/

  switch (stage){

    case 0:
      if (checkForButtonPush()){
        stage++;
      }
      break;
    case 1:
      if (boardLevelled()){
        stage++;
        displayGO();
        startTime = millis(); //start timer
      }
      break;
    case 2:

      /*
       * collect and store data here
       */
       
      if (millis() - startTime) > RUN_TIME){
        stage++;

        //make sure to store end of data here

        /*
         * data processing...
         */     
      }
      
      break;
    case 3:
      /*
       * display data for user
       */
      stage=0;
      break;
    default:
      //msgDisplay("ERROR");
      digitalWrite(ledPin, HIGH);
      digitalWrite(heartbeat, HIGH);
      break;
  }
  
  delay(LCD_DELAY);
}//end: loop()

/**
  * @brief  Display "hello" for 2 seconds
  * @param  None
  * @retval None
  */
void hello(){

  for (int i=0; i<5; i++){
    switch(i){
      case 0: //h
        lc.setChar(0,3,'h',0); //may need to be upper case H or use setRow();
        break;
      case 1: //E
        lc.setChar(0,2,0xE,0); //either setDig or setChar, might need 'E' 
        break;
      case 2: //1
        lc.setDigit(0,1,0x1,0);
        break;
      case 3: //1
        lc.setDigit(0,0,0x1,0);
        break;
      case 4: //o
        lc.setRow(1,3,0x1D);
        break;     
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }

  delay(2000);
}//end:hello()

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

}//end checkForButtonPush()

/**
  * @brief  Set up sensors for DAQ
  * @param  None
  * @retval None
  */
bool boardLevelled(){
  int flag=0;
  /*
   * read gyro and return true if < 15deg from level
   */
}

/**
  * @brief  Blink "GO" for 1 second
  * @param  None
  * @retval None
  */
void displayGO(){

//note: it's ok to block on this function, to give the rider a reaction time to start pushing the board and get moving
  const int GO_TIME = 2500; //how long to blink GO for
  int elapsed = 0;
  long int startGo = millis();

  for (int i=0; i<4; i++){
    switch(i){
      //art.
      case 0: //
        lc.setRow(0,3, B01001110); 
        break;
      case 1: //
        lc.setRow(0,2,B00101101); 
        break;
      case 2: //
        lc.setRow(0,1,B00110000);
        break;
      case 3: //
        lc.setRow(1,3,B01111110);
        break;
      case 4:
        lc.setRow(1,3,B00000110);
        break;
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
  
  while (millis() - startGo < GO_TIME){
    if (millis() - elapsed > 200){
      elapsed = millis();
      signState = !signState;
      lc.shutdown(0,signState);  //blink LCD on or off
      delay(LCD_DELAY);
    }
  }
}//end:displayGO()

