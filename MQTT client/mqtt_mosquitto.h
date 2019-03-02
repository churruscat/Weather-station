#include <ESP8266WiFi.h>
#define MQTT_MAX_PACKET_SIZE 455 //cambialo antes de incluir docpatth\Arduino\libraries\pubsubclient-master\src\pubsubclient.h
#define MQTT_KEEP_ALIVE 60
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include <Pin_NodeMCU.h>

/*************************************************
 ** -------- Personalized values -------------- **
 * ***********************************************/
char* ssid;
char* password;
char ssid1[] = "mywifiSSID";
char password1[] = "password1";
char ssid2[] = "mywifiSSID2";
char password2[] = "password2";
/*************************************************
 ** ----- End of Personalized values ---------- **
 * ***********************************************/
#define ESPERA_NOCONEX 70000  // when there is no connnection, wait 70sec
char server[] = "192.168.1.11"; // MQTT broker address
char * authMethod = NULL;   
char * token = NULL;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
char publishTopic[] = "meteo/envia";  // Device sends data to MQTT broker
char rebootTopic[]  = "meteo/reboot";



