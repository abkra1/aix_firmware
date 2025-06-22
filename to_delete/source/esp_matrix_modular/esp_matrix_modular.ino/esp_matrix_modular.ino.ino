#include "config.h"
#include <vector>
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
#include <NTPClient.h>
#endif
// really used wifi stuff
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
// matrix pixel stuff
#include <SPI.h>
// #include <Adafruit_GFX.h>
//#include <Max72xxPanel.h>
#include <MD_MAX72xx.h>
// from https://github.com/MajicDesigns/MD_MAX72XX/blob/main/examples/MD_MAX72xx_Pacman/MD_MAX72xx_Pacman.ino
// clock
#include <time.h>
// config stuff

#include <ArduinoUniqueID.h>

// include the webserver module / class
#include "config_filesystem/config_filesystem.h"
#include "config_webserver/config_webserver.h"
#include "config_httpget/config_httpget.h"

// configurtion stuff
//  saved on local
#include "FS.h"

// globals
//
//  
//  pinMode(6, OUTPUT); // INPUT
//  digitalWrite(13, HIGH); // sets the digital pin 13 on
//  digitalWrite(13, LOW);  // sets the digital pin 13 off



// Declare matrix object:
// Which pin on the Arduino is connected to the NeoPixels?
//#define LED_PIN    13
#define SWITCH_PIN  5
#define ERROR_PIN   4

// How many panels are attached
#define MAX_DEVICES 4 // your device count
#define LED_COLS 32
#define LED_ROWS 8

#define CLK_PIN   14 //D5 // or SCK
#define DATA_PIN  13 //D7 // or MOSI
#define CS_PIN    15 //D8 // or SS // you can set it to any pin

//#define CLK_PIN   SCL // or SCK
//#define DATA_PIN  MOSI // or MOSI
//#define CS_PIN    SS // CS // or SS // you can set it to any pin

//#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR1_HW // change according to your display type
#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR1_HW // change according to your display type


// for debugging return time string


String twoChars(int value) {
  String ret;
  ret = "00" + String(value);
  return ret.substring(ret.length()-2,ret.length());
  
}

String getTimestring(int timeZone) {
  
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        int hour = timeinfo->tm_hour + timeZone;
        if (hour > 23) {
          hour = hour - 24;
        }
        else if (hour < 0) {
          hour = hour + 24;
        }


        // we have someting looking like time
        String theTime = twoChars(hour) + ":" + twoChars(timeinfo->tm_min); //+ ":" + twoChars(timeinfo->tm_sec);
        return theTime;
  
}


String getTimesInt(int timeZone) {
  
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        int hour = timeinfo->tm_hour + timeZone;
        if (hour > 23) {
          hour = hour - 24;
        }
        else if (hour < 0) {
          hour = hour + 24;
        }

        // we have someting looking like time
        String theTime = twoChars(timeinfo->tm_year) + twoChars(timeinfo->tm_mon +1)+ twoChars(timeinfo->tm_mday) + twoChars(hour) + twoChars(timeinfo->tm_min) + twoChars(timeinfo->tm_sec);

        printf("getTimesInt(): %s\n",theTime.c_str());

        
        return theTime;
  
}


//    ------------   end global time helper

typedef std::vector<std::vector<bool>> field;

// matrix definiton
class MatrixDefinition {
    public:
        
    
        MatrixDefinition(uint8 colsIn, uint8 rowsIn):
        cols(colsIn),
        rows(rowsIn),
        actIndex(0)
        { printf("init MatrixDefinition\n");};
        ~MatrixDefinition() {};

        field getEmptyField() {
          field newField;
          for (size_t index = 0; index < cols; index++) {
            std::vector<bool> newRow(rows);
            for (size_t i = 0; i < rows; i++) {
              newRow[i] = false;
            }
            //printf("add new col\n");
            newField.push_back(newRow);
          }
          return newField;
        }


        void addField(field data) {
          /*
          field newField[32,8];
          uint8 col;
          for (col = 0; col < data.length(); col++) {
            uint8 row;
            for (row = 0; row < rowData.length(); row++) {
              field[col,row] = rowData
            }
            newField.add(col,rowData)
          }
          */
          printf("add field\n");
          content.push_back(data);
        }

        field* getNextField() {
          if (content.size() == 0) {
            printf("empty definiton\n");
            return NULL;
          }
          actIndex = actIndex+1;
          printf("index %d\n", actIndex);
          if (actIndex >= content.size()) {
            actIndex=0;
          }
          // printf("field index %d\n", actIndex);
          return &content[actIndex];
        }
        

