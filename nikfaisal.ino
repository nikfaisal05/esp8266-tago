//nikfaisal05@gmail.com
//This code is for liquid level control. Device used is espresso lite 2.0.
//It should work in any other esp8266

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char* ssid = "<ssid>";
const char* password = "<password>";

String apiKey = "<tago device key>";
String data= "";
const char* server = "api.tago.io";

int level = 0;
String callbutton;
String alarm;
char buffer[256];
float avgcount = 0.0;
int y=0;
WiFiClient client;

long previousMillis = 0;      
long redLedInterval = 1000;  
unsigned long currentMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.print("Connecting to WiFi : ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void pub_iot(void){
    String postdata = buffer;  //Data sent need to be json format; in format variables: "what variable" value: "value of the variable"
    String token = String("Device-Token: ")+ String(apiKey);
    Serial.println(buffer);
  if (client.connect(server,80)) {
    digitalWrite(LED_BUILTIN,LOW);
    delay(500);
    digitalWrite(LED_BUILTIN,HIGH);

    Serial.print("CONNECTED AT TAGO\n");
    client.println("POST /data? HTTP/1.1");
    client.println("Host: api.tago.io");
    client.println("_ssl: false");
    client.println(token);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(postdata.length());
    client.println();
    client.println(postdata);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

  }
  client.stop();

}

void loop() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  currentMillis = millis();
  avgcount = 0;
  for(int i=0; i<200;i++){
    avgcount += analogRead(A0);
  }
  level = (int)(avgcount/200);
  level = map(level,0, 1024, 0, 100);
  if(digitalRead(0)) {
    callbutton = "Patient is calling";
  }
  else{
    callbutton = "Call button inactive";
  }

  if(digitalRead(2)) {
    alarm = "level is low.";
  }
  else {
    alarm = "level is OK.";
  }
  y=1;
  if (y==0){
  root["variable"] = "level";   
  root["value"] = String(level);
  y++;
  }else if (y==1){
  root["variable"] = "call";
  root["value"] = callbutton;
  y++;
  } else if (y==2){
  root["variable"] = "alarm";
  root["value"] = alarm;
  y=0;
  }

 
  root.printTo(buffer, sizeof(buffer));
  
  Serial.print(callbutton);
  Serial.print("  ");
  Serial.print(alarm);   
  Serial.print("  ");
  Serial.println(String(level)); 
    
  if (currentMillis - previousMillis > redLedInterval) {
    previousMillis = currentMillis;
    pub_iot();
  }
}
