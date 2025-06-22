#include "config.h"

// really used wifi stuff
#include <ESP8266WiFi.h>
// #include <WiFiClientSecure.h>
// pixel stuff
// clock
// config stuff
#include <ESP8266WebServer.h>

// configurtion stuff
//  saved on local
#include "SPI.h"
#include "SD.h"
#include <TMRpcm.h>

//  connections
#define LED_PIN    5
#define LED_GREEN_PIN    4
#define MIC_PIN    A0

//#define DHTYPE DHT22
//#define DHTPIN D4   // used for ESP8266
#define chipSelect  15  // used for ESP8266
const String fileName = "samples.wav";
// global SD access
//DHT dht(DHTPIN, DHTYPE);

//
//   recorder stuff
//

TMRpcm myAudio;


// sd rread write stuff
class Recorder {
  public:
    Recorder(int inDurationSecs, int inFrequency)
    {
       // allocate mem
       bufferSize = (size_t) inDurationSecs * inFrequency;
       microsPerSample = (unsigned long) (1000000.0 / (float) inFrequency); 
       // audioBuffer = (uint16_t*) malloc(2*(bufferSize + 1)); //  off by one :-)
       printf("Recorder() size %d rate %d\n", bufferSize, microsPerSample);
    }
    ~Recorder() 
    {
    }

    void writeSample(uint16_t inValue)
    {
       if (dataFile) {
         // uint16_t value = inValue * 32;
#if 0
         if ((inValue < 10) || (inValue > 1000))
         {
            printf("v:%hu\n", inValue);
         }
#endif         
         uint16_t value = inValue / 4;  // should be 32
         dataFile.write((uint8_t*)&value, 2);
         // printf("%d\n", value);
       }
    }

    void fillHeader()
    {
       if (dataFile) {
          dataFile.seek(0);
         // 44 byte riff / wav header 16 sample / rate from passed / size implicitely ... etc
           /*header*/
          unsigned int uiSampleSize = 2*bufferSize;
          char buff[44];
          /*id*/
          buff[0] = 0x52;
          buff[1] = 0x49;
          buff[2] = 0x46;
          buff[3] = 0x46;
          /*len-8*/
          buff[4] = (uiSampleSize+36)&0xFF; /*LSB*/
          buff[5] = ((uiSampleSize+36)/0x100)&0xff;
          buff[6] = ((uiSampleSize+36)/0x10000)&0xff;
          buff[7] = ((uiSampleSize+36)/0x1000000)&0xff;
          /*WAVE*/
          buff[8] = 0x57; /*LSB*/
          buff[9] = 0x41;
          buff[0xa] = 0x56;
          buff[0xb] = 0x45;
          /*Sign*/
          buff[0xc] = 0x66; /*LSB*/
          buff[0xd] = 0x6D;
          buff[0xe] = 0x74;
          buff[0xf] = 0x20;
          /*--*/
          buff[0x10] = 0x10;//12 /*LSB*/
          buff[0x11] = 0x00;
          buff[0x12] = 0x00;
          buff[0x13] = 0x00;
          /*PCM*/
          buff[0x14] = 0x01; /*LSB*/
          buff[0x15] = 0x00;
          /*CH - 1*/
          buff[0x16] = 0x01;
          buff[0x17] = 0x00;
        
          /* already set */
          //rev1.2
#if 0          
          if(ucWMBitrate == 0)
          {
            /*Sample Rate - 6000*/
            buff[0x18] = 0x70;
            buff[0x19] = 0x17;
          }
          else if(ucWMBitrate == 1)
          {
            /*Sample Rate - 7000*/
            buff[0x18] = 0x58;
            buff[0x19] = 0x1B;
          }
          else //if(ucWMBitrate == 2)
          {
            /*Sample Rate - 8000*/
            buff[0x18] = 0x40;
            buff[0x19] = 0x1F;
          }
          
          k = buff[0x19];
          k *= 256;
          k += buff[0x18];
          uiSampleSize = k * 2;
          if(ucWMPacketLen == 0)
            uiSampleSize *= (60 * 5);
          else if(ucWMPacketLen == 1)
            uiSampleSize *= (60 * 10);
          else if(ucWMPacketLen == 2)
            uiSampleSize *= (60 * 30);
          else //if(ucWMPacketLen == 3)
            uiSampleSize *= (60 * 60);
#endif
          /*Sample Rate - 8000  simply in hex */
          buff[0x18] = 0x40;
          buff[0x19] = 0x1F;
          
          /*-*/
          buff[0x1a] = 0x00;
          buff[0x1b] = 0x00;
          /* */
          buff[0x1c] = 0x40;
          buff[0x1d] = 0x1F;
          buff[0x1e] = 0x00;
          buff[0x1f] = 0x00;
          /* sample byte - 2*/
          buff[0x20] = 0x02;
          //buff[0x20] = 0x01;
          buff[0x21] = 0x00;
          /* bit per sample - 16*/
          buff[0x22] = 0x10;
          //buff[0x22] = 0x08;
          buff[0x23] = 0x00;
          /* data */
          buff[0x24] = 0x64;
          buff[0x25] = 0x61;
          buff[0x26] = 0x74;
          buff[0x27] = 0x61;
        
          /* len */
          buff[0x28] = uiSampleSize&0xff;
          buff[0x29] = (uiSampleSize/0x100)&0xff;
          buff[0x2a] = (uiSampleSize/0x10000)&0xff;
          buff[0x2b] = (uiSampleSize/0x1000000)&0xff;
          dataFile.write(buff, 44);
       }
       else {
        Serial.println("Writing header to SD card failed!");
       }
    }


