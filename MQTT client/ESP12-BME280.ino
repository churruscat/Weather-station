/**
 ESP-12E  (ESP8266) to work as an IOT client
 it reads data from differenta gauges and sends data toan 
 MQTT client (see mqtt_mosquitto.ino)
 Morrastronics -- by chuRRuscat
 v1.0 2017 initial version
 v2.0 2018 mqtt & connectivity  functions separated
*/

#undef CON_DEBUG
//#define CON_DEBUG
/* Set up a macro that expands to print in debug mode
   or does nothing when in production 
*/
#ifdef CON_DEBUG
  #define DPRINT(...)    Serial.print(__VA_ARGS__)
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)     // blank line
  #define DPRINTLN(...)   // blank line
#endif
/*************************************************
 ** -------- Personalized values -------------- **
 * **********************************************/
#include "1stDevice.h"
#include "mqtt_mosquitto.h"
/***************************************************
 ** -------- End of Personalized values --------- **
 * ***********************************************/
#define AJUSTA_T 10000 // To adjust delay in some places along the program
#define SDA D5   // for BME280 I2C 
#define SCL D6
#define interruptPin D7 // PIN where I'll connect the rain gauge
#define sensorPin    A0  // analog PIN  of Soil humidity sensor
#define CONTROL_HUMEDAD D2  // Transistor base that swiths on and off soil sensor
#define L_POR_BALANCEO 0.2794 // liter/m2 for evey rain gauge interrupt
#include <Wire.h>             //libraries for sensors and so on
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Pin_NodeMCU.h>
#define PRESSURE_CORRECTION (1.080)  // HPAo/HPHh 647m

#define BME280_ADDRESS   (0x76)   //IMPORTANT, sometimes it is 0x77
Adafruit_BME280 sensorBME280;     // this represents the sensor

#define JSONBUFFSIZE 250
#define DATOSJSONSIZE 250

volatile int contadorPluvi = 0; // must be 'volatile',for counting interrupt 
// ********* these are the sensor variables that will be exposed ********** 
float temperatura,humedadAire,presionHPa,lluvia=0,sensacion=20;
int humedadMin=HUMEDAD_MIN,
    humedadMax=HUMEDAD_MAX,
    humedadSuelo,humedadCrudo;
int humedadCrudo1,humedadCrudo2;

// other variables
int intervaloConex INTERVALO_CONEX;
char datosJson[DATOSJSONSIZE];

// Interrupt counter for rain gauge
void ICACHE_RAM_ATTR balanceoPluviometro() {  
  contadorPluvi++;
}

// let's start, setup variables
void setup() {
boolean status;
#ifdef CON_DEBUG
 Serial.begin(115200);
#endif 
 DPRINTLN("starting ... "); 
 Wire.begin(SDA,SCL);
 status = sensorBME280.begin();  
 if (!status) {
   DPRINTLN("Can't connect to BME Sensor!  ");    
 }
 /* start PINs */
 pinMode(CONTROL_HUMEDAD,OUTPUT);
 pinMode(interruptPin, INPUT);
 attachInterrupt(digitalPinToInterrupt(interruptPin), balanceoPluviometro, RISING);
 digitalWrite(CONTROL_HUMEDAD, HIGH); // prepare to read soil humidity sensor
 espera(1000);
 humedadCrudo1 = analogRead(sensorPin); //first read to have date to get averages
 espera(1000);
 humedadCrudo2 = analogRead(sensorPin);  //second read
 digitalWrite(CONTROL_HUMEDAD, LOW);
 wifiConnect();   // prepare WiFi
 mqttConnect();   // and MQTT environment 
 delay(50);
 initManagedDevice();  // Setup the device to the IOT Server
 publicaDatos();       // and publish data. This is the function that gets and sends
}

uint32_t ultima=0;

void loop() {
 DPRINT("*");
 if (!loopMQTT()) {  // Check if there are MQTT messages and if the device is connected  
   DPRINTLN("Connection lost; retrying");
   sinConectividad();        
     mqttConnect();
     initManagedDevice();  // subscribe again
 } 
 if ((millis()-ultima)>intervaloConex) {   // if it is time to send data, do it
   DPRINT("interval:");DPRINT(intervaloConex);
   DPRINT("\tmillis :");DPRINT(millis());
   DPRINT("\tultima :");DPRINTLN(ultima);
   publicaDatos();        // publish data. This is the function that gets and sends
   ultima=millis();
 }
 espera(1000); and wait
}

