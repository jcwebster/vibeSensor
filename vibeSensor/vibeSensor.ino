/**
  ******************************************************************************
  * @file    vibeSensor/vibeSensor.c 
  * @author  John Webster | 07/09/18
  * @brief   Firmware for an electronic vibration sensor for longboards, 
  *          meant to give comparative readings when used with different wheel/truck setups.
  ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include <MPU6050_tockn.h>
#include <LedControl.h> 
#include <Wire.h>
//#include <SevenSeg.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEBUG         1     //comment out to eliminate print statements and other testing
#define LCD_DELAY    50   //250ms delay at the end of each loop (for lcd display)
#define HEARTBEAT_DELAY 1000  //2s period heartbeat
#define RUN_TIME      (10000)
#define NUM_PTS       (700)
/* Private constants ------------------------------------------------------------*/
//const int buttonPin = 2;    // the number of the footswitch pin
//int buttonPushed;             // the current reading from the input pin

//const uint32_t RUN_TIME=10000;
const byte buttonPin = 2; //now configured as interrupt pin
volatile byte buttonPushed = LOW;

const int ledPin = 7;      // general purpose LED pin
const int heartbeat = 13;   //use D13 LED as heartbeat for debugging
//const int accelPin = 11;    // the number of the accelerometer pin
const int accelAddress = 0x68; // may be pull-down resistor at AD0 (address = 0x68), others have a pull-up resistor (address = 0x69).

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
bool startup = true;         // set to true only on reset or new cycle
bool startRun = false;       // flag to signal start of DAQ
int ledState = HIGH;         // the current state of the output pin
int lastButtonState = LOW;   // the previous reading from the input pin
bool signState;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

uint32_t mainTimer = 0;
uint32_t mainElapsed = 0;
uint32_t startTime = 0;
uint32_t daqElapsed = 0;
int progressFraction =0;

uint32_t daqPt = 0;
uint8_t data[NUM_PTS] = {};
uint32_t sum = 0;
uint8_t vibeMagnitude = 0;

/*
 * stage =0; after initialization finishes the first time
 * stage =1; set after start button push, waits for board to be levelled
 * stage =2; set after board is levelled, runs DAQ loop
 * stage =3; set after DAQ finishes, displays results and resets data
 */
uint8_t stage=0;

/*
 * Pin12 to DataIn
 * Pin11 to CLK
 * Pin10 to LOAD (~CS)
 * 2, number of MAX72x devices
 */
MPU6050 mpu6050(Wire);        //vibration sensor

/*
 * Pin12 to DataIn
 * Pin11 to CLK
 * Pin10 to LOAD (~CS)
 * 2, number of MAX72x devices
 */
LedControl lc=LedControl(12,11,10,1); 


/* Private function prototypes -----------------------------------------------*/
void hello();
void freezeAndCalibrate();
void msgDisplay();
//bool checkForButtonPush();
bool boardLevelled();
void displayGO();
void displayPush2Start();
void results(uint8_t);

#ifdef DEBUG
/*    testing only    */
const int fakeInterruptPin = 6;
#endif
/**
  * @brief  Setup peripherals
  * @param  None
  * @retval None
  */
void setup() {
#ifdef DEBUG
 /*    testing only    */
  pinMode(fakeInterruptPin, OUTPUT);
#endif

  Serial.begin(9600);
  Wire.begin();

  lc.shutdown(0,false);     //wakeup call
  lc.setIntensity(0,0);     //0,lvl 0-15
  lc.clearDisplay(0);
  
//  pinMode(buttonPin, INPUT); //now interrupt pin
  pinMode(ledPin, OUTPUT);
  pinMode(heartbeat, OUTPUT); //heartbeat onboard pin 13
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), checkForButtonPush, RISING); //'blink' is the ISR
 
  mpu6050.begin();
 
  // set initial LED state
  digitalWrite(ledPin, ledState);

  hello();  
  
