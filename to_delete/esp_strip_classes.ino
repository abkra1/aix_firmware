#include "config.h"
#if 0
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiServerSecureBearSSL.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiGratuitous.h>
#include <WiFiServerSecure.h>
#include <ArduinoWiFiServer.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#endif
// pixel stuff
#include <Adafruit_NeoPixel.h>


// globals
//
//  
//  pinMode(6, OUTPUT); // INPUT
//  digitalWrite(13, HIGH); // sets the digital pin 13 on
//  digitalWrite(13, LOW);  // sets the digital pin 13 off



// Declare our NeoPixel strip object:
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    13
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 150
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);



//
//   define some basic colors
//
#define COL_RED 0 
#define COL_GREEN 1 
#define COL_BLUE 2 
#define COL_YELLOW 3
#define COL_PINK 4
#define COL_ORANGE 5
#define COL_WHITE 6 
#define NUM_COLORS 7

#define COL_R 0
#define COL_G 1
#define COL_B 2

std::vector<std::vector<size_t>> Colors = {{255,0,0}, // red
                                           {0,255,0}, // green
                                           {0,0,255}, // blue
                                           {155,50,10}, // yellow
                                           {255,0,255}, // pink
                                           {255,25,5}, // orange
                                           {255,255,255}, // white
                                          };

// define the miodes
#define WHEEL 0
#define LEDCLOCK 1
#define TWINCLE 2
#define RUNNER 3


//  runner
class SingleRunner {
  public:
    SingleRunner() {
    
    };
    ~SingleRunner() {};
  private:
  


  
};
class MultiRunner {
  public:
    MultiRunner() {};
    ~MultiRunner() {};
  private:

  
};

// clock
// attention need to get the real time from somewhere ???
//
class LedClock {
  public:
    LedClock(size_t numLedsIn, bool circleIn):
    circle(circleIn),
    lightBefore(false),
    numLeds(numLedsIn),
    hour(2),
    minute(10),
    second(0),
    fraction(0.0)
    {
      factor = ((float)numLeds) / 60.0;
    };
    ~LedClock() {};
    
    void Init() {};
    
    int GetNext(int in) {
      if (in <= 0) {
        return 59;
      }
      if (in >= 60) {
        return 0;
      }
      return in;
    }
    int GetPrev(int in) {
      if (in <= 0) {
        return 59;
      }
      if (in >= 60) {
        return 0;
      }
      return in;
    }


    void Next(int msWaitIn, Adafruit_NeoPixel& strip) {
      SetTime(msWaitIn);
      // hour in 12 format
      int hour12 = (int)(((float)(hour % 12)) * factor);
      // minute in 60
      int minute60 = (int) ((float)minute * factor);
      int second60 = (int) ((((float)second)+fraction) * factor);
      //bool lightBefore = false;
      //if ((minute60 % 2) > 0) {
      //  lightBefore = true;
      //}

      ////printf("second: %d 60: %d before: %d\n",second, second60, lightBefore);


      // all dark
      // strip.clear(); 
      size_t pix;
      for (pix = 0; pix < numLeds; pix++) {
        size_t red = 0;
        size_t green = 0;
        size_t blue = 0;

        // make elseif -> no merge
        if (pix == minute60) {
          blue = 200;
        }
        else if (pix == hour12) {
          green = 200;
        }
        else if (pix == second60) {
          red = 200;
        }
          // now check for seconds "ring"
        else if (circle) {
          if (lightBefore) {
            if (pix < second60) {
              red = 30;
            }
          }
          else {
            if (pix > second60) {
              red = 30;
            }
          }
        }
        else { // no circle
          if ((pix == (GetPrev(pix))) || (pix == (GetNext(pix)))) {
            red = 30;
          }
        }
        strip.setPixelColor(pix,Adafruit_NeoPixel::Color(red, green, blue));
      }  // led count loop
      // now show it
      strip.show();
    }
    