    // switch led and start looping until time is up
    void TMRRecord()
    {
      // start CPU ticks
      unsigned long startListen = millis();
      unsigned long lastSample = micros();
      printf("Recorder() start recording\n");
      SD.remove(fileName);
      
      digitalWrite(LED_PIN, HIGH);
      
      myAudio.startRecording((char*) fileName.c_str(),16000,A0);
      
      size_t bufferPos = 0;
      // int secs = 0;
      while (bufferPos < bufferSize) {
        unsigned long now = micros();
        if (now > lastSample+microsPerSample) {
          lastSample = now;
          // writeSample(analogRead(MIC_PIN));
          bufferPos++;
          //secs++;
        }
        //else {       
        //  if (secs > 10000) {
        //    printf(".");
        //    secs = 0;
        //  }
        //}
      }
      
      myAudio.stopRecording((char*) fileName.c_str());
      digitalWrite(LED_PIN, LOW);
      printf("Recorder() end recording\n");
    }


    // switch led and start looping until time is up
    void Record()
    {
      // start CPU ticks
      unsigned long startListen = millis();
      unsigned long lastSample = micros();
      printf("Recorder() start recording\n");
      SD.remove(fileName);
      dataFile = SD.open(fileName, FILE_WRITE);
      if (!dataFile) {
        printf("Recorder() error opening SD card file\n");
        return;
      }
      
      // we need to add a wav heder here ......
      fillHeader();
      digitalWrite(LED_PIN, HIGH);
      size_t bufferPos = 0;
      // int secs = 0;
      while (bufferPos < bufferSize) {
        unsigned long now = micros();
        if (now > lastSample+microsPerSample) {
          lastSample = now;
          writeSample(analogRead(MIC_PIN));
          bufferPos++;
          //secs++;
        }
        //else {       
        //  if (secs > 10000) {
        //    printf(".");
        //    secs = 0;
        //  }
        //}
      }
      printf("\n");
      
      digitalWrite(LED_PIN, LOW);
      printf("Recorder() file size: %d\n", dataFile.size());
      dataFile.close();
      //dataFile = 0;
      printf("Recorder() end recording\n");
    }

    bool GetRecording(uint16_t& sample)
    {
      if (!dataFile) {
        dataFile = SD.open(fileName, FILE_WRITE);
        if (!dataFile) {
          printf("Recorder() error opening SD card file\n");
        }
      }

      if (dataFile.available()) {
        dataFile.read((uint8_t*) (&sample), 2);
        return true;
      }
      
      dataFile.close();
      // dataFile = 0;
      return false;
    }
    

  private:
    int durationSecs;
    int sampleMs;
    size_t bufferSize;
    unsigned long microsPerSample;
    File dataFile;
};


//
//   webserver stuff 
//

const char *APssid = "AxRECORDER";
const char *APpassword = "";

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

//ESP8266WebServer* server;
ESP8266WebServer server(80);

int dauer = 10;
int frequenz = 8000; // in Hz
bool recording = false;

// global wep page for credentials

