#ifndef H_CONFIG_WIFIGETTER
#define H_CONFIG_WIFIGETTER
//
//   configuration local http getter module
//     to be included and called by setup + loop
//


// for time init
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset 



class WifiGetter
{

  public:

    WifiGetter(String newSid, String newPassword, String redirectHostIn, int redirectPortIn, String redirectPageIn, String redirectSecretIn) {
   
      ssid = newSid;
      password = newPassword;
      if ((ssid == "") || (password.length() < 8)) {
        printf("illegal ssid <%s> or passowrd (min 8 chars) <%s> \n", ssid.c_str(), password.c_str());
       
      }
      
      redirectHost = redirectHostIn;
      redirectPort = redirectPortIn;
      redirectPage = redirectPageIn;
      redirectSecret = redirectSecretIn;
      
    }
   
   
    // simple call to extract all between two xml tags
    //   does not really need an instance
    String parseHtml(String reply, String tag, String def) {
      String tagStart = String("<"+tag+">");
      String tagEnd = String("</"+tag+">");
      String value = def;
      int posStart = reply.indexOf(tagStart);
      int posEnd = reply.indexOf(tagEnd);
      //printf("check %d tag: %s %d-%d len: %d\n",reply.laength(), tag, posStart, posEnd, tagStart.length());
  
      if ((posStart > -1) && ((posStart + tagStart.length()) < posEnd) && (posEnd < reply.length())) {
        value = reply.substring(posStart + tagStart.length(), posEnd);
        //printf("Tag: %s Val: #%s#\n",tag,value);
      }
      return value;
  
    }


    //
    //   execute the passed request and return the reply
    //     bool false on error

    bool sendHttpGetRequest(String htmlRequest, String& htmlReply, bool refreshRedirect) {
    
      connect();
    
      if (refreshRedirect) {
        if (!getIPViaRedirectHost()) {
	  return false;
	}
      } else {
         printf("no refresh\n");
      }
    
      WiFiClientSecure client;
      client.setInsecure(); // we have insecure certs here (own certs so server validation will fail)
    
      printf("try connect to: %s\n",host.c_str());
      if (!client.connect(host.c_str(), port)) {
        String message = String("connection failed to ") + host + String(" : ") + String(port) + String(" failed");
	Serial.println(message.c_str());
        return false;
      }

      
      String realRequest = String("GET ") + path + htmlRequest;
      realRequest.replace("//","/");
      realRequest.replace("//","/");
      printf("http getter : '%s'", realRequest.c_str());
      client.print(realRequest);
      
      delay(5000);
      String line;
      char nextChar = 0;
      //printf("setGlobals: html read: %s\n", getTimestring());
      while(client.available() && (nextChar < 254)){
        nextChar = client.read();
          line += String(nextChar);
	delay(1);
      }

      client.stop();
      delete wifiData;
 
      // cleanup html quotes
      // TODO: use a real quoting function
      line.replace("%3A",":");
      line.replace("%21","!");
      line.replace("%20"," ");
      line.replace("%24","$");
      line.replace("%2E",".");
      line.replace("%2B","+");
      line.replace("%2c",",");
      line.replace("%2D","-");
      line.replace("%3C","<");
      line.replace("%3D","=");
      line.replace("%3E",">");
      line.replace("%40","@");
      line.replace("%5F","_");
      line.replace("%25","%");
 
 
      htmlReply = line;
      return true;

    }

    String GetRealIP() {
       String ipStr = ip.toString();
       // it takes some time to fill this struct
       // just do a wait loop
       if (ipStr == String("(IP unset)")) {
          printf("wait and retry to get IP\n");
	      connect();
	      delay (5000);
	      ipStr = ip.toString();
       }
       return ip.toString();
    }



  private:
  
    //
    //    connect to the WIFI
    //
  
