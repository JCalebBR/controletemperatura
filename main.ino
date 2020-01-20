#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <OneWire.h> //INCLUSÃO DE BIBLIOTECA
#include <DallasTemperature.h> //INCLUSÃO DE BIBLIOTECA

#define DS18B20 14 //D5

#define ssid "Caleb"
#define senha "caleb123"

OneWire ourWire(DS18B20);
DallasTemperature sensors(&ourWire);

double temp = 0;
volatile int i = 0;
volatile boolean zero_cross = 0;
int AC_pin = 13; //D7
int zero_cross_pin = 12; //D6
double dim = 0;

uint32_t intervalo = 1000;
uint32_t tempo_ultimo = 0;

AsyncWebServer server(80);

String readTemp() {
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  Serial.println(temp);
  if (temp != -127.00) {
    return String(temp);
  }
}

ICACHE_RAM_ATTR void zero_cross_detect() {
  zero_cross = 1;
}

ICACHE_RAM_ATTR void dim_check() {
  if(zero_cross == 1) {
    double ndelay = (1.0 - dim) * 7330.0;
    delayMicroseconds(ndelay);
    digitalWrite(AC_pin, HIGH);
    delayMicroseconds(100);
    digitalWrite(AC_pin, LOW);
    zero_cross = 0;
  }
}

void setup() {
  if(!SPIFFS.begin()){
    Serial.println("Erro ao inicializar o SPIFFS");
    return;
  }
  
  Serial.begin(115200);

  pinMode(zero_cross_pin, INPUT);
  pinMode(AC_pin, OUTPUT);
  digitalWrite(AC_pin, LOW);
  
  attachInterrupt(digitalPinToInterrupt(zero_cross_pin), zero_cross_detect, FALLING);

  timer1_isr_init();
  timer1_attachInterrupt(dim_check);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write(325);
  
  //Serial.println("Tell me why");
  
  WiFi.begin(ssid, senha);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Conectado! ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  server.on("/ang", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("angulo")) {
      AsyncWebParameter* p = request->getParam("angulo");
      dim = p->value().toDouble();
      dim = dim / 100.0;
      request->send(205);
    }
  });
  server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/highcharts.js", "text/javascript");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado!");
  
  sensors.begin();
  sensors.setResolution(11);
}
