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

    WifiGetter(String newSid, String newPassword, String redirectHostIn, int redirectPortIn, String redirectPageIn) {
   
      ssid = newSid;
      password = newPassword;
      if ((ssid == "") || (password.length() < 8)) {
        printf("illegal ssid <%s> or passowrd (min 8 chars) <%s> \n", ssid.c_str(), password.c_str());
       
      }
      
      redirectHost = redirectHostIn;
      redirectPort = redirectPortIn;
      redirectPage = redirectPageIn;
      
    }
   
   
    // simple call to extract all between two http tags
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

    bool sendHttpRequest(String htmlRequest, String& htmlReply, bool refreshAbkra) {
    
      connect();
    
      if (refreshAbkra) {
        if (!getIPViaAbkra()) {
	  return false;
	}
      } else {
         printf("no refresh\n");
      }
    
      WiFiClientSecure client;
      client.setInsecure(); // we have insecure certs here
    
      printf("try connect to: %s\n",host.c_str());
      if (!client.connect(host.c_str(), port)) {
        String message = String("connection failed to ") + host + String(" : ") + String(port) + String(" failed");
	Serial.println(message.c_str());
        return false;
      }

      client.print(htmlRequest);
      
      delay(5000);
      String line;
      char nextChar = 0;
      //printf("setGlobals: html read: %s\n", getTimestring());
      // while(client.available() && (nextChar < 254)){
      while(client.available() && (nextChar < 254)){
        nextChar = client.read();
        //if (nextChar < 254) {
          //printf("%c",nextChar); 
          line += String(nextChar);
        //}
	delay(1);
      }

      client.stop();
      delete wifiData;
 
      // cleanup html quotes
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
       if (ipStr == String("(IP unset)")) {
          printf("wait and retry to get IP\n");
	  connect();
	  delay (5000);
	  ipStr = ip.toString();
       }
       // return WiFi.localIP().toString();
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
    //   ask abkra.de for the real IP to connect to
    //   returns false on error
  
    bool getIPViaAbkra() {
      // I think we can omit this in later builds
      
      if (ssid == "noldor") {
        host = "192.168.93.13";
	port = 443;
        //newHost = host;
        //newPort = port;
        return true;
      }
  
      WiFiClientSecure abkraClient;
      abkraClient.setInsecure(); // we have no 1:1 root certs here (many domains for one IP)
      String myHost = redirectHost;
      printf("abkra myhost: %s\n",myHost.c_str());
      if (!abkraClient.connect(myHost.c_str(), redirectPort)) {
        String message = String("abkra connection to ") + myHost + String(" : ") + String(redirectPort) + String(" failed");
	Serial.println(message.c_str());
        return false;
      }
  
      // This will send the request to the server
      //abkraClient.print(String("GET ") + String(redirectPage) + String(" HTTP/1.1\r\nHost: ")+ myHost + String("\r\nConnection: close\r\n\r\n"));
      String req = String("GET ") + String(redirectPage) + String(" HTTP/1.1\r\nHost: ")+ myHost + String("\r\nConnection: close\r\n\r\n");
      abkraClient.print(req);
      

      delay(2000);
      bool inRead = true;
      String line;
      char nextChar = 0;
      int loops = 0;
      //printf("html read: ");
      //Serial.println(client.status());
      //printf("\n");
      // while(abkraClient.available() && (nextChar < 254)){
      while(inRead){
        if (abkraClient.available()) {
	/*
          nextChar = abkraClient.read();
          if (nextChar < 254) {
            //printf("%c",nextChar); 
            line += String(nextChar);
          }
	  */
	  String lineChunk = abkraClient.readStringUntil('\n');
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
	  printf("doc has been read \n");
	}
      }
      
      printf("abkra reply: %s\n",line.c_str());
  
      abkraClient.stop();
  
      String newIP;
      //printf("\n");
      if (line.length() > 100) {
      
        String myHost = parseHtml(line,String("thehost"),String(""));
        String myPort = parseHtml(line,String("theport"),String("443"));
	
	if ((atoi(myPort.c_str()) > 40) && (myHost.length() > 8)) {
	    port = atoi(myPort.c_str());
	    host = myHost;
	    //newHost = host;
	    //newPort = port;
	    return true;
	} 
      
      /*  old code for parsing the href link
        String ref = parseHtml(line,String("td"),"<a href=\"https://88.152.251.180\">Heimat</a>");
        int posStart = ref.indexOf("https");
        int posEnd = ref.indexOf("Heimat");
        if ((posStart > 0) && (posEnd-2 > posStart+8)) {
          newIP = ref.substring(posStart+8,posEnd-2);
          printf("realIP: %s\n",newIP.c_str());
          return newIP;
        }
	*/
      }
      Serial.println("abkra getting IP failed");
      return false;
    }

    //
    //
    //
    
    String ssid;
    String password;
    String host;
    int port;
    IPAddress ip;
    
    String redirectHost;
    int redirectPort;
    String redirectPage;

};

#endif
