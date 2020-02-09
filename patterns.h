
byte readPins[] = {39,2, 36};
int breakThreshold = 2000;
int laneReading[] = {0, 0, 0};
int numberOfLanes = 3;
int count = 0;
int laneNumber;
int finishPlace[] = {0, 0, 0};
int finishPlaceCursor[] = {2, 9, 17}; //for the matrix, the starting cursor position of each lane
int cursorStart = -1;//used in processingResultsUpdate(), define cursor start position to move the ? through each lane, this starts at -1 so the cursor postion will start at position 0 after increment
bool complete[] = {false, false, false};
int place; // temporary place holder to store which place is next to cross the finish line
bool raceFinished = false;

String words[4] = {"GET", "SET", "TO", "RACE"}; //words to display on the screen prior to race
int textCursorStart[4] = {3, 3, 6, 0};
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, WINNING_LANE, MOVING_DOT, BLANK };
enum  direction { FORWARD, REVERSE };
class NeoPatterns //: public Adafruit_NeoPixel
{
  public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    CRGB Color1, Color2;  // What colors are in use
    CRGB LaneColor;
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    uint8_t Repeats;  //the numbrer of times a patten is to repeat is set in the call back function for this class
    void (*OnComplete)();  // Callback on completion of pattern
    CRGB* leds;
    int numLeds;
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, void (*callback)())
    {
      numLeds = pixels;
      leds = new CRGB[numLeds];
      
      OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {

      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        
        /*Serial.print("Active Pattern: ");
        Serial.print(ActivePattern);
        Serial.print("\t");
        Serial.print("Last Update: ");
        Serial.println(lastUpdate);*/
        switch (ActivePattern)
        {
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
          case COLOR_WIPE:
            ColorWipeUpdate();
            break;
          case SCANNER:
            ScannerUpdate();
            break;
          case FADE:
            FadeUpdate();
            break;
          case WINNING_LANE:
            WinningLaneUpdate();
            break;
          case BLANK:
            BlankUpdate();
            break;
          case MOVING_DOT:
            MovingDotUpdate();
            break;
          default:
            break;
        }
      }
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

    // Reverse pattern direction
    void Reverse()
    {
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else
      {
        Direction = FORWARD;
        Index = 0;
      }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 800;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    { 
      int gHue = Index * 2;
      fill_rainbow( leds, numLeds, gHue, 7);
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(CRGB color1, CRGB color2, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = THEATER_CHASE;
      Interval = interval;
      TotalSteps = 100;
      Color1 = color1;
      Color2 = color2;
      Index = 100;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
      for (int i = 0; i < numLeds; i++)
      {
        if ((i + Index) % 3 == 0)
        {
          leds[i] = Color1; //setPixelColor(i, Color1);
        }
        else
        {
          leds[i] = Color2; //setPixelColor(i, Color2);
        }
      }
      //show();
      Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = COLOR_WIPE;
      Interval = interval;
      TotalSteps = numLeds;
      Color1 = color;
      Index = 0;
      Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
      leds[Index] = Color1; //setPixelColor(Index, Color1);
      //show();
      Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint8_t interval)
    {
      ActivePattern = SCANNER;
      Interval = interval;
      TotalSteps = (numLeds - 1);
      Index = (numLeds - 1);
      Direction = REVERSE;
      Repeats = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    {
      for (int i = 0; i < numLeds; i++)
      {
        if (i == Index)  // Scan Pixel to the right
        {
          leds[i] = LaneColor; //setPixelColor(i, Color1);
        }
        else if (i == TotalSteps - Index) // Scan Pixel to the left
        {
          leds[i] = LaneColor;//setPixelColor(i, Color1);
        }
        else // Fading tail
        {
          leds[i] = DimColor(leds[i]); //setPixelColor(i, DimColor(getPixelColor(i)));
        }
      }
      //show();
      Increment();
 
    }
        // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    CRGB DimColor(CRGB color)
    {
      // Shift R, G and B components one bit to the right
      CRGB dimColor = color.fadeToBlackBy(64);
      return dimColor;
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = FADE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
      uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
      uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

      ColorSet(CRGB(red, green, blue));
      //show();
      Increment();
    }

    void  MovingDot(uint8_t interval)
    {
      ActivePattern = MOVING_DOT;
      Interval = interval;
      TotalSteps = numLeds;
      Index = 0;
      Direction = FORWARD;
      
      
    }

    void MovingDotUpdate()
    {
      for(int i = 0; i < numLeds; i++)
      {
        leds[i] = CRGB::Black;
      }
      leds[Index] = CRGB::Red;
      Increment();
      
    }

    void WinningLane()
    {
      ActivePattern = WINNING_LANE;
      Interval = 250;
      TotalSteps = 20;
    }

    void WinningLaneUpdate()
    {
      if(leds[0])
      {
       fill_solid(leds,numLeds,0);  //make the strip black
       
      }
      else
      {
       fill_solid(leds, numLeds/*number of leds*/, LaneColor);
      }  
      Increment(); 

    }

    void Blank()
    {
     ActivePattern = BLANK;
     Interval = 1000;
     TotalSteps = 2;
      
    }

    void BlankUpdate()
    {
      fill_solid(leds,numLeds,0); //make the whole strip black
      Increment();
    }



    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (int i = 0; i < numLeds; i++)
      {
        leds[i] = color; //setPixelColor(i, color);
      }
   
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
      return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }


};