//  displayGo();
  displayPush2Start();
  delay(1000);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void loop() {

  mainElapsed = millis();

  if (mainElapsed - mainTimer > HEARTBEAT_DELAY){
    if (digitalRead(heartbeat) == HIGH){
      digitalWrite(heartbeat,LOW);
    } else {
      digitalWrite(heartbeat, HIGH);
    }

    mainTimer = mainElapsed;
  }

  switch (stage){
    case 0: //wait for button push to start
#ifdef DEBUG
      Serial.println("case0");
#endif
      if (buttonPushed){
        freezeAndCalibrate(); //board must be held still for gyro calibration
        delay(1000);
        mpu6050.update(); //updates all data
        stage++;
        buttonPushed=false;
#ifdef DEBUG
        digitalWrite(fakeInterruptPin, LOW); //test response to button push
#endif
      }
      displayPush2Start();
      delay(2000);
#ifdef DEBUG
      if (mainTimer> 1500){
        digitalWrite(fakeInterruptPin, HIGH); //test response to button push
        Serial.println("button pushed");
      }
#endif
      break;
    case 1: //wait for board to be levelled
#ifdef DEBUG
      Serial.println("case1");

      lc.clearDisplay(0);

      mpu6050.update(); //updates all data 
      Serial.print("angleX : ");
      Serial.print(mpu6050.getAngleX());
      Serial.print("\tangleY : ");
      Serial.print(mpu6050.getAngleY());
      Serial.print("\tangleZ : ");
      Serial.println(mpu6050.getAngleZ());
      delay(500);
#endif

      if (boardLevelled()){
 
        displayGetReady();
        delay(2500);
        lc.setDigit(0,6,0x1,0);
        delay(1500);
        lc.setDigit(0,6,0x0,0);
        delay(1500);
        displayGO();
        lc.clearDisplay(0);

        startTime = millis(); //start timer
        stage++;
      }
      break;
    case 2:
    
#ifdef DEBUG
      Serial.println("case2");
#endif
      /*
       * collect and store data here
       */
      daqElapsed = millis() - startTime;
      mpu6050.update();
      Serial.print("\taccZ : ");Serial.println(mpu6050.getAccZ());
      //store mag of acc-z in array for i-th iteration;

      data[daqPt] = mpu6050.getAccZ();
      daqPt++;
      //when done, calculate RMS vibe
      
      if (daqElapsed > RUN_TIME){
        lc.setRow(0,0,B00001000);
        delay(LCD_DELAY);
        stage++;
        Serial.println("Done DAQ!");
        //make sure to store end of data here
        /*
         * data processing...
         */     
      } 

      if ((daqElapsed*8) > (RUN_TIME*progressFraction)){
        for (int i=0; i < progressFraction; i++){
          lc.setRow(0,7-i, B00001000);
          delay(LCD_DELAY);
        }
        progressFraction++;
      }
      
      break;
    case 3:
#ifdef DEBUG
      Serial.println("case3");
      
#endif
      /*
       * display data for user
       */
       //when done, calculate RMS vibe      
      for (int k=0; k < (daqPt); k++){
        
        data[k] = data[k]/1.4142135624;
        sum += data[k];
      }
      
      /* 
       *by taking the RMS average of all data points, 
       *I am calculating the RMS value of accleration vertically, 
       *in g's (vibeMagnitude*9.8 gives m/s2)    
       *  THIS IS WHAT I WANT OUTPUT  
       */
      vibeMagnitude = sum / (NUM_PTS); 
      daqPt = 0;

      lc.clearDisplay(0);
      results(vibeMagnitude);
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
        lc.setChar(0,4,'h',0); 
        break;
      case 1: //E
        lc.setChar(0,3,0xE,0); 
        break;
      case 2: //1
        lc.setDigit(0,2,0x1,0);
        break;
      case 3: //1
        lc.setDigit(0,1,0x1,0);
        break;
      case 4: //o
        lc.setRow(0,0,0x1D);
        break; 
      default:
        break;    
    }

    delay(LCD_DELAY); //delete or extend as necessary
  }

  delay(1000);
}//end:hello()

/**
  * @brief  Display "freeze" while calibrating gyro
  * @param  None
  * @retval None
  */
