# AXAMP ussage

AXAmp is a small tool to read four power lines.
It connects to an existing WiFi network and sends the values to a server vis IP4 http get request.
The values are captured and sent to the server every minute.
It uses an edp32 and standard industy SCT013 sensors.

## Measurement

Each sensor works independently and returns a raw measurement in 1/10 of an ampere (10 = 1A).
The sensors are named amps1,amps2,amps3,amps4.
The accuracy is about 5-10% with a positive offset to 0.
The server needs to clean the raw data stream.

## Required server

The defice calls a setter routine every minute / measurement.

### Setter calls
The tool will execute two calls

to read the final host configuration (only onc and on network errors)
    http-get https://<redirect-url> 
    
To send the data every minute
    http-get: https://<result_from_redirect>/set?device_type=AXAMP&device_id=zweiter&amps1=0.1069&amps2=0.0000&amps3=7.8502&amps4=6.9648


## Configuration
The gadget can be put into config mode by setting the small awitch on the lower part of the platine to the left.
In this mode the device acts as an access point with the SSID "AIX-Gadged passw: 12345678" and the password "12345678".
Connet to this acess point and open a browser with the addess 192.168.0.1

Then the configuration page will be shown.
Note that the order of the parameter is random. (to be fixed in a later version)


### Configuration parameter
This is a list of the parameter to configure before using the gadget.

Device identification:
"WiFi-DeviceID" : Unique Id/Name of this device, it is sent as "device_id" in the data
"WiFi-DeviceType" : Type of this device AXAMP, do not change

Network configuration:
"WiFi-SSID" : SSID of the WiFi to connect to
"WiFi-Passphrase" : Passphrase of the WiFi to connect to (less than 8 chars may cause connection issues)

Command redirect:
The device uses a get command to retrieve the real / final hostname and port that should be used to send the data.
This is done since the server may have an IP not published to the DNS.
The reply is scanned for an xml-tag <axurl>...valie...</axurl>.>
This hostname is used for the "set" call.
When the parsing fails the redirect host is used instead as fallback.

"Redirect-URL" : Full url https://<host>[;<port>]/<http-path> to return a webpage with the xml tag <axurl>.
"Redirect-URL-User" : http header basic auth usename if passed
"Redirect-URL-Passphrase" : http header basic auth password if passed

Data connection:
"URL" : this is the fallback send name/IP url to be used when the redirect resolver failed
"URL-User" : http header basic auth usename if passed, required when "ax-server" is used as backend
"URL-Passphrase" : http header basic auth password if passed, required when "ax-server" is used as backend

Prometheus oush
The device can send the data by oush request.
It uses the "URL" as endpoint. Do not enter anythin in the "redirect-*" config parameters.
The data is sent at text POSt reqest, one value per line.