    void SetTime(int msWaitIn) {
      fraction = fraction + (((float)msWaitIn)/1000.0);
      if (fraction >= 1.0) {
        fraction = 0.0;
        second = second + 1;
      }
      if (second > 59) {
        second = 0;
        minute = minute +1;
        //if (lightBefore) {
        //  lightBefore = false;       
        //}
        //else {
        //  lightBefore = true;
        //}
        lightBefore = !lightBefore;
        ////printf("new minute %d\n",lightBefore);
      }
      if (minute > 59) {
        minute = 0;
        hour = hour+1;
      }
    }
  
  private:
    bool circle;
    bool lightBefore;
    size_t numLeds;
    int hour;
    int minute;
    int second;  
    float fraction;
    float factor;
};



// twincle
class Twincle {
  public:
    Twincle(size_t numLedsIn, size_t percentOnIn, size_t percentChangeIn):
    numLeds(numLedsIn),
    percentOn(percentOnIn),
    percentChange(percentChangeIn)
    {
      
      size_t pix;
      for (pix=0; (pix < numLeds); pix++) {
        if (percentOnIn > random(100)) {  
          stars.push_back(GetBrightness());
        }
        else {
          stars.push_back(0);
        }
      }
      
    }
    ~Twincle() {};

    int GetBrightness() {
      return random(2) +1;
    }

    uint32_t GetDimmedPixel(int factor) { 
      int realFactor = factor;
      return Adafruit_NeoPixel::Color(Colors[COL_YELLOW][COL_R] / realFactor, Colors[COL_YELLOW][COL_G] / realFactor, Colors[COL_YELLOW][COL_B] / realFactor);
    }

    

    void Next(Adafruit_NeoPixel& strip) {
      // loop all pix any change when needed
      size_t pix;
      for (pix=0; (pix < stars.size()); pix++) {
        // only change a small part
        if (percentChange > random(100)) {
          if (percentOn > random(100)) {  
            stars[pix] = GetBrightness();
          }
          else {
            stars[pix] = 0;
          }
        }
        // now set the light
        if (stars[pix] > 0) {
          strip.setPixelColor(pix,GetDimmedPixel(stars[pix]));
        }
        else {
          strip.setPixelColor(pix,0);
        }
      }
      strip.show();
    }
    
  private:
    size_t numLeds;
    size_t percentOn;
    size_t percentChange;
    std::vector<size_t> stars;
  
};



// WheelOfFortune
class SingleWheel {
  public:
    SingleWheel(size_t numLedsIn, size_t colIndex, size_t numLoops, float inWidth, bool bounceIn) :
    numLeds((float) numLedsIn),
    actPos(0.0),
    right(true),
    loops(numLoops),
    width(inWidth),
    bounce(bounceIn)
    {
      Init(colIndex, numLoops, inWidth);
    }
    ~SingleWheel() {}

    void Init(size_t colIndex, size_t numLoops, float inWidth) {
      //printf("SingleWheel::init\n");
      if (numLoops < 50) {
        maxLoops = numLoops;
      }
      else {
        maxLoops = 10; 
      }
      if (colIndex < NUM_COLORS) {
        colorR = Colors[colIndex][COL_R];     
        colorG = Colors[colIndex][COL_G];      
        colorB = Colors[colIndex][COL_B];     
      }  
      else {
        colorR = Colors[COL_WHITE][COL_R];     
        colorG = Colors[COL_WHITE][COL_G];     
        colorB = Colors[COL_WHITE][COL_B];   
      }
      if ((inWidth > 0.0) && (inWidth < 20.0)) {
        width = inWidth;
      }
      else {
        width =0.1;
      }
      if (random(10) > 5) {
        right = false;
        actPos = numLeds;

      }
      else {
        right = true;
        actPos = 0.0;
      }

      // calc max calls 
      // maxCalls = (int) (random(10) * ((float) maxLoops) * inWidth) + 1;
      maxCalls = (int) (random(2*numLeds) + numLeds);
      
      loops = 0;
      numCalls = 0;

      //printf("init:\nmaxCalls: %d\nwidth: %f\nmaxLoops: %d \n",maxCalls,width,maxLoops);

      
    }