    bool connect() {
    
      if (WiFi.status() == WL_CONNECTED) {
        return true;
      }
      WiFi.disconnect();
      delay (1000); // this is async and give the router some time
      
      printf("Wifi init\n");
      size_t wait = 0;  
      WiFi.begin(ssid.c_str(), password.c_str());
      while ((WiFi.status() != WL_CONNECTED) && (wait < 30)) {
        wait++;
        //printf("connecting: %d \n", wait);
        delay(500);
      }
      if (WiFi.status() == WL_CONNECTED) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        ip = WiFi.localIP();
        Serial.print("IP Address: ");
        Serial.println(ip);
    
        // printf("time: %s \n", getTimestring(0));
    
        return false;
      }
      printf("failed: %d \n", WiFi.status());
      return true;
    }

  
    //
    //   ask the redirect host for the real IP to connect to
    //   returns false on error
  
    bool getIPViaRedirectHost() {
    
      // I think we can omit this in later builds
      // TODO: delete       
      if (ssid == "noldor") {
        host = "192.168.93.13";
	    port = 443;
	    path = "";
        return true;
      }
  
      WiFiClientSecure redirectClient;
      redirectClient.setInsecure(); // we have no 1:1 root certs here (many domains for one IP)
      String myHost = redirectHost;
      String mySecret = redirectSecret;
      printf("redirect host: %s\n",myHost.c_str());
      if (!redirectClient.connect(myHost.c_str(), redirectPort)) {
        String message = String("redirect connection to ") + myHost + String(" : ") + String(redirectPort) + String(" failed");
	    Serial.println(message.c_str());
        return false;
      }
  
      // This will send the request to the server
      // and use the redirect secret
      String req = String("GET /") + String(redirectPage) + String(" HTTP/1.1\r\n") 
                   + String("Host: ") + myHost + String("\r\n")
                  // + String("Authorization: Basic ") + mySecret + String("\r\n")
                   + String("Connection: close\r\n\r\n");
      
      // send it
      // attention the webserver is very allergic against double slashes (//)
      req.replace("//","/");
      req.replace("//","/");
      
      printf("redirect request: %s\n",req.c_str());

      redirectClient.print(req);

      delay(2000);
      bool inRead = true;
      String line;
      char nextChar = 0;
      int loops = 0;
      //printf("html read: ");
      //Serial.println(client.status());
      //printf("\n");
      
      while(inRead){
        if (redirectClient.available()) {
	      String lineChunk = redirectClient.readStringUntil('\n');
	      line += lineChunk;
	    }
	    else {
	      // wait a little and see what happens
	      delay (10);
	      loops++;
	      if (loops > 10000) {
	        // 10 secs is max
	        inRead = false;
	        printf("waiting a little \n");
	      }
	    }
	    // maybe the document has been read by now  
	    if (line.indexOf("</html>") != -1) {
	      inRead = false;
	      printf("redirect has been read \n");
	    }
      }
      
      
      printf("redirect reply: %s\n",line.c_str());
  
      redirectClient.stop();
  
      String newIP;
      //printf("\n");
      if (line.length() > 100) {
        // make this configurable .... naaa just create an own redirect page      
        String myUrl = parseHtml(line,String("aixurl"),redirectHost + String(redirectPort));
        int colon = myUrl.indexOf(":");
        int slash = myUrl.indexOf("/");
        String myHost = myUrl;  // default, only new IP/name
        String myPort = "443";  // default https
        String myPath = "";     // default ""
        
        // no port but a path
        if ((colon == -1) && (slash != -1)) {
            myHost = myUrl.substring(0,slash);
            myPath = myUrl.substring(slash);
        }
        else 
        // no path but colon
        if ((colon != -1) && (slash == -1)) {
            myHost = myUrl.substring(0,colon);
            myPort = myUrl.substring(colon+1);
        }
        else {
        // all given
            myHost = myUrl.substring(0,colon);
            myPort = myUrl.substring(colon+1,slash);        
            myPath = myUrl.substring(slash);        
        }
        
        printf("redirect extraction: host:'%s' port:'%s', path :'%s'\n",myHost.c_str(), myPort.c_str(), myPath.c_str());
        
	    if ((atoi(myPort.c_str()) > 40) && (myHost.length() > 8)) {
	        port = atoi(myPort.c_str());
	        host = myHost;
            path = myPath;            
	        return true;
	    }
         
      }
      Serial.println("redirect getting IP failed");
      return false;
    }

    //
    //
    //
    
    String ssid;
    String password;
    // forwarder address
    // format "https://<host>:<port>/<path/......"
    String host;
    String path;
    int port;
    IPAddress ip;
    
    String redirectHost;
    int redirectPort;
    String redirectPage;
    String redirectSecret;

};

#endif
