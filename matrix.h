enum matrixPattern {RAINBOW, CAR, SETRACE, RESULTS, PROCESSING};
CRGB textColor[3] = {CRGB::Red, CRGB::Blue, CRGB::Green};
class Matrix : public FastLED_NeoMatrix
{
  public:
    matrixPattern ActivePattern; // Active pattern of the mattrix
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    direction Direction;     // direction to run the pattern
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    uint8_t Repeats;  //the numbrer of times a patten is to repeat is set in the call back function for this class
    void (*OnComplete)();  // Callback on completion of pattern
    CRGB* leds;
    int numLeds;
    // Constructor - calls base-class constructor to initialize matrix
    //NOTE TO FIX:  I couldn't get the type to be part of the constructor so that different matrices with different orientations would be possible
    Matrix(uint8_t MATRIX_W, uint8_t MATRIX_H, uint8_t TILES_W, uint8_t TILES_H,  void (*callback)())
      : FastLED_NeoMatrix(leds = new CRGB[MATRIX_W*MATRIX_H*TILES_W*TILES_H],MATRIX_W, MATRIX_H, TILES_W, TILES_H,NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE + NEO_TILE_TOP  + NEO_TILE_LEFT + NEO_TILE_COLUMNS + NEO_TILE_PROGRESSIVE)
      //Adafruit_NeoMatrix(MATRIX_W, MATRIX_H, TILES_W, TILES_H, pin, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE + NEO_TILE_TOP  + NEO_TILE_LEFT + NEO_TILE_COLUMNS + NEO_TILE_PROGRESSIVE, NEO_GRB + NEO_KHZ800)
    {
      OnComplete = callback;
      numLeds = MATRIX_W*MATRIX_H*TILES_W*TILES_H;
    }
    void Update()
    {
      if ((millis() - lastUpdate) > Interval) // time to update
      {

        lastUpdate = millis();
        switch (ActivePattern)
        {
          case RAINBOW:
            rainbowUpdate();
            break;
          case CAR:
            drawCarUpdate();
            break;
          case SETRACE:
            setToRaceUpdate();
            break;
          case RESULTS:
            displayResultsUpdate();
            break;
          case PROCESSING:
            processingResultsUpdate();
            break;
          default:
            break;
        }


      }
    }

 

    void drawCar()
    {
      Repeats = 0;
      ActivePattern = CAR;
      Interval = 25;
      Direction = FORWARD;
      Index = 48;
      TotalSteps = 48;
    }
    void drawCarUpdate()
    {

      //clear();//clear the screen before the next fram is set

      fillScreen(LED_WHITE_HIGH); //make the background white
      drawRGBBitmap(Index, 0, (const uint16_t *)raceCar, 16, 8);
      //show();
      Increment();

    }

    void rainbow()
    {
      ActivePattern = RAINBOW;
      TotalSteps = 256;//number of postions on the color wheel
      Direction = FORWARD;
      Interval = 3;

    }

    void rainbowUpdate() {

      uint16_t i, j;


      for (j = 0; j < 24; j++)
      {
        for (i = 0; i < 8; i++) { //196 is the number of leds in the matrix 3 x 8 x 8
          drawPixel(j, i, Wheel((j + (Index)) & 255));
        }
      }
      //show();
      Increment();
    }

    void setToRace()
    {
      setTextColor(CRGB::White);
      ActivePattern = SETRACE;
      Interval = 1000;
      TotalSteps = 5;
      Index = 0;
      Direction = FORWARD;
    }

    void setToRaceUpdate()
    {
      fillScreen(0);
      setCursor(textCursorStart[Index], 0); //vary the tart position of the text based on which word is being shown
      //Future - add random color generator to text color to make this more interesting.

      print(words[Index]);
      //show();
      Increment();

    }

    void processingResults()
    {
      ActivePattern = PROCESSING;
      Interval = 350;
      TotalSteps = 12;
      Index = 0;
      Direction = FORWARD;
    }

    void processingResultsUpdate()
    {
      
      if(cursorStart<2)
      {
        cursorStart++;
      }
      else
      {
        cursorStart = 0;
      }
      
      fillScreen(0);
      setTextColor(textColor[cursorStart]);
      setCursor(finishPlaceCursor[cursorStart],0);
      print("?");

      Increment();
      
    }

    void displayResults()
    {
      clear();
      setTextColor(CRGB::White);
      Interval = 2000;
      ActivePattern = RESULTS;

      TotalSteps = 2;
 
    
      for (int i = 0; i < numberOfLanes; i++)
      {
        setCursor(finishPlaceCursor[i], 0);
        if (finishPlace[i] == 0)
        {
          print("X");
        }
        else
        {
          print(finishPlace[i]);
        }
      }




    }

    void displayResultsUpdate()
    {
      
      
      Increment();
    }

    // Increment the Index and reset at the end
    void Increment()
    {
      if (Direction == FORWARD)
      {
        Index++;
        if (Index >= TotalSteps)
        {
          Index = 0;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else // Direction == REVERSE
      {
        --Index;
        if (Index <= 0)
        {
          Index = TotalSteps - 1;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color(255 - WheelPos * 3 , 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }

};