    private:
        std::vector<field> content;
        uint8 rows;
        uint8 cols;
        size_t actIndex;

};





// matrix mapper
// since the font for this hardware is broken, we need to define our own sh***


class MatrixHandler {
    public:
      ~MatrixHandler()
      {};
       
      MatrixHandler(size_t cols, size_t rows, bool invertIn):
      numCols(cols),
      numRows(rows),
      invert(invertIn)
      {
          printf("init Matrix, inverted: %d \n", invert);
  
          matrix = new MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

          matrix->begin();           // INITIALIZE object (REQUIRED)
          matrix->setFont(nullptr);  // broken, but set to default
          // matrix->transform(MD_MAX72XX::TFLR);      // ?????   
       }

    void setBrightness(uint8 brightness) {
      if (matrix) {
        matrix->control(MD_MAX72XX::INTENSITY, brightness);
      }
    }

    void setChar(uint8 col, uint8 character) {
      if (matrix) {
        clear();
        printf("unsupported ----------------> setChar() !!\n");
        matrix->setChar(toCol(col), character);
      }
    }

    void clear() {
     if (matrix) {
        if (!inDraw) {
          // printf("clearing\n");
          matrix->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
          // matrix->transform(MD_MAX72XX::TFLR);      // ?????   
          matrix->clear();
        }
        inDraw = true;
     }
    }


    void show() {
      if (matrix) {
        inDraw = false;
        matrix->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
      }
    }


    bool setPoint(uint8 col, uint8 row, bool on) {
     if (matrix) {
        // clear();
        /*
        if (invert) {
          return matrix->setPoint(toRow(row),toCol(col), !on);
        }
        if (on) {
            printf("showing %d %d\n", col, row);
        }
        */
        // note the matrix has row and col switched
        return matrix->setPoint(toRow(row),toCol(col), on);
     }
    }

    void print(uint8 col, String text) {
          print(col, text, 6);
    }
    
    void print(uint8 col, String text, uint8 charWidth) {
      if (matrix) {
          // clear();
          for (uint8 i=0; i<text.length(); i++) {
            matrix->setChar(toCol(col + (i*charWidth)), text[i]);        
          }
      }
    }

    void showField(field* data) {
      if (matrix && data) {
          clear();
          for (size_t col = 0; col < numCols; col++) {
            for (size_t row = 0; row < numRows; row ++) {
              setPoint(col, row, (*data)[col][row]);
            }
          }          
          show();
      }
    }
    


    private:


      uint8 toCol(uint8 col) {
        if (invert) {
          return 8*MAX_DEVICES-1-col;
        }
        return col;
      }
      uint8 toRow(uint8 row) {
        if (invert) {
          return 7-row;
        }
        return row;
      }

      bool invert;
      bool inDraw;
      MD_MAX72XX* matrix;
      size_t numCols;
      size_t numRows;
        
};



// clock
// get time from ntp.org
//
class ShowFrames {
  public:
    ShowFrames(size_t cols, size_t rows):
    numRows(rows),
    numCols(cols),
    enabled(-1),
    definition(NULL)
    {
        definition = new MatrixDefinition(numCols, numRows);
    }

    void ParseFieldString (int enabledIn, int frame, String definitionString) {
        size_t r = 0;
        size_t c = 0;

        if (enabledIn != enabled) {
          delete definition;
          definition = new MatrixDefinition(numCols, numRows);
          enabled = enabledIn;
        }
        
        if (definitionString.length() > 10) {
          
           field newField = definition->getEmptyField();

           printf("parsing definitin string \n");
           
           for (c = 0; c < numCols ; c++) {
             for (r = 0; r < numRows; r++) {
               // filed index is col, row
               String marker = "_" + String(c*8 + r) + "_id";
               //printf("marker %s\n",marker.c_str());
               if (definitionString.indexOf(marker,0) >= 0) {
                    //printf("setting %d %d true\n",c,r);
                    newField[c][r] = true;
               }
               else {
                    //printf("setting %d %d false\n",c,r);
                    newField[c][r] = false;
               }
             }
           }

           definition->addField(newField);  
           
        }
    }

    bool EnabledChanged (int enabledIn) {
      return (enabledIn != enabled);
    }
 
    ~ShowFrames() {};
    
    void Init() {};
    

