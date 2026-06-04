#ifndef H_CONFIG_WEBSERVER
#define H_CONFIG_WEBSERVER

//
//   configuration web module
//     to be included and called by setup + loop
//

#ifdef ESP_32
#include <WiFi.h>
#else
#include <ESP8266WebServer.h>
#endif

#include <WiFiClientSecure.h>

#ifdef ESP_32
#include <WebServer.h>
#else
#include <ESP8266WiFi.h>
#endif

// the one global instance of the webserver
// has to be global and static for callback handling
#ifdef ESP_32
static WebServer*        server;
#else
static ESP8266WebServer*        server;
#endif
static ConfigParams*     configData;
static String            hardwareDevice;
static String            deviceType;

// -----------------------------


class WifiConfigWebserver {


  public: 
    WifiConfigWebserver (ConfigParams* inConfigData, String newHwDevice, String newDeviceType)
    {
      printf("WifiConfigWebserver: v 2.1\n");
      configData = inConfigData;
      hardwareDevice = newHwDevice;
      deviceType = newDeviceType;
    }
        
    // really start the acess point and give over the handling
    // this will not return
    void runAcessPoint ()
    {
      Serial.print("WIFI name: ");
      Serial.println("AIX-Gadged passw: 12345678");
      WiFi.mode(WIFI_AP);
      // WiFi.softAP(APssid, APpassword);
      // need at least8 chars in PW !!!
      boolean result = WiFi.softAP("AIX-Gadged (PW:12345678)", "12345678");
    
      // Serial.println(WiFi.softAP("aix","aix") ? "AP Ready" : "AP Failed!");
      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());
    #if 0
      // some debug BS ..... 
      while (true) {
        delay(5000); // just avoid busy wait
        Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
      }
    #endif
      
      handleConfig();
      
    }
     

  private:

    // just really create and start the ap
    void handleConfig()
    {
#ifdef ESP_32
      server = new WebServer(80);
#else
      server = new ESP8266WebServer(80);
#endif
    
      server->begin();
      
      server->on("/", handleRoot);      // the routine to handle at root location  (fallback for all)
      //server->on("/config", handleNewConf); //as Per  <a href="ledOn">, Subroutine to be called
      server->on("/action_page", this->handleForm);
      //server->on("/restart", this->handleRestart);
    
      while (true) {
        delay(100); // just avoid busy wait
        server->handleClient();
        
        // Serial.println("http read loop");
      }
    }

    //
    //
    //
    

    static String nextInputLine(ConfigParams* configData, const String& name) {
        String display;
	configData->GetDisplay(name, display);
        String value;
	configData->GetValue(name, value);
	
        String line = display
	              + String(":<br><input type='text' name='") + name
		      + String("' value='") + value
		      + String("'><br><br>");
        return line;
    }
    
    static String nextLine(ConfigParams* configData, const String& name) {
        String display;
	configData->GetDisplay(name, display);
        String value;
	configData->GetValue(name, value);
	
        String line = display
	              + String(": ") + value
		      + String("<br><br>");
        return line;
    }



    //
    //  this is the one page returned to the browser
    //   
    //
    static String makeHTMLPage(bool reply)
    {
    
    
        // there are nicer ways to define this ....
        // I wanted to avoid having to embed a template, well this is not better but it gets the job done
        String theValue;
        if (!reply) {
          theValue = String("<!DOCTYPE html><html><head><title>AIX-Gadged config page</title></head><body><h2>AIX-Gadged:<br>Type: ") + deviceType + String("<br>HW-ID: ") + hardwareDevice + String("</h2><br><h3>Settings and Wifi Credentials</h3>")
                        + String("<form action='/action_page'>");



          if (configData) {
	          String name = configData->GetNextParamValue("");
			
	          while (name != "") {
	              theValue += nextInputLine(configData, name);
	              name = configData->GetNextParamValue(name); 
	          }
					
          }          
          theValue +=     String("<input type='submit' value='Submit'></form>")
                        + String("</body></html>"); 
        }
        else {
          theValue = String("<!DOCTYPE html><html><head><title>AIX-Gadged config page</title></head><body><h2>AIX-Gadged:<br>Type: ") + deviceType + String(", W-ID-") + hardwareDevice + String("<h2><h3>Settings and  Wifi Credentials</h3>")
                        + String("<b>Saved values<b><br>");
			
	  String name = configData->GetNextParamValue("");
	  while (name != "") {
	      theValue += nextInputLine(configData, name);
	      name = configData->GetNextParamValue(name); 
	  }
            
          theValue +=     String("<a href='/'>Edit Mode</a><br><br>")
                        + String("</body></html>"); 
        }
        return theValue;
    }
    

    static void handleRoot() {
     Serial.println("You called root page");
     String s = makeHTMLPage(false);
     server->send(200, "text/html", s); //Send web page 
    }
    
    static void handleRestart() {
     Serial.println("Restarting by web request");
     ESP.restart();     
    }
    
    
    static void handleForm() {
    
        Serial.println("You posted a config");
     
        String name = configData->GetNextParamValue("");
	while (name != "") {
	    String value = server->arg(name); 
            Serial.print(name);
            Serial.print(": ");
            Serial.println(value);
	    configData->SetValue(name, value);
	    name = configData->GetNextParamValue(name);
	}
        
     String s = makeHTMLPage(true);
     server->send(200, "text/html", s); //Send web page
    }


};


#endif