void freezeAndCalibrate(){
  lc.clearDisplay(0);

  for (int i=0; i<10; i++){
    switch(i){
      case 0: //L
        lc.setChar(0,7,'L',0); 
        break;
      case 1: //E
        lc.setChar(0,6,0xE,0); 
        break;
      case 2: //v
        lc.setRow(0,5,B00011100);
        break;
      case 3: //E
        lc.setChar(0,4,0xE,0);
        break;
      case 4: //L
        lc.setChar(0,3,'L',0); 
        break;  
      case 5: //...
        lc.setRow(0,3,B10000000|B00001110);
        lc.setRow(0,2,B10000000);
        lc.setRow(0,1,B10000000);
        break;   
      case 6: //3
        lc.setChar(0,0,'3',0); 
        delay(1500);
        break;
      case 7: //2
        lc.setChar(0,0,'2',0); 
        delay(1500);
        break;
      case 8: //1
        lc.setChar(0,0,'1',0); 
        mpu6050.calcGyroOffsets(true); 
//        mpu6050.setGyroOffsets(3.2075, 0.6875, 0.45); //not consistent enough
        break;
      case 9: //0
        lc.setChar(0,0,'0',0);
        break;
      default:
        break;  
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
}//end:freezeAndCalibrate()

/**
  * @brief  Display start instruction
  * @param  None
  * @retval None
  */
void displayPush2Start(){
  for (int i=0; i<12; i++){
    switch(i){
      case 0: //P
        lc.clearDisplay(0);
        lc.setChar(0,7,'P',0);
        break;
      case 1: //U
        lc.setRow(0,6,B00111110);
        break;
      case 2: //S
        lc.setChar(0,5,'5',0); 
        break;
      case 3: //H
        lc.setRow(0,4,B00110111);
        break;
      case 4: //[
        lc.setRow(0,1,B01001110);
        break;
      case 5: //]
        lc.setRow(0,0,B01111000);
        delay(2000);
        break;
      case 6: //2
        lc.clearDisplay(0);
        lc.setChar(0,6,'2',0); 
        break;
      case 7: //b
        lc.setChar(0,4,'b',0); 
        break;
      case 8: //e
        lc.setChar(0,3,0xE,0); 
        break;
      case 9: //g
        lc.setRow(0,2,B01111011);
        break;
      case 10: //i
        lc.setRow(0,1,B00010000);
        break;
      case 11: //n
        lc.setChar(0,0,'n',0);
        break;
      default:
        break;  
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
}//end:displayPush2Start()

/**
  * @brief  ISR for button push. Check if ISR is triggered outside of debounceDelay 
  *         and if so, change button state to PUSHED.
  * @param  None
  * @retval None
  */
void checkForButtonPush(){
  if ((millis() - lastDebounceTime) > debounceDelay) { //millis() may not work here, "will never increment inside an ISR."
    buttonPushed = !buttonPushed;
  }

  #ifdef DEBUG
  Serial.print("i");
  #endif
  
  lastDebounceTime=millis(); //this will cause the state to be switched on the first triggering of the interrupt, and not on the subsequent bounces

  // read the state of the switch into a local variable:
//  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
//  if (reading != lastButtonState) {
    // reset the debouncing timer
//    lastDebounceTime = millis();
//  }

//  if ((millis() - lastDebounceTime) > debounceDelay) { //millis() may not work here, "will never increment inside an ISR."
//    // whatever the reading is at, it's been there for longer than the debounce
//    // delay, so take it as the actual current state:
//    state = !state;

    // if the button state has changed:
//    if (reading != buttonState) {
//      buttonState = reading;
//
//      // only toggle the LED if the new button state is HIGH
//      if (buttonState == HIGH) {
//        ledState = !ledState;
//      }
//    }
//  }
    // save the reading. Next time through the loop, it'll be the lastButtonState:
//  lastButtonState = reading;

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
   mpu6050.update(); //updates all data 

   if (mpu6050.getAngleX() < 20 && mpu6050.getAngleX() > -20 
        && mpu6050.getAngleY() < 20 && mpu6050.getAngleX() > -20){

     Serial.print("board levelled! x: ");
     Serial.print(mpu6050.getAngleX());  
     Serial.print(" y: ");
     Serial.println(mpu6050.getAngleY()); 
     return true;
   }
   else {
    return false;
   }
}

/**
  * @brief  Give "get ready" msg
  * @param  None
  * @retval //PREPARE
            //2 PUSH
  */
void displayGetReady(){
  
  for (int i=0; i<12; i++){
    switch(i){
      case 0: //P
        lc.clearDisplay(0);
        lc.setChar(0,7,'P',0);
        break;
      case 1: //r
        lc.setRow(0,6,B00000101);
        break;
      case 2: //E
        lc.setChar(0,5,'E',0); 
        break;
      case 3: //P
        lc.setChar(0,4,'P',0); 
        break;
      case 4: //A
        lc.setChar(0,3,'A',0); 
        break;
      case 5: //R
        lc.setRow(0,2,B00000101);
        break;
      case 6: //E
        lc.setChar(0,1,'E',0); 
        delay(1500);
        break;
      case 7: //2
        lc.clearDisplay(0);
        lc.setDigit(0,6,0x2,0);
        break;
      case 8: //P
        lc.setChar(0,3,'P',0);
        break;
      case 9: //U
        lc.setRow(0,2,B00111110);
        break;
      case 10: //S
        lc.setChar(0,1,'5',0); 
        break;
      case 11: //H
        lc.setRow(0,0,B00110111);
        break;
      default:
        break;  
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
}//end:displayGetReady()
  
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

  for (int i=0; i<5; i++){
    switch(i){
      //art.
      case 0: //G
        lc.clearDisplay(0);
        lc.setRow(0,5, B01001110); 
        break;
      case 1: //G
        lc.setRow(0,4,B01011001); 
        break;
      case 2: //
        lc.setRow(0,3,B00110000);
        break;
      case 3: //
        lc.setRow(0,2,B01111110);
        break;
      case 4:
        lc.setRow(0,1,B00000110);
        lc.setRow(0,0,B10110000);
        break;
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
  
  while (millis() - startGo < GO_TIME){
    if (millis() - elapsed > 400){
      elapsed = millis();
      signState = !signState;
      lc.shutdown(0,signState);  //blink LCD on or off
      delay(LCD_DELAY);
    }
  }
  lc.shutdown(0,false);  //blink LCD on or off

}//end:displayGO()


/**
  * @brief  Display results post-DAQ
  * @param  None
  * @retval None
  */
void results(uint8_t vibe){
  
//  const int GO_TIME = 2500; //how long to blink GO for
//  int elapsed = 0;
//  long int startGo = millis();

  for (int i=0; i<6; i++){ //FINISH
    switch(i){
      //art.
      case 0: //F
        lc.clearDisplay(0);
        lc.setRow(0,5, B01000111); 
        break;
      case 1: //I
        lc.setRow(0,4,B00110000); 
        break;
      case 2: //N
        lc.setRow(0,3,B00010101);
        break;
      case 3: //I
        lc.setRow(0,2,B00110000); 
        break;
      case 4: //S
        lc.setRow(0,1,B01011011);
        break;
      case 5: //H
        lc.setRow(0,0,B00110111);
        break;
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }
  
  delay(2500);
  for (int i=0; i<6; i++){ //rEcord
    switch(i){
      case 0: //r
        lc.clearDisplay(0);
        lc.setRow(0,6, B00000101); 
        break;
      case 1: //e
        lc.setRow(0,5,B01001111); 
        break;
      case 2: //c
        lc.setRow(0,4,B00001101);
        break;
      case 3: //o
        lc.setRow(0,3,B00011101); 
        break;
      case 4: //r
        lc.setRow(0,2, B00000101); 
        break;
      case 5: //d
        lc.setRow(0,1,B10111101);
        lc.setRow(0,0,B10000000);
        break;
    }
    
    delay(LCD_DELAY); //delete or extend as necessary
  }

  /*
   * HERE: Need to define an enum type where enum number { one='0x1', two='0x2', three..}
   * where vibeMagnitude can be turned into a value readable by lc.setDigit() so that the 
   * vibeMagnitude can be displayed
   */
}