    void Next(MatrixHandler* matrix) {
      //delay(msWaitIn);
      // hour in 12 format
      // int hour24 = (int)(((float)(hour % 12)) * factor * 5);
      
      // printf("h: %d m: %d s: %d\n",hour12, minute60,second60);
      // String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);
      //String timeStr = getTimestring(timeZone);
      
      //if (lastTimeStr != timeStr) {
        //printf(String("Timestr: "+ timeStr + "\n").c_str());
        //lastTimeStr = timeStr;
        /*
        matrix->clear();
        matrix->print(0, timeStr);
        //matrix->setChar(15,'a');
        //matrix->setChar(23,timeStr[timeStr.length()-1]);
        //matrix.setTextColor(matrix.Color(colorR,colorG,colorB));
        //matrix.setCursor(1, 0);
        //matrix.print(timeStr);
        matrix->show();
        */
        matrix->showField(definition->getNextField());
        
      }
    

  private:

    size_t numRows;
    size_t numCols;
    int enabled;

    MatrixDefinition* definition;
};




//
//     ---------------------  main -------------------------------
//

// config page
String matrixVersion = "0.05";


//
//   WIFI shit ... we make it global
//
//
//   another part of spaghetti code 
//    wifi config mode
//

//IPAddress local_IP(192,168,0,1);
//IPAddress gateway(192,168,0,1);
//IPAddress subnet(255,255,255,0);
bool refreshProxy = true;
WifiGetter* getter = NULL;
String hardwareDdeviceID ="empty";

MatrixHandler* matrix;
ShowFrames* frames;

//
//  globals definition
//
//   what to startup
//
String mode = "FRAMES";
String oldMode = "x";
String oldIndex = "x";
String oldEnable = "x";
String oldDelay = "x";
String oldBrightness = "x";

int delayMs = 100;
int enable = 0;
int brightness = 5;

void setGlobals () {

  // we will move this later ... just somewhere else
  ConfigData* wifiData = new ConfigData();
  
  printf("setGlobals: init\n");
  // digitalWrite(ERROR_PIN,HIGH);

  if (getter == NULL) {
      getter = new WifiGetter(wifiData->getWifiSid(), 
                              wifiData->getWifiPassword(), 
                              wifiData->getValue("redirectwebserver"), 
                              wifiData->getValue("redirectwebserverport").toInt(), 
                              wifiData->getValue("redirectwebserverpage"));
  }
   

  // auth string is set as 
  //  base64 encoded user:password
  // 

  // This will send the request to the server

  //String  httpRequest = String("GET /axclock/params.sh?device_id=") + wifiData->getWifiDeviceId() + String(" HTTP/1.1\r\n") +
  String  httpRequest = String("GET /axmonomatrix/params_") + wifiData->getWifiDeviceId() + String(".html HTTP/1.1\r\n") +
               String("Host: ") + getter->GetRealIP() + String("\r\n") + 
               String("Authorization: Basic ") + wifiData->getValue("redirectwebserversecret") + String("\r\n\r\n");

     // YmxpbmRlcjpHaWJIZXI=
  
  String line;
  if (getter->sendHttpRequest(httpRequest, line, refreshProxy)) {

    printf("-------------------\n");
    printf("request:\n%s\n", httpRequest.c_str());
    printf("-------------------\n");
    printf("line:\n%s\n", line.c_str());
    printf("-------------------\n");
 
    if (line.length() > 800) {

      // String newIndex = getter->parseHtml(line,String("index"),oldIndex);
      String newEnable = getter->parseHtml(line,String("enable"),oldEnable);
      String newDelay = getter->parseHtml(line,String("delay"),oldDelay);
      String newBrightness = getter->parseHtml(line,String("brightness"),oldBrightness);


  
      // printf("new: %s %s %s\n",newMode, newStart, newWheels);
  #if 0    
      // mode is switched my new string
      String newMode = getter->parseHtml(line,String("mode"),oldMode);
      if (newMode != oldMode) {
        oldMode = newMode;
        mode = newMode;
        printf("setting mode to %s\n",mode);
      }
  #endif
      
      if (newBrightness != oldBrightness) {
        oldBrightness = newBrightness;
        int bright = oldBrightness.toInt();
        if ((bright >= 0) && (bright < 15)) {
          brightness = bright;
        }
        printf("setting brightness to %d\n",brightness);
      }
      
      if (newEnable != oldEnable) {
        oldEnable = newEnable;
        int ena = oldEnable.toInt();
        if ((ena >= 0) && (ena < 15)) {
          enable = ena;
        }
        printf("setting enable to %d\n",enable);
      }
      
      if (newDelay != oldDelay) {
        oldDelay = newDelay;
        int dela = oldDelay.toInt();
        if ((dela >= 0) && (dela < 300000)) {
          delayMs = dela;
        }
        
        printf("setting delay to %d\n",delayMs);
      }
      
      if (frames->EnabledChanged(enable)) {
        String newData = getter->parseHtml(line,String("frame_")+oldEnable+String("_0"),"");
        int loops = 0;
        while ((newData.length() > 10) && (loops < 10)) {
          printf("parsing setter: %s\n",newData.c_str());
          frames->ParseFieldString(enable, loops, newData);
          loops++;
          newData = getter->parseHtml(line,String("frame_")+oldEnable+String("_")+String(loops),"");
        }
      }
      else {
        printf("same definition, skipping\n");
      }
      printf("reply parsed\n  Index: %s\n", oldEnable.c_str());
      
    }
    else {
      printf("reply too short, retry next iteration\n");
      // failed on first connect ... enforce retry
      if (oldEnable == "x") {
        refreshProxy = true;
        digitalWrite(ERROR_PIN,HIGH);
      }
    }
      refreshProxy = false;
      digitalWrite(ERROR_PIN,LOW);
  }
  else {
    printf("failed, full refresh initiated\n");
    refreshProxy = true;
    digitalWrite(ERROR_PIN,HIGH);
  }
    // printf("setGlobals: finished: %s\n", getTimestring());

}





