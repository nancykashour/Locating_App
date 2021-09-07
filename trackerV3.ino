#include "WiFi.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Dictionary.h>

const char* ssid = "TP-LINK_73EDBA";
const char* password = "20+20=200";
String userId = "6089ada588c80e0015776f16";
String positionValue = "C306";
String serverName = "https://monitor2021.herokuapp.com/";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
unsigned long timerDelay = 10;

String names;
String limValue;
String children[100];
int childrenNumber = 0;
String existNetwork[100];
int RSSIValues[100];
int limits[6];
bool ValidToSend[100];
bool visited[100];


void MakeRequest(String SSID, int RRIS);
String getNames();
void split(String n);
void splitLimits(String str);
String getLimits();
void runBuuzer();

void setup(){
  Serial.begin(57600);
  pinMode(23, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  names = getNames();
  split(names);
  for(int i=0;i<childrenNumber;i++)
    visited[i] = 0;
  limValue = getLimits();
  Serial.println(limValue);
  splitLimits(limValue);
  for(int i=0;i<6;i++)
  Serial.println(limits[i]);
  
  randomSeed(analogRead(33));
}

void loop(){
    //Send an HTTP POST request every 10 seconds
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      
  //Serial.println("Start scan");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("no networks found");
  } else {
      for(int i=0;i<childrenNumber;i++){
        visited[i] = 0;
      ValidToSend[i] = 0;
      }
    //Serial.print(n);
    //Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      int ind = checkName(WiFi.SSID(i));
      //Serial.print("Start check: ");
      //Serial.println(ind);
      if(ind != -1){
        visited[ind] = 1;
        if(WiFi.RSSI(i)!= RSSIValues[ind]) {
          RSSIValues[ind] = WiFi.RSSI(i);
          ValidToSend[ind] = 1;
        } else {
          ValidToSend[ind] = 0;
        }
      }
      /* Serial.print("End check");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ");
      Serial.print(" [");
      Serial.print(WiFi.channel(i));
      Serial.print("] ");
      delay(10);*/
    }
    for(int i=0;i<childrenNumber;i++)
        if(visited[i] == 0){
          if(RSSIValues[i]!= -999)
            ValidToSend[i] = 1;
          RSSIValues[i]=-999;
        }
   for (int i = 0;i < childrenNumber; i++) {
    if(ValidToSend[i]){
      MakeRequest(children[i], RSSIValues[i]);
      /*Serial.println(RSSIValues[i]);
        Serial.println(limits[0]);
          Serial.println(limits[1]);
     Serial.println(RSSIValues[i]>= limits[0]);
       Serial.println(RSSIValues[i]<= limits[1]);*/
    if(RSSIValues[i]>= limits[0] && RSSIValues[i]<= limits[1]){
      runBuuzer();
      
    }
    }
    ValidToSend[i] = 0;
   }
  }
//  Serial.println("Scan done");
//  Serial.println("");
 
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
 
}
/*
 *****************************************************************************************************
 */
void MakeRequest(String SSID, int RSSI){
      HTTPClient http;
      // Your Domain name with URL path or IP address with path
      String serverPath = serverName + "data";
//            Serial.println(serverPath) ;
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "user=6089ada588c80e0015776f16&child="+String(SSID)+"&RSSI="+String(RSSI)+"&node="+positionValue;//"api_key=" + apiKey + "&field1=" + String(random(40)); 
//      Serial.println(httpRequestData) ;         
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();

}
/*
 *****************************************************************************************************
 */
String getNames() {
    String payload = "";
  //Send an HTTP POST request every 10 minutes
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName + "children?user="+userId;;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
 return payload;
}

String getLimits() {
    String payload = "";
  //Send an HTTP POST request every 10 minutes
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName + "setting/node?user="+userId+"&position="+positionValue;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
 return payload;
}
int checkName(String name){
  int found = -1;
  for(int i=0; i < childrenNumber ; i++){
    if(children[i] == name){
      found = i;
      break;
    }
  }
  return found;
}
/*
 *****************************************************************************************************
 */
void split(String str) {
  int Start = 1;
  int End = str.indexOf(',',0);
  while(End != -1){
      String child = str.substring(Start, End);
      children[childrenNumber] = child;
      RSSIValues[childrenNumber] = -999;
      ValidToSend[childrenNumber] = 0;
      childrenNumber += 1;
      Serial.println(child);
      Start = End + 2;
      End = str.indexOf(',',Start);
  }
}
void splitLimits(String str) {
  int Start = 1;
  int End = str.indexOf(',',0);
  int ind = 0;
  while(End != -1){
      String lim = str.substring(Start, End);
      limits[ind] = lim.toInt();
      Start = End + 1;
      End = str.indexOf(',',Start);
      ind++;
  }
}
void runBuuzer(){
  Serial.println("Run Buuzer");
  digitalWrite(23, HIGH);
  delay(5000);
  digitalWrite(23, LOW);
}
