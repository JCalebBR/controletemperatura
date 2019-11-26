#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <OneWire.h> //INCLUSÃO DE BIBLIOTECA
#include <DallasTemperature.h> //INCLUSÃO DE BIBLIOTECA
 
#define DS18B20 13 //DEFINE O PINO DIGITAL UTILIZADO PELO SENSOR
#define FIREBASE_HOST "testestcc-38d19.firebaseio.com"
#define FIREBASE_AUTH "kCVdFH0fNvllqrm5X3Y7gARaLkJ3GicjlYpQhhO3"
#define WIFI_SSID "LINSCA"
#define WIFI_PASSWORD "JF0UR13R"

FirebaseData firebaseData;
OneWire ourWire(DS18B20);
DallasTemperature sensors(&ourWire);

void setup(){
  Serial.begin(9600); //INICIALIZA A SERIAL
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Conectado! ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.enableClassicRequest(firebaseData, true);
  
  sensors.begin();
   
  delay(1000);
}
 
void loop(){
  sensors.requestTemperatures();
  double temp = sensors.getTempCByIndex(0);
  Serial.println(temp);
  if (Firebase.pushDouble(firebaseData, "Temperatura", temp)) {
    Serial.println("Enviado");
    Serial.println(firebaseData.dataPath());
    Serial.println(firebaseData.pushName());
  } else {
    Serial.println("Falha");
    Serial.println(firebaseData.errorReason());
  }
  delay(250);
}
