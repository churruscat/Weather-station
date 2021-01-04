#define DEVICE_ID "1st-Device"
#define DEVICE_TYPE "ESP12E"
#define ORG "Home"
#define IS_BME280
#undef IP_FIJA
#undef CON_LLUVIA
#undef CON_UV
#define CON_SUELO   // con sensor de humedad del suelo
#define PRESSURE_CORRECTION (1.080)  // HPAo/HPHh 647m
#define HUMEDAD_MIN  50  // valores de A0 para suelo seco y empapado
#define HUMEDAD_MAX  450
#define INTERVALO_CONEX 58000 // 5 min en milisecs
