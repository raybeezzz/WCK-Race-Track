//color definitions
#define LED_WHITE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)
#define LED_GREEN_HIGH    (63 << 5)
#define LED_BLUE_HIGH     31
#define LED_RED_HIGH    (31 << 11)
#define LED_RED_LOW     (7 <<  11)
#define LED_GREEN_LOW     (15 << 5)
#define LED_BLUE_LOW     7
#define LED_WHITE_LOW    (LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define matrixBrightness 32
#define stripBrightness 64

//#include <Adafruit_NeoPixel.h> // Old Library that is no longer used
#include <Adafruit_GFX.h>
//#include <Adafruit_NeoMatrix.h> // Old Library that is no long used
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include "raceCar.h"  //This is the image for the 8 x 16 pixel race car image
#include "patterns.h"
#include "matrix.h"

int core0LastUpdate = 0;

/**************************************************************
   Lane Definitions looking head on at the 8 X 24 Matrix
   postion, colour
   Lane1 - Left, Red
   Lane2 - Center, Blue
   Lane3 - Right, Green

*/
//variables for reading finish line

// Pattern types supported:

// Patern directions supported:

//Matrix Pattern types supported:


/**********************************************************************/
/*Note the fastest update time of the whole program is 40ms because of the time to refresh the lights.
   Any interval below 40 will not be effectual.
   To increase the refresh rate there are two options:
   1.  Move to a led strip and matrix type with a faster refresh rate
   2.  There are some parallel output methods that can be used on different microcontrollers.  If this can work on ESP32 we
   can use the parallel output function and get a refresh rate coloser to 15ms.
*/



//matrix class for running the 24X8 matrix, almost identical to the NeoPatterns class

// NeoPattern Class - derived from the Adafruit_NeoPixel class

void Lane1Complete();
void Lane2Complete();
void Lane3Complete();
void matrixComplete();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
NeoPatterns Lane1(432, &Lane1Complete);
NeoPatterns Lane2(432, &Lane2Complete);
NeoPatterns Lane3(432, &Lane3Complete);

Matrix matrix(8, 8, 3, 1, &matrixComplete);

TaskHandle_t Task1;

// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);

  // Initialize all the pixelStrips
  FastLED.addLeds<WS2812B, 5, GRB>(Lane1.leds, Lane1.numLeds).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, 15,GRB>(Lane2.leds, Lane2.numLeds).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, 13, GRB>(Lane3.leds, Lane3.numLeds).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, 12, GRB>(matrix.leds, matrix.numLeds).setCorrection( TypicalLEDStrip );

  //Set the LaneColor of each Lane

  Lane1.LaneColor = CRGB::Red;
  Lane2.LaneColor = CRGB::Blue;
  Lane3.LaneColor = CRGB::Green;


  matrix.begin();
  FastLED.setBrightness(64);
  matrix.setToRace();

  // limit my draw to 40A at 5v of power draw
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 40000);

  // Kick off a pattern for each lane
  Lane1.RainbowCycle(10);
  Lane2.RainbowCycle(10);
  Lane3.RainbowCycle(10);


  //initialize pin reading inputs
  for (count = 0; count < 3; count++)
  {
    pinMode(readPins[count], INPUT_PULLDOWN);
  }
  //set the variables that track which lane finished first
  place = 1;
  int laneReading[] = {0, 0, 0};
  int finishPlace[] = {0, 0, 0};
  bool complete[] = {false, false, false};

  randomSeed(analogRead(1));//randomseed for the random generator
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

}

// Main loop
void loop()
{
  // Update the lanes.
  Lane1.Update();
  Lane2.Update();
  Lane3.Update();
  matrix.Update();
  FastLED.show();
  //checkSensors();

  /*
    Serial.print("Lane1: ");
    Serial.print(Lane1.ActivePattern);
    Serial.print("\t");
    Serial.print("Lane2: ");
    Serial.print(Lane2.ActivePattern);
    Serial.print("\t");
    Serial.print("Lane3: ");
    Serial.print(Lane3.ActivePattern);
    Serial.println("");
  */


}
int currentMillis;
int lastUpdate;