String makeHTMLPage()
{

    String theValue = String("<!DOCTYPE html><html><style>p {text-align: center;}</style>")
                    + String("<body><p><h2>Ax-Recorder<h2><h3> Aufnahme:</h3>")
                    + String("<form action='/action_page'>")
                    + String("<br>Dauer:<input type='text' name='dauer' value='") + dauer + String("'><br>")
                    + String("<br>Frequenz:<input type='text' name='frequenz' value='") + frequenz + String("'><br><br>")
                    // + String("ClockID:<br><input type='text' name='ClockID' value='") + wifiData->getWifiBrand() + String("'><br><br>")
                    + String("<br><input type='submit' value='Start'></form>")
                    //
                    // + String("<br><br><a href=\"/recording\">Aufnahme Laden</a><br>") // yes will call this in start ... but now for debugging ...
                    + String("<form action='/recording'>")
                    // + String("<br>Dauer:<input type='text' name='dauer' value='") + dauer + String("'><br>")
                    // + String("<br>Frequenz:<input type='text' name='frequenz' value='") + frequenz + String("'><br><br>")
                    // + String("ClockID:<br><input type='text' name='ClockID' value='") + wifiData->getWifiBrand() + String("'><br><br>")
                    + String("<br><input type='submit' value='Download'></form>")
                    //
                    + String("</p></body></html>"); 
    return theValue;
}


/*
server.setContentLength(CONTENT_LENGTH_UNKNOWN);
server.sendHeader("Content-Length", (String)fileSize);
server.send(200, "text/html", "");```
server.sendContent_P( <contentArr> , <size> );.
*/

void handleRoot() {
 Serial.println("You called root page");
 String s = makeHTMLPage();
 server.send(200, "text/html", s); //Send web page
}


void streamRecording() {
  digitalWrite(LED_GREEN_PIN, HIGH);

  File dataFile = SD.open(fileName, FILE_READ);
  printf("returning %s %d \n", fileName.c_str(), dataFile.size());
  
  server.streamFile(dataFile, "audio/wav");
  dataFile.close();

  printf("done\n");
  digitalWrite(LED_GREEN_PIN, LOW);
}


void handleRecording() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  File dataFile = SD.open(fileName, FILE_READ);

  printf("returning %s %d \n", fileName.c_str(), dataFile.size());
  
  //server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.setContentLength(dataFile.size());
  //server.sendHeader("Content-Length: ", String(dataFile.size()));
  server.send(200, "audio/wav", "");
  char sendBuffer[513];
  size_t filePos = 0;
  size_t i = 0;
  while (filePos < dataFile.size()) {
    sendBuffer[i] = dataFile.read();
    sendBuffer[i+1] = '\0';
    i++; 
    filePos++;
    if (i>511) {
      printf("send loop pos %d\n", filePos);
      server.sendContent(sendBuffer,512);
#if 0      
      String myStr(sendBuffer);
      server.sendContent(myStr);
#endif      
      i=0;
    }
  }
  printf("final loop reaining %d\n", i);
  server.sendContent(sendBuffer,i);
#if 0  
  String myStr(sendBuffer);
  server.sendContent(myStr);
#endif  
#if 0      
    char myByte = dataFile.read();
    String myStr(myByte);
    server.sendContent(myStr);
#endif    

  dataFile.close();
  printf("done\n");
  digitalWrite(LED_GREEN_PIN, LOW);
}


void handleForm() {
 String myDauer = server.arg("dauer"); 
 String myFrequenz = server.arg("frequenz"); 

 Serial.print("dauer:");
 Serial.println(myDauer);

 Serial.print("frequenz:");
 Serial.println(myFrequenz);

 dauer = myDauer.toInt();
 frequenz = myFrequenz.toInt();

 String s = makeHTMLPage();
 server.send(200, "text/html", s); //Send web page
 recording = true; // off we go
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("startup ....");
  delay(1000);
  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);

  Serial.println(WiFi.softAP(APssid,APpassword) ? "AP Ready" : "AP Failed!");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

 if (!SD.begin(chipSelect)) {
   Serial.println("Initialization SD card failed!");
 }
 else {
   Serial.println("SD card mounted");
 }
 // the lib directly records to the card 
 myAudio.CSPin = chipSelect;

    
}

void loop() {
  // start Wifi+Webserver
  // server = new ESP8266WebServer(80);
  
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  //server.on("/recording", handleRecording); 
  server.on("/recording", streamRecording); 
  server.on("/action_page", handleForm);
  server.begin();
  
  Serial.println("webserver started");


  Serial.println(".... startup done");
  int counter = 0;
  // recording = true;
  while (true) {

    if (recording) {
      // 
      Recorder aufnahme(dauer, frequenz);
      aufnahme.Record(); // will return after recording duration and create a file
      recording = false;
    }
    else {
      if (counter > 100) {
        digitalWrite(LED_GREEN_PIN, HIGH);
        Serial.println(".");
        counter = 0;
      }
      delay(100); // just avoid busy wait
      server.handleClient();
      digitalWrite(LED_GREEN_PIN, LOW);
//      Serial.print(".");
//      uint16_t sample = analogRead(MIC_PIN);
//      Serial.print(sample);
      counter++;
    }     
  }
  

}
  

  
