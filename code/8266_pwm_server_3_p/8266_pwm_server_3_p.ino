
/*
 * Non professional ventilator software 
 * Use it at your own risk
 * I decline any responsability
 * 
 * Davide Dondi
 * 030420
 */





#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"

const int pwmPin=4; //D2

//=============================   CUSTOMIZED  PARAMETERS    ==========================================
const char *device_name = "VENTILATOR_4";  // VENTILATOR_x number           -- change it if you want
const char *password = "v4v4v4v4";         // v4v4v4v4     access password  -- change it if you want
//====================================================================================================



const char *ssid = device_name;     

int pushPwm = 200;

unsigned long startMillis;
unsigned long currentMillis;
unsigned long t_stop=1000;
unsigned long t_push=1000;

bool f=1;


ESP8266WebServer server ( 80 );


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(pwmPin,0);
  
  SPIFFS.begin(); 
  
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  WiFi.softAP(ssid, password);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  server.on ( "/", mainhtm );
  server.on("/STATUS", HTTP_POST, handleSTATUS); 

  server.begin();
  Serial.println ( "HTTP server started" ); 

  reader();

  startMillis = millis();
    
}  //  setup end



void mainhtm (){
  File fn =SPIFFS.open("/main.htm","r");
  Serial.println(fn);
  server.streamFile(fn,"text/html");
  fn.close();  
  }


void handleSTATUS() { 
    
    String message="";
    
   Serial.println("Numero coppie : " + (String)(server.args()));
    
    
    for (int i = 0; i < server.args(); i++) {
      
      message += "Arg nº" + (String)i + " –> ";
      message += server.argName(i) + ": ";
      message += server.arg(i) + "\n";
       
      Serial.println(message); 

      if(server.argName(i)=="push_p"){
        Serial.println(server.arg(i));
        pushPwm =(server.arg(i).toInt()); 
      }

      if(server.argName(i)=="push_t"){
        Serial.println(server.arg(i));
        t_push =(server.arg(i).toInt());
      }


      if(server.argName(i)=="stop_t"){
        Serial.println(server.arg(i));
        t_stop =(server.arg(i).toInt());
      }


       if(server.argName(i)=="load"){
        Serial.println("Ricevuto comando load");
        reader();
      }


       if(server.argName(i)=="store"){
        Serial.println("Ricevuto comando store");
        writer();
      }  


       if(server.argName(i)=="update"){
        Serial.println("Ricevuto comando update");
       //viene inviata una risposta con lo stato attuale
      }
          
      message="";
      }
    
    String rispo = String(pushPwm) + "£" + String(t_push) + "£" + String(t_stop) + "£" + device_name;
    server.send(200, "text/plain",rispo);     // ritorno  stringa con valori aggiornati variabili
} 



void reader(){  
    File f=SPIFFS.open("/stored.txt","r");
    if(!f) {
      Serial.println("File open failed");
    } else {    
        Serial.println("-----  Reading stored.txt  file ------");
        String s[3];  //service
        for (int i=0; i<3; i++) {
          s[i] = f.readStringUntil('\n');
          Serial.print(i);
          Serial.print(" : ");
          Serial.println(s[i]);
        }  
        pushPwm = s[0].toInt();
        t_push  = s[1].toInt();
        t_stop  = s[2].toInt();

        Serial.println("pushPwm = " + s[0]);
        Serial.println("t_push  = " + s[1]);
        Serial.println("t_stop  = " + s[2]);
          
      }  
      f.close();
  }



void writer(){  
    File f=SPIFFS.open("/stored.txt","w");
    if(!f) {
      Serial.println("File open failed");
    } else {    
        Serial.println("-----  Writing stored.txt  file ------");
        f.println(String(pushPwm));
        f.println(String(t_push));
        f.println(String(t_stop));     
      }  
      f.close();
  }



void loop() { 

  currentMillis = millis();

  if(t_stop){  

      if( f & ((currentMillis - startMillis)) >= t_push){
        f=0;
        startMillis= millis();
        digitalWrite(LED_BUILTIN, HIGH);
        analogWrite(pwmPin,0);
      }
   
    
      if( !f & ((currentMillis - startMillis)) >= t_stop){
        f=1;
        startMillis= millis();
        digitalWrite(LED_BUILTIN, LOW);
        analogWrite(pwmPin,pushPwm);
      }
      
  } else {
    analogWrite(pwmPin,0);
    digitalWrite(LED_BUILTIN, HIGH);
    }
  
         
  server.handleClient();
 
  }
