// Do not forget to include mosquitto.h in main program
// define callback function 
void funcallback(char* topic, byte* payload, unsigned int payloadLength);
WiFiClient wifiClient;

PubSubClient clienteMQTT(server, 1883, funcallback, wifiClient);
/* function to connect to WiFi AP, there are to possible networks; if 
 you are only going to use one, use same valles for ssid1 and ssid2
 in mosquitto.h */

boolean wifiConnect() {
int i=0,j=0;  
  ssid=ssid1;
  password=password1;
 
  DPRINT("Connecting to WiFi  "); DPRINTLN(ssid);  
  WiFi.mode(WIFI_STA);  //The 8266 is a station, not an AP 
  WiFi.disconnect();
  WiFi.begin(ssid,password);
 
  while ((WiFi.status() != WL_CONNECTED )) {
    espera(500);
    DPRINT(i++);
    DPRINT(".");   
    if (i>120) { 
      if (ssid==ssid1){
        ssid=ssid2;
        password=password2;
      } else {
        ssid=ssid1;
        password=password1;
      }
      i=0;
      j++;
      if (j>4) { return false;} /* none of Wifi work */
     
      DPRINTLN();
      DPRINT("Try with other network ");DPRINTLN(j);      
      DPRINT("I will try to connect to "); DPRINTLN(ssid);
      WiFi.disconnect();
      espera(1000);
      WiFi.begin(ssid,password);      
    }
  }
 
 DPRINTLN(ssid);  DPRINT("*******Conected; ADDR= ");
 DPRINTLN(WiFi.localIP());
 
 return true;
}

/* No connectivity; will retry */
void sinConectividad(){
int j=0;

  clienteMQTT.disconnect(); 
  espera(500);
  while(!wifiConnect()) {   
  DPRINT("No connectivity Wait for secs  ");DPRINTLN(int(intervaloConex/2000));
  espera(ESPERA_NOCONEX);
  }
}

/****************************************
 * connect to MQTT broker               *
 * it requires to connect to Wifi first *
 ***************************************/
void mqttConnect() {
 int j=0;
 
  if ((WiFi.status() == WL_CONNECTED )) {
   while (!clienteMQTT.connect(clientId, authMethod, token)) {      
     DPRINT(j);DPRINTLN("  I will retry connecting MQTT client  ");
     j++;
     espera(2000);
     if (j>20) {
       sinConectividad();  
       j=0;
      }
     }
   } else {
    sinConectividad();   
  } 
}

boolean loopMQTT() {
return clienteMQTT.loop();
}

/*************************************************************************
 * initialize the device, subscribing to 
 * some actions; in this case reboot(noexplanation),
 **************************************************************************/
void initManagedDevice() {
 int rReboot; 

 rReboot=  clienteMQTT.subscribe(rebootTopic,1);
  DPRINT("\tSubscribe to Reboot= ");DPRINT(rReboot);
 }

/* and here are the funtions invoked depending on the topic */
void funcallback(char* topic, byte* payload, unsigned int payloadLength) {
 DPRINT("funcallback invoked for topic: "); DPRINTLN(topic);
 if (strcmp (rebootTopic, topic) == 0) {
   DPRINTLN("Rebooting...");    
   ESP.restart(); // Not always works due to an issue in ESP12
 }
}

/********** Send data to broker **************/

boolean enviaDatos(char * topic, char * datosJSON) {
  int k=0;
  boolean pubresult=false;  
  
 while (!clienteMQTT.loop() & k<20 ) {
    DPRINTLN("Device ws disconnected, reconnecting ");   
    mqttConnect();
    initManagedDevice();
    k++; 
  } 
  pubresult = clienteMQTT.publish(topic,datosJSON);
  DPRINT("Sending ");DPRINT(datosJson);
  DPRINT("to ");DPRINTLN(publishTopic);
  if (pubresult) 
    DPRINTLN("... OK Success");      
  else
    DPRINTLN(".....KO Failure");
  return pubresult;    
}

/* Wait loop  but mantaining internal routines */
void espera(unsigned long tEspera) {
  uint32_t principio = millis();
  
  while ((millis()-principio)<tEspera) {
    yield();
    delay(500);
  }
}