//
//   the init / setup boot loop (called only once)
//

void setup() {

  //  do NOT use the IO-pin (6) from some examples !!
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100); // we need time to switch the port
  // if we have a problem, give me 1-2 secs to interrupt on startup

  // read the unique string, we should use this as setter for the default ID
  // maybe even the ID is non-mutable in next release
  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  String idStr = "";
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + " "; 
     hardwareDdeviceID = String(buff); // let us hope this is "sufficiently unique"
  }
  
  printf("\n---------------------------------------------------------------\n");
  printf("        Version %s\n",matrixVersion.c_str());
  printf("        Id %s\n",idStr.c_str());
  printf("        HardwareId %s\n",hardwareDdeviceID.c_str());
  printf("---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");
  
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(1000);
  digitalWrite(ERROR_PIN,HIGH);

  matrix = new MatrixHandler(LED_COLS, LED_ROWS, false);
  frames = new ShowFrames(LED_COLS, LED_ROWS);
  
  printf("setup done\n");

}


//
//   the main loop to execute
//
void loop() {

  bool configMode = digitalRead(SWITCH_PIN); // open+3.3v = true, gnd = false
  
  printf("start loop\n");


  //   15 sec intro
  //printf("intro run\n");
  int i = 0;
  {  // as init show all colors and pixels for 2 secs
 
     printf("intro \n");

     ConfigData* wifiData = new ConfigData();
     
     matrix->clear();
     if (configMode) {
       matrix->setChar(18,'S');
     }
     else {
       String initStr = "id: " + wifiData->getWifiDeviceId();
       matrix->setChar(18,'I');
     }
     //matrix.show(); 

     delete wifiData;
     
     printf("done show\n");
     delay (2000);            // wait 5 secs for next loop
  } // 3 times

  size_t loops = 0;

  if (configMode)
  {
     printf("enter config mode\n");
     // create nec config instance and
     // open access point
     ConfigData* confData = new ConfigData();
     WifiConfigWebserver* configServer = new WifiConfigWebserver(confData, hardwareDdeviceID);
     configServer->runAcessPoint(); // this does not return
  }
  else {
    setGlobals();
    // this call should have fixed

    while (true) {
    
      if (mode == "FRAMES") {
          printf("start frames %s\n", oldEnable.c_str());
                    
          while ((loops < 1000000) && (mode == "FRAMES")) {
            matrix->setBrightness(brightness);
            frames->Next(matrix);
            delay(delayMs);
            loops++;
            if (loops%500 == 0) {
              setGlobals();
            }
          }
      }
      else {
        printf("unknown mode: %s, using FRAMES\n",mode);
        oldMode = mode;
        mode = "FRAMES";
        delay(20000);
      }
    }
  }
  printf("ending + restart\n");
  // strip.end();

}
