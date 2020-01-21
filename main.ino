//Biblioteca Wifi 
#include <ESP8266WiFi.h>
//Bibliotecas servidor web
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//Biblioteca do sistema de arquivos SPIFFS
#include <FS.h>
//Bibliotecas de leitura e conversao do sensor de temperatura
#include <OneWire.h>
#include <DallasTemperature.h>
//Definiçao do pino do sensor
#define DS18B20 14
//Definiçao das credenciais da rede wifi
#define ssid "REDE"
#define senha "SENHA"

//Instancia da biblioteca passando o pino do sensor como argumento
OneWire ourWire(DS18B20);
//Instancia da biblioteca a partir da OneWire
DallasTemperature sensors(&ourWire);
//Instancia da biblioteca de servidor web
AsyncWebServer server(80);

//Criaçao das variaveis:
//Variavel de temperatura
double temp = 0;
//Variavel de dimming
double dim = 0;
//Variavel/flag de controle de cruzamento de zero
volatile boolean zero_cross = 0;

//Variaveis dos pinos utilizados
//Pino de saida do sinal de disparo (GPIO13/D7)
int AC_pin = 13; //D7
//Pino de entrada do sinal de cruzamento de zero (GPIO12/D6)
int zero_cross_pin = 12; //D6

//Funçao de leitura de temperatura
String readTemp() {
  //Requisiçao de dados ao sensor
  sensors.requestTemperatures();
  //Conversao de dados do sensor
  temp = sensors.getTempCByIndex(0);
  //Quando o sensor esta com mal contato ou nao teve tempo suficiente para conversao
  //ele retorna -127,00 Celsius como temperatura, entao para evitar dados falsos
  //Se a temperatura for diferente de -127,00
  if (temp != -127.00) {
    //Retorne a temperatura
    return String(temp);
  }
}

//Funçao de interrupçao do cruzamento de zero
//ICACHE_RAM_ATTR significa que a funçao sera guardada na 
//RAM diferente da memoria flash (padrao)
ICACHE_RAM_ATTR void zero_cross_detect() {
  //Valor 
  zero_cross = 1;
}
//Funçao de disparo
ICACHE_RAM_ATTR void dim_check() {
  //Se houve cruzamento de zero
  if(zero_cross == 1) {
    //Um ciclo em 60hz equivale a aproximadamente 16,66ms, ja que o triac dispara
    //em ambos os semiciclos (8,33ms) e o pulso de cruzamento de zero tem largura
    //de aproximadamente 1ms, o momento de disparo dependendo da potencia (%) desejada
    //pode ser calculado por ((100 - POTENCIA)/100) * 7330 ou (1.0 - dim) * 73330
    double ndelay = (1.0 - dim) * 6330.0;
    delayMicroseconds(ndelay);
    delay(1);
    //Inicio do pulso de disparo
    digitalWrite(AC_pin, HIGH);
    //Duraçao do pulso de disparo
    delayMicroseconds(100);
    //Fim do pulso de disparo
    digitalWrite(AC_pin, LOW);
    //Reset da variavel de cruzamento de zero
    zero_cross = 0;
  }
}

//Setup
void setup() {
  //Inicia a comunicaçao serial em 115200 baudrate
  Serial.begin(115200);
  
  //Se houver falha ao inicializar o sistema de arquivos, exibe erro
  if(!SPIFFS.begin()){
    Serial.println("Erro ao inicializar o SPIFFS");
    return;
  }
  
  //Declaraçao dos pinos de entrada e saida
  pinMode(zero_cross_pin, INPUT);
  pinMode(AC_pin, OUTPUT);
  //Deixando o estado inicial conhecido
  digitalWrite(AC_pin, LOW);

  //Adiciona interrupcao a entrada digital do cruzamento de zero no momento de descida
  //Interrupcao executara a funçao zero_cross_detect
  attachInterrupt(digitalPinToInterrupt(zero_cross_pin), zero_cross_detect, FALLING);

  //Inicializaçao do timer1
  timer1_isr_init();
  //Adiciona a funçao de disparo a uma interrupçao
  timer1_attachInterrupt(dim_check);
  //Habilita o timer em modo DIV16, TIM_EDGE e TIM_LOOP
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  //TIM_DIV = 80MHz
  //TIM_DIV16 = TIM_DIV / 16 = 5 Mhz = incrementos de timer a cada 200ns,
  //logo para timer criar interrupçoes no tempo correto,
  //65µs / 200ns = 325
  timer1_write(325);

  //Tenta realizar a conexao wifi com as credenciais inseridas
  WiFi.begin(ssid, senha);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  //Quando conectado exibe status e ip
  Serial.print("Conectado! ");
  Serial.println(WiFi.localIP());

  //Para a pagina inicial, enviar o arquivo index.html localizado na memoria flash
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  //Deixa o arquivo javascript localizado na memoria flash disponivel ao servidor
  server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/highcharts.js", "text/javascript");
  });
  //Ao receber um GET na pagina /temp, enviar a temperatura medida pelo sensor e o estado HTTP 200 (OK)
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  //Ao receber um GET na pagina
  server.on("/ang", HTTP_GET, [](AsyncWebServerRequest *request){
    //Se o get posuir parametro angulo
    if (request->hasParam("angulo")) {
      //Recolha o valor String
      AsyncWebParameter* p = request->getParam("angulo");
      //Converta para double
      dim = p->value().toDouble();
      //e divida por 100 para ficar
      dim = dim / 100.0;
      //Enviar resposta 205 (Dados recolhidos e resete a pagina)
      request->send(205);
    }
  });
  //Inicie o servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado!");

  //Inicie o sensor
  sensors.begin();
  //Muda a resoluçao do sensor para 11 bits
  //Resoluçao 12 bits tem precisao de 0,0625ºC, 11 bits com 0,125ºC
  //Em 12 bits o tempo de resposta e de 700ms
  sensors.setResolution(11);
}
