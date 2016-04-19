/*------------------------------------------------------------------------
  Simple ESP8266 test.  Requires SoftwareSerial and an ESP8266 that's been
  flashed with recent 'AT' firmware operating at 9600 baud.  Only tested
  w/Adafruit-programmed modules: https://www.adafruit.com/product/2282

  The ESP8266 is a 3.3V device.  Safe operation with 5V devices (most
  Arduino boards) requires a logic-level shifter for TX and RX signals.
  ------------------------------------------------------------------------*/

#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>

#define ESP_RX   2
#define ESP_TX   3
#define ESP_RST  4
SoftwareSerial softser(ESP_RX, ESP_TX);

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);
// Must call begin() on the stream(s) before using Adafruit_ESP8266 object.

#define ESP_SSID "HUB" // Your network name here
#define ESP_PASS "IFeelGood" // Your network password here



#define HOST     "52.28.200.14"     // Host to contact
#define PORT     9999                     // 80 = HTTP default port

#define LED_PIN  13
unsigned long previousMillis = 0;


void setup() {
  pinMode(12,OUTPUT);
  //dvoubarevna LED, spolecna anoda - zapojena proti +5V, pri HIGH nesviti, pri LOW sviti
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  
  
  
  // comment/replace this if you are using something other than v 0.9.2.4!
  wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));
  
  softser.begin(9600); // Soft serial connection to ESP8266
  Serial.begin(57600); while(!Serial); // UART serial debug



  // Test if module is ready
  Serial.print(F("Hard reset..."));
  if(!wifi.hardReset()) {
    Serial.println(F("no response from module."));
    for(;;);
  }
  Serial.println(F("OK."));

  //Serial.print(F("Soft reset..."));
  //if(!wifi.softReset()) {
  //  Serial.println(F("no response from module."));
  //  for(;;);
  //}
  //Serial.println(F("OK."));

}

void loop() {
  char buffer[50];
  String x="";
  char data;
  
  Serial.print(F("Connecting to WiFi..."));
  if(wifi.connectToAP(F(ESP_SSID), F(ESP_PASS))) {

    // IP addr check isn't part of library yet, but
    // we can manually request and place in a string.
    Serial.print(F("OK\nChecking IP addr..."));
    wifi.println(F("AT+CIFSR"));
    if(wifi.readLine(buffer, sizeof(buffer))) {
      Serial.println(buffer);
      wifi.find(); // Discard the 'OK' that follows

      Serial.print(F("Connecting to host..."));
      if(wifi.connectTCP(F(HOST), PORT)) {
        while(true){
          delay(100);
          
          data=(char)softser.read();
         
          while(data>0){
            if(data=='\n'){
              Serial.print(x);
              x="";
            }
            x+=data;
            
            if (x.indexOf("volno")>=0){
              Serial.println("ZELENA");
              digitalWrite(13, HIGH); 
              digitalWrite(12, LOW); 
              
            }
            if (x.indexOf("obsazeno")>=0){
              Serial.println("CERVENA");
              digitalWrite(13, LOW);
              digitalWrite(12, HIGH);
               
            }
            data=(char)softser.read();
              
          }
          if(millis()>previousMillis+10000){
            previousMillis=millis();
            wifi.println(F("AT+CIPSTATUS"));
            if(!wifi.find(F("STATUS:3"))){
              break;
            }
          }
         
        }
        wifi.closeTCP();
      } else { // TCP connect failed
        Serial.println(F("D'oh!"));
      }
    } else { // IP addr check failed
      Serial.println(F("error"));
    }
    wifi.closeAP();
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
  } else { // WiFi connection failed
    Serial.println(F("FAIL"));
  }
  delay(10000);
}
