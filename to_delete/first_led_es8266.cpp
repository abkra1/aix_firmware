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
//  led = Pin(4, Pin.IN)
//  Pin(5, Pin.OUT)
//   led.value(0)



// Declare our NeoPixel strip object:
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6
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
#define COL_WHITE 5 
#define COL_ORANGE 6
#define NUM_COLORS 7

#define COL_R 0
#define COL_G 1
#define COL_B 2

std::vector<std::vector<size_t>> Colors = {{255,0,0}, // red
                                           {0,255,0}, // green
                                           {0,0,255}, // blue
                                           {0,255,255}, // yellow
                                           {255,0,255}, // pink
                                           {255,255,255}, // white
                                           {255,155,155} // orange
                                          };
//
//   own LED calsses
//

class BaseLed {
  public:
  BaseLed() {};
  ~BaseLed() {};
  
  private:
  
};


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


// twincle


// WheelOfFortune
class SingleWheel {
  public:
    SingleWheel(size_t numLedsIn, size_t colIndex, size_t numLoops, float inWidth) :
    numLeds((float) numLedsIn),
    actPos(0.0),
    right(true),
    loops(numLoops),
    width(inWidth)
    {
      Init(colIndex, numLoops, inWidth);
    }
    ~SingleWheel() {}

    void Init(size_t colIndex, size_t numLoops, float inWidth) {
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
      if (random() > 0.5) {
        right = false;
        actPos = numLeds;

      }
      else {
        right = true;
        actPos = 0.0;
      }

      // calc max calls 
      maxCalls = (int) (random() * ((float) maxLoops) * inWidth) + 1;
      
      loops = 0;
      numCalls = 0;
    }

    void Next()
    {
      if (!IsDone()) {
        numCalls++;
        if (right) {
          actPos = actPos + (width / ((float)loops) + 1.0);
          if (actPos > numLeds) {
            actPos = actPos - numLeds;
            loops++;
          }
        }
        else {
          actPos = actPos - (width / ((float)loops) + 1.0);
          if (actPos < 0.0) {
            actPos = actPos + numLeds;
            loops++;
          }
        }
      }
    }

    bool IsDone() {
      return (numCalls >= maxCalls);
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
        if (i < wheels.size()) {
          wheels[i].Init(i, (int) (random() * 5.0 +1.0), random() * 20.0);
        }
        else {
          wheels.push_back(SingleWheel(LED_COUNT, i, (int) (random() * 5.0 +1.0), random() * 20.0));
        }
      }
    }
  
    void Next() {
      size_t i;
      for (i=0; (i < wheels.size()); i++) {
        wheels[i].Next();
      }
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

void setup() {
  // put your setup code here, to run once:
  // https://www.instructables.com/ESP8266-controlling-Neopixel-LEDs-using-Arduino-ID/
  //
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("setup loop\n");
  delay(5000);

}

void loop() {
    printf("start loop\n");

  // put your main code here, to run repeatedly:

  // init, call this only once
  Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
  printf("strip begin\n");
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  printf("strip brightness\n");
  strip.setBrightness(200); // Set BRIGHTNESS (max = 255)

  //   15 sec intro
  printf("3 run\n");
  int i = 0;

    for (i=0; i < 3 ; i++) {
      uint32_t nowCol;
      if (i==0) {
        nowCol = strip.Color(255,   0,   0);
      }
      else if (i==1) {
        nowCol = strip.Color(0,   255,   0);       
      }
      else if (i==2) {
        nowCol = strip.Color(0,   0,   255);       
      }
      size_t pixel;
      for (pixel = 0; pixel < LED_COUNT; pixel++) {
        strip.setPixelColor(pixel,nowCol);
      }
      // off we go
      printf("intro %d\n",i);
      strip.show();            // Turn OFF all pixels ASAP
      delay (5000);            // wait 5 secs for next loop
    } // 3 times

  //  intro done
  printf("start wheels\n");
  // wheel with 3 weels 
  // and there we go
  WheelOfFortune wheels(5);
  
  while (true) {
    wheels.Next();
    wheels.SetStrip(strip);

    // wait 1/10 sec
    delay(100);
    
    if (wheels.AllDone()) {
      printf("restart wheels\n"); 
      delay(10000);
      printf("and ... go\n"); 
      
      wheels.Init();
    }
    
  }

  

  // strip.end();

}