    void Next()
    {
      if (!IsDone()) {
        numCalls++;
        if (right) {
          //actPos = actPos + (width / ((float)loops) + 1.0);
          actPos = actPos + width;
          if (actPos > numLeds-1) {
            if (bounce) {
             right = false;
             actPos = (float)numLeds -width;
            }
            else {
             actPos = actPos - (float)numLeds;
            }
            loops++;
          }
        }
        else {
          // actPos = actPos - (width / ((float)loops) + 1.0);
          actPos = actPos - width;
          if (actPos < 0.0) {
            if (bounce) {
             right = true;
             actPos = width;
            }
            else {
              actPos = actPos + (float)numLeds;
            }
            loops++;
          }
        }
        width = width * (1.0-1.0/maxCalls);
        //printf("SingleWheel::Next %f width %f  calls: %d of %d\n", actPos, width, numCalls, maxCalls);
      }
      else {
        //printf("SingleWheel::Next is Done\n");
      }
    }

    bool IsDone() {
      return ((numCalls >= maxCalls) || (width < 0.05));
    }

    bool IsPixel(size_t pixel) {
       return (pixel == (size_t) actPos);
    }

    uint32_t GetColor() {
      return Adafruit_NeoPixel::Color(colorR, colorG, colorB);
    }
    uint32_t GetDimmedColor(float factor) {
      return Adafruit_NeoPixel::Color(colorR * factor, colorG * factor, colorB * factor);
    }
    
    size_t GetR() {
      return colorR;
    }
    size_t GetG() {
      return colorG;
    }
    size_t GetB() {
      return colorB;
    }

  
  private:
  
    int colorR;
    int colorG;
    int colorB;
    float numLeds;
    float actPos;
    float width;
    int loops;
    int maxLoops;
    int numCalls;
    int maxCalls;
    bool right;
    bool bounce;
};


class WheelOfFortune {
  public:
    WheelOfFortune(size_t maxWheels):
    numWheels(maxWheels) 
    {
      Init();
    }
    ~WheelOfFortune() {}
  
    void Init() 
    {
      int i;
      for (i=0; (i < numWheels) && (i < NUM_COLORS) ; i++) {
        if (i >= wheels.size()) {
          wheels.push_back(SingleWheel(LED_COUNT, i, (int) (random(5)+1), 2.0 + ((float)random(20.0)) / 5.0, true));
        }
        else {
          wheels[i].Init(i, (int) (random(5)+1), 2.0 + ((float)random(20.0)) / 5.0);
        }
      }
      //printf("size: %d\n",wheels.size());
    }
  
    void Next(Adafruit_NeoPixel& strip) {
      size_t i;
      for (i=0; (i < wheels.size()); i++) {
        wheels[i].Next();
      }
      SetStrip(strip);
    }
    
    bool AllDone() {
      size_t i;
      for (i=0; (i < wheels.size()); i++) {
        if (!wheels[i].IsDone()) {
          return false; 
        }
      }
      return true;
    }