/* get data function. Read the sensors and set values in global variables */
/* get data function. Read the sensors and set values in global variables */
boolean tomaDatos (){
  float bufTemp,bufTemp1,bufHumedad,bufHumedad1,bufPresion,bufPresion1;
  boolean escorrecto=true;  //return value will be true unless there is a problem
  /* read and then get the mean */
  bufHumedad= sensorBME280.readHumidity();   
  bufTemp= sensorBME280.readTemperature();
  bufPresion=sensorBME280.readPressure()/100.0F;
  /* activate soil sensor setting the transistor base */
  digitalWrite(CONTROL_HUMEDAD, HIGH);
  espera(10000);  
  humedadCrudo = analogRead(sensorPin); // and read soil moisture
  humedadCrudo=constrain(humedadCrudo,humedadMin,humedadMax); 
  digitalWrite(CONTROL_HUMEDAD, LOW);  // disconnect soil sensor
  // calculate the moving average of soil humidity of last three values 
  humedadCrudo=(humedadCrudo1+humedadCrudo2+humedadCrudo)/3;
  humedadCrudo2=humedadCrudo1;
  humedadCrudo1=humedadCrudo;
  // read again from BME280 sensor
  bufHumedad1= sensorBME280.readHumidity();
  bufTemp1= sensorBME280.readTemperature();
  bufPresion1= sensorBME280.readPressure()/100.0F;
  DPRINTLN("Data read"); 
  lluvia+=contadorPluvi*L_POR_BALANCEO;
  detachInterrupt(digitalPinToInterrupt(interruptPin));
  contadorPluvi=0;
  attachInterrupt(digitalPinToInterrupt(interruptPin), balanceoPluviometro, RISING);
  if (humedadMin==humedadMax) humedadMax+=1; 
  humedadSuelo = map(humedadCrudo,humedadMin,humedadMax,0,100);
  /* if data could not be read for whatever reason, raise a message (in CONDEBUG mode) 
    Else calculate the mean */
  if (isnan(bufHumedad) || isnan(bufTemp) || isnan(bufHumedad1) || isnan(bufTemp1) ) {       
     DPRINTLN("I could not read from BME280msensor!");       
     escorrecto=false;    // flag that BME280 could not read
  } else {
  temperatura=(bufTemp+bufTemp1)/2;
  humedadAire=(bufHumedad+bufHumedad1)/2;
  presionHPa=(bufPresion+bufPresion1)/2*PRESSURE_CORRECTION;
  if (temperatura>60) escorrecto=false;   //if temperature out of reasonable range
  if ((humedadAire>101)||(humedadAire<0)) escorrecto=false;    // or humidity
  DPRINT("\tTemperature: \t ") ;  DPRINT(temperatura);
  DPRINT("\tAir humidity: \t ");  DPRINT(humedadAire);
  DPRINT("\tPressure HPa : \t "); DPRINT(presionHPa);
  DPRINT("\tMoisture: \t ")     ; DPRINT(humedadSuelo);
  DPRINT("\tRaw Moisture: \t"); DPRINTLN(humedadCrudo);  
  } 
  return escorrecto;
}


/* this function sends data to MQTT broker */
void publicaDatos() {
  int k=0;
  char signo;
  boolean pubresult=true;  
 
  while(!tomaDatos()) {   // if tomaDatos() returns false, retry 30 times 
     espera(1000);        // waiting 1 sec between iterations
     if(k++>30) {         // after 30 iterations with no data, return
      return; 
     }
  }
  // Data is read an stored in global var. Prepare data in JSON mode
  if (temperatura<0) {  // to avoid probles with sign
    signo='-';          // if negative , set '-' character
    temperatura*=-1;  // if temp was negative, convert it positive 
  }  else signo=' ';
  // prepare the message
  sprintf(datosJson,"[{\"temp\":%c%d.%1d,\"hAire\":%d,\"hSuelo\":%d,\"hCrudo\":%d,\"HPa\":%d,\"l/m2\":%d.%3d},{\"deviceId\":\"%s\"}]",
        signo,(int)temperatura, (int)(temperatura * 10.0) % 10,\
        (int)humedadAire, (int) humedadSuelo,(int)humedadCrudo,(int)presionHPa,
        (int)lluvia, (int)(lluvia * 10.0) % 10,DEVICE_ID);
  // and publish them.
  pubresult = enviaDatos(publishTopic,datosJson);
  if (pubresult) 
    {lluvia=0;}      // I sent data was successful, set rain to zero 
}
