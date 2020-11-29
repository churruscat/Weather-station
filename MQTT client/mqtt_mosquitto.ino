// Do not forget to include mosquitto.h in main program
// define callback function 
void funcallback(char* topic, byte* payload, unsigned int payloadLength);
WiFiClient wifiClient;
PubSubClient clienteMQTT(server, 1883, funcallback, wifiClient);

/* function to connect to WiFi AP, there are to possible networks; if 
 you are only going to use one, use same values for ssid1 and ssid2
 in mosquitto.h */
boolean wifiConnect() {
int i=0,j=0;  
  ssid=ssid1;
  password=password1;
 
  // if (WiFi.status() == WL_CONNECTED ) return true;  // if already connected
  DPRINT("Connecting to WiFi  "); DPRINTLN(ssid); 
  WiFi.persistent(false); 
  WiFi.mode(WIFI_OFF);
  delay(1000);
  //WiFi.setPhyMode(WIFI_PHY_MODE_11B);  
  WiFi.mode(WIFI_STA);  //The 8266 is a station, not an AP 
  #ifdef IP_FIJA
    WiFi.config(ip, gateway, subnet);
  #endif
  delay(1000); 
  WiFi.begin(ssid,password);

  while ((WiFi.status() != WL_CONNECTED )) {
    espera(500);
    DPRINT(i++);
    DPRINT(".");   
    if (i>60) { 
      if (ssid==ssid1){
        ssid=ssid2;
        password=password2;
      } else {
        ssid=ssid1;
        password=password1;
      }
      i=0;
      j++;
      if (j>8) { return false;} /* none Wifi works */     
      DPRINTLN();
      DPRINT("Try with other network ");DPRINTLN(j);      
      DPRINT("I will try to connect to "); DPRINTLN(ssid);
      WiFi.mode(WIFI_OFF);
      delay(1000);
      // WiFi.setPhyMode(WIFI_PHY_MODE_11B);  // gives more stability
      WiFi.mode(WIFI_STA);  //The 8266 is a station, not an AP 
      #ifdef IP_FIJA
        WiFi.config(ip, gateway, subnet);
      #endif      
      espera(1000);
      WiFi.begin(ssid,password);      
    }
  }
  DPRINTLN(ssid);  DPRINT("*******Conected; ADDR= ");
  DPRINTLN(WiFi.localIP());
  return true;
}

/****************************************
 There is no connectivity , correct situation 
******************************************/
void sinConectividad(){
int j=0;

  clienteMQTT.disconnect(); 
  espera(500);
  while(!wifiConnect()) {   
    DPRINT("No connectivity, wait secs  ");DPRINTLN(int(intervaloConex/1000));
    espera(intervaloConex);
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
     if (WiFi.status() != WL_CONNECTED ) sinConectividad();      
     DPRINT(j);DPRINTLN("  Retry connection to MQTT  ");
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
  initManagedDevice(); 
}

boolean loopMQTT() {
  return clienteMQTT.loop();
}

/*************************************************************************
 * initialize the device, subscribing to 
 * some actions; in this case reboot(noexplanation),
 **************************************************************************/

void initManagedDevice() {
 int rReboot,rUpdate,rResponse; 

 clienteMQTT.setBufferSize(455);
 rReboot=  clienteMQTT.subscribe(rebootTopic,1);
 rUpdate=  clienteMQTT.subscribe(updateTopic,1);
 rResponse=clienteMQTT.subscribe(responseTopic,1);
 DPRINTLN("Suscripcion. Response= ");
 DPRINT("\tReboot= ");DPRINT(rReboot);
 DPRINT("\tUpdate= ");DPRINTLN(rUpdate); 
}

void funcallback(char* topic, byte* payload, unsigned int payloadLength) {
 DPRINT("funcallback invoked for topic: "); DPRINTLN(topic);
 if (strcmp (updateTopic, topic) == 0) {
   handleUpdate(payload);  
 }
 else if (strcmp (responseTopic, topic) == 0) { 
   DPRINTLN("handleResponse payload: ");
   DPRINTLN((char *)payload); 
 } 
 else if (strcmp (rebootTopic, topic) == 0) {
   DPRINTLN("Rearrancando...");    
   ESP.restart(); // this has issues, sometimes hangs
   //ESP.reset();
 }
}

void handleUpdate(byte* payload) {
 
 boolean cambia=false,pubresult;
 char sensor[20],elpayload[150];

 return;
}

/********** Send data to broker **************/

boolean enviaDatos(char * topic, char * datos_) {
  int k=0;
  boolean pubresult=false;  
  
 while (!clienteMQTT.loop() & k<20 ) {
    DPRINTLN("Device was disconnected, reconnecting ");   
    mqttConnect();
    initManagedDevice();
    k++; 
  }
  pubresult = clienteMQTT.publish(topic,datos_);
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
    ArduinoOTA.handle();
    delay(100);
  }
}