    void SetStrip(Adafruit_NeoPixel& strip) {
      size_t pixel;
      size_t wheel;
      // clear the strip (all off)
      strip.clear(); 
      for (pixel = 0; pixel < LED_COUNT; pixel++) {
        // clear the pixel (off)
        // done by clear() -> strip.setPixelColor(pixel, Adafruit_NeoPixel::Color(0,0,0));
        
        // set if position matches
        for (wheel = 0; wheel < wheels.size(); wheel++) {
          if (wheels[wheel].IsPixel(pixel)) { // position of the fortune wheel
            // extract all wheels
            // we need to do something about "overwriting here"
            strip.setPixelColor(pixel,wheels[wheel].GetColor());
            //("%d color\n", pixel);
            // before pixel
            if (pixel == 0) {
              strip.setPixelColor(LED_COUNT-1,wheels[wheel].GetDimmedColor(0.3));
            }
            else {
              strip.setPixelColor(pixel-1,wheels[wheel].GetDimmedColor(0.3));
            }
            // after pixel
            if (pixel == LED_COUNT-1) {
              strip.setPixelColor(0,wheels[wheel].GetDimmedColor(0.3));
            }
            else {
              strip.setPixelColor(pixel+1,wheels[wheel].GetDimmedColor(0.3));
            }
          }
        }
      }
      // make it visible
      strip.show();
    }

  private:
    size_t numWheels;
    std::vector<SingleWheel> wheels; 

    
};


//
//     ---------------------  main -------------------------------
//

void setup() {
  // put your setup code here, to run once:
  // https://www.instructables.com/ESP8266-controlling-Neopixel-LEDs-using-Arduino-ID/
  //  do NOT use the IO-pin from the example !!
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100); // we need time to switch the port
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("\n---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  //printf("setup done\n");

}

void loop() {
  printf("start loop\n");

  // put your main code here, to run repeatedly:

  // init, call this only once
  Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
  //printf("strip begin\n");
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  //printf("strip brightness\n");
  strip.setBrightness(20); // Set BRIGHTNESS (max = 255)

  //   15 sec intro
  //printf("intro run\n");
  int i = 0;

  //for (i=0; i < 3 ; i++) {}
  for (i=0; i < 1 ; i++) {
      uint32_t nowCol;
      if (i==0) {
        //printf("R\n");
        size_t col = COL_ORANGE;
        nowCol = Adafruit_NeoPixel::Color(Colors[col][COL_R], Colors[col][COL_G], Colors[col][COL_B]);
        //nowCol = strip.Color(255,   0,   0);
      }
      else if (i==1) {
        //printf("G\n");
        nowCol = strip.Color(0,   255,   0);       
      }
      else if (i==2) {
        //printf("B\n");
        nowCol = strip.Color(0,   0,   255);       
      }
      size_t pixel;
      for (pixel = 0; pixel < LED_COUNT; pixel++) {
        strip.setPixelColor(pixel,nowCol);
      }
      // off we go
      //printf("intro \n");
      strip.show();            // Turn OFF all pixels ASAP
      //printf("done show\n");
      delay (2000);            // wait 5 secs for next loop
  } // 3 times

  size_t loops = 0;
  int delayMs = 100; // wait 1/10 sec
  int mode = WHEEL;;
  if (mode == WHEEL) {
      //  intro done
      printf("start wheels\n");
      // wheel with 3 weels 
      // and there we go
      WheelOfFortune wheels(6);
      while (loops < 100000) {
        wheels.Next(strip);
    
        // wait 1/10 sec
        delay(delayMs);
        ////printf("100 wait done\n"); 
        
        if (wheels.AllDone()) {
          //printf("all done, restart wheels\n"); 
          wheels.Init();
          delay(10000);
          //printf("and ... go\n"); 
        }
        loops++;
      }
  }
  else if (mode == LEDCLOCK) {
      printf("start ledClock\n");
      LedClock ledClock(LED_COUNT, true);
      while (loops < 100000) {
        // wait 1/10 sec
        delay(delayMs);
        ledClock.Next(delayMs, strip);
        loops++;
      }
  }     
  else if (mode == TWINCLE) {
      printf("start twincle\n");
      Twincle twincle(LED_COUNT, 80, 1);
      
      while (loops < 100000) {
        // wait 1/10 sec
        delay(2*delayMs);
        twincle.Next(strip);
        loops++;
      }     
  }
  
  printf("ending + restart\n");
  // strip.end();

}
