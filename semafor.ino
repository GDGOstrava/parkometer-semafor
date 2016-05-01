/*------------------------------------------------------------------------
  Simple ESP8266 test.  Requires SoftwareSerial and an ESP8266 that's been
  flashed with recent 'AT' firmware operating at 9600 baud.  Only tested
  w/Adafruit-programmed modules: https://www.adafruit.com/product/2282

  The ESP8266 is a 3.3V device.  Safe operation with 5V devices (most
  Arduino boards) requires a logic-level shifter for TX and RX signals.
  ------------------------------------------------------------------------*/

#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>
#include "conn_conf.h"

#define ESP_RX   2
#define ESP_TX   3
#define ESP_RST  4
SoftwareSerial softser(ESP_RX, ESP_TX);

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);
// Must call begin() on the stream(s) before using Adafruit_ESP8266 object.



#define LED_OBSAZENO  13
#define LED_VOLNO     12
#define LED_WIFI_CONN 11
#define LED_TCP_CONN  10


unsigned long previousMillis = 0;

void setup() {
  pinMode(LED_VOLNO,OUTPUT);
  pinMode(LED_WIFI_CONN,OUTPUT);
  pinMode(LED_TCP_CONN,OUTPUT);
  
  //dvoubarevna LED, spolecna anoda - zapojena proti +5V, pri HIGH nesviti, pri LOW sviti
  digitalWrite(LED_OBSAZENO, HIGH);
  digitalWrite(LED_VOLNO, HIGH);

  //connection status LOW - connected, HIGH - disconnected
  digitalWrite(LED_WIFI_CONN, HIGH);
  digitalWrite(LED_TCP_CONN, HIGH);
  
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

  Serial.print(F("Soft reset..."));
  if(!wifi.softReset()) {
    Serial.println(F("no response from module."));
    for(;;);
  }
  Serial.println(F("OK."));

}

void loop() {
  char buffer[50];
  String x="";
  char data;
  
  Serial.print(F("Connecting to WiFi..."));
  if(wifi.connectToAP(F(ESP_SSID), F(ESP_PASS))) {
    digitalWrite(LED_WIFI_CONN, LOW);
    while(true){
        // IP addr check isn't part of library yet, but
        // we can manually request and place in a string.
        Serial.print(F("OK\nChecking IP addr..."));
        wifi.println(F("AT+CIFSR"));
        if(wifi.readLine(buffer, sizeof(buffer))) {
          Serial.println(buffer);
          wifi.find(); // Discard the 'OK' that follows
    
          Serial.print(F("Connecting to host..."));
          if(wifi.connectTCP(F(HOST), PORT)) {
            digitalWrite(LED_TCP_CONN, LOW);
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
                  Serial.println(F("ZELENA"));
                  digitalWrite(LED_OBSAZENO, HIGH); 
                  digitalWrite(LED_VOLNO, LOW); 
                  
                }
                if (x.indexOf("obsazeno")>=0){
                  Serial.println(F("CERVENA"));
                  digitalWrite(LED_OBSAZENO, LOW);
                  digitalWrite(LED_VOLNO, HIGH);
                   
                }
                data=(char)softser.read();
                  
              }
              if(millis()>previousMillis+2000){
                previousMillis=millis();
                wifi.println(F("AT+CIPSTATUS"));
                if(!wifi.find(F("STATUS:3"))){
                  break;
                }
              }
             
            }
            wifi.closeTCP();
            digitalWrite(LED_OBSAZENO, HIGH);
            digitalWrite(LED_VOLNO, HIGH);
            digitalWrite(LED_TCP_CONN, HIGH);
          } else { // TCP connect failed
            Serial.println(F("D'oh!"));
          }
        } else { // IP addr check failed
          Serial.println(F("error"));
        }
        if(millis()>previousMillis+10000){
            previousMillis=millis();
            wifi.println(F("AT+CWJAP?"));
            if(!wifi.find(F(ESP_SSID))){
              break;
            }
        }
    }
    wifi.closeAP();
    digitalWrite(LED_WIFI_CONN, HIGH);
  } else { // WiFi connection failed
    Serial.println(F("FAIL"));
      // Test if module is ready
    Serial.print(F("Hard reset..."));
    if(!wifi.hardReset()) {
      Serial.println(F("no response from module."));
      for(;;);
    }
    Serial.println(F("OK."));
  
    Serial.print(F("Soft reset..."));
    if(!wifi.softReset()) {
      Serial.println(F("no response from module."));
      for(;;);
    }
    Serial.println(F("OK."));
  }
  delay(2000);
}