void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    currentMillis = millis();
    for (int count = 0; count < 3; count++)
    {
      laneReading[count] = analogRead(readPins[count]);
      /*Serial.print("Lane");
      Serial.print(count);
      Serial.print(": ");
      Serial.print(laneReading[count]);
      Serial.print("\t");*/
    
    if (laneReading[count] > breakThreshold && !complete[count])
    {
      complete[count] = true;
      finishPlace[count] = place;
      place++;
      matrix.processingResults();
    }
    }
    /*
    Serial.print("Update Time: ");
    Serial.print(currentMillis - lastUpdate);
    Serial.println("");*/
    lastUpdate = currentMillis;
    delay(1);
  }

}

/*
DEPRECIATED - used when working on single core only
void checkSensors()
{
  Serial.println(millis());
  for (count = 0; count < 3; count++)
  {
    laneReading[count] = analogRead(readPins[count]);

    if (laneReading[count] > breakThreshold && !complete[count])
    {
      complete[count] = true;
      finishPlace[count] = place;
      place++;
      matrix.processingResults();
    }
  }
}
*/


//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Lane1 Completion Callback
void Lane1Complete()
{
  Serial.print("Lane1 callback entered.");
  Serial.println(Lane1.Repeats);
  Lane1.Repeats++;
  if (Lane1.ActivePattern == RAINBOW_CYCLE)
  {
    Lane1.TheaterChase( CRGB::Black, Lane1.LaneColor, 100, REVERSE);
  }
  else if (Lane1.ActivePattern == THEATER_CHASE)
  {
    Lane1.Scanner(10);
  }
  else if (Lane1.ActivePattern == SCANNER && Lane1.Repeats > 2)
  {
    Lane1.RainbowCycle(10);
  }
}

// Lane2 Completion Callback
void Lane2Complete()
{
  Lane2.Repeats++;
  if (Lane2.ActivePattern == RAINBOW_CYCLE)
  {
    Lane2.TheaterChase(CRGB::Black, Lane2.LaneColor,  100, REVERSE);
  }
  else if (Lane2.ActivePattern == THEATER_CHASE)
  {
    Lane2.Scanner(10);
  } 
  else if (Lane2.ActivePattern == SCANNER && Lane2.Repeats >2)
  {
    Lane2.RainbowCycle(10);
  }
}



// Stick Completion Callback
void Lane3Complete()
{
  Lane3.Repeats++;
  if (Lane3.ActivePattern == RAINBOW_CYCLE)
  {
    Lane3.TheaterChase(CRGB::Black, Lane3.LaneColor, 100, REVERSE);
  }
  else if (Lane3.ActivePattern == THEATER_CHASE)
  {
    Lane3.Scanner(10);
  }
  else if (Lane3.ActivePattern == SCANNER && Lane3.Repeats > 2)
  {
    Lane3.RainbowCycle(10);
  }

}

void matrixComplete()
{

  matrix.Repeats++;
  if (matrix.ActivePattern == CAR && matrix.Repeats > 3)
  {
    matrix.setToRace();
  }
  else if (matrix.ActivePattern == SETRACE)
  {
    matrix.drawCar();
  }
  else if (matrix.ActivePattern == PROCESSING)
  {
    matrix.displayResults();
    flashWinningLane();
  }
  else if (matrix.ActivePattern == RESULTS)
  {
    reset();
  }

}

void reset()
{
  for (int i = 0; i < 3; i++)
  {
    finishPlace[i] = 0;
    complete[i] = false;
  }
  place = 1; //next place to cross the finish line is set to 1
  cursorStart = -1;
  matrix.setToRace();//start the loop pattern between setToRace and drawCar
  Lane1.RainbowCycle(40);
  Lane2.RainbowCycle(40);
  Lane3.RainbowCycle(40);
  raceFinished = false;//

}

void flashWinningLane()
{

  if (finishPlace[0] == 1)
  {
    Lane1.WinningLane();
    Lane2.Blank();
    Lane3.Blank();
  }
  else if (finishPlace[1] == 1)
  {
    Lane1.Blank();
    Lane2.WinningLane();
    Lane3.Blank();
  }
  else if (finishPlace[2] == 1)
  {
    Lane1.Blank();
    Lane2.Blank();
    Lane3.WinningLane();
  }

}

