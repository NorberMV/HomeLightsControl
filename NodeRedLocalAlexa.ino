/*
  IoT system using MQTT protocol to control the  module build by Signal2 that has embedded a ESP32 board within
  and uses four independent PWM channels for light control that allows dimm  the lights from 0 to 100%.
  Author: Norberto Moreno Velez
*/


#include <ArduinoJson.h>    // To handle the needed classes for the JSON Format
#include <WiFi.h>           // Library for WiFi managment of the ESP-32
#include <PubSubClient.h>   // library to handle the needed MQTT classes
#include <secrets.h>

 
// ********************** PWM Channels Configuration**************************************************************

// We are going to be using 4 different output channels in our module to control 4 different set of LED´s

const int ledOut1 = 21;  
const int ledOut2 = 22;
const int ledOut3 = 33;
const int ledOut4 = 34;

// Chanel 0
const int freq = 6000;
const int ledChannel0 = 0;
const int resolution = 8;

// Chanel 1
const int freq = 6000;
const int ledChannel1 = 1;
const int resolution = 8; 

// Chanel 2
const int freq = 6000;
const int ledChannel2 = 2;
const int resolution = 8; 

// Chanel 3
const int freq = 6000;
const int ledChannel3 = 3;
const int resolution = 8; 

// *****************************************************************************************************


char ssid[] = SECRET_SSID;     // The network SSID (name) 
char pass[] = SECRET_PASS;     // The network password
const char* mqtt_server = "";  // the MQTT Broker IP


WiFiClient espClient2;               // Crea una instancia tipo WifiCliente llamado espClient
PubSubClient client(espClient2);     //Gestiona la Biblioteca de publicación-subscripción para el cliente creado

int lastBright = 0;
bool flag=false;
long lastMsg = 0;                   // Declaración de variable para conteo de tiempo de transmisión de mensaje
char msg[50];                       // Decalaración de buffer para guardar el mensaje recibido
String sr2 = "";                    //Declaración de la variable sr2 para guardar la cadena de caracteres recibidos
String inputString = "";            //Declaración de variable auxiliares para datos recibidos
float gf = 0.0, vel = 0.0;          //Declaración de variable flotante  para configuración del posicionador
int gi = 0;                         //Declaración de variable int  para configuración del posicionador
static char buffer2[6];             //Declaración de variables para almacenar caracteres
char rec[50];                       //Declaración de varible auxiliar para la recepción de mensajes


void setup() {

// ************************************ PWM SETUP **************************************************************  
  // configure LED PWM functionalitites for the cha
  ledcSetup(ledChannel0, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledOut1, ledChannel0);

//************************************************************************************************************
  Serial.begin(115200);                    //Inicializa la comunicación serial a una velocidad de 115200
  setup_wifi();                            //Inicializa la Biblioteca WIFI
  client.setServer(mqtt_server, 1883);     //Inicializa el servidor con las dirección descrita en el encabezado y el puerto que usa
  client.setCallback(callback);            //Inicializa la función que se activa cuando llegan mensajes

}

// **WIFI SETUP**

void setup_wifi() {

  delay(10);                              // espera 10 milisegundos
  Serial.println();                       // Imprime un espacio de linea
  Serial.print("Conectando a ");          // Imprime la leyenda "Conectando a"
  Serial.println(ssid);                   // Imprime la identificación de la red a al que se conecta
  WiFi.begin(ssid, pass);             // Realiza la conexión a la red configurada usando el password configurado en el encabezado

  while (WiFi.status() != WL_CONNECTED) { // Espera mientras se establece la conexión
    delay(500);                           // Espera 500 milisegundos
    Serial.print(".");                    // Imprime en la terminal "." hasta que se logre la conexión
  }

  Serial.println("");                     //Imprime en la terminal un espacio
  Serial.println("WiFi connecteda");       //Imprime en la terminal un espacio
  Serial.println("dirección IP: ");       //Imprime en la terminal "dirección IP: "
  Serial.println(WiFi.localIP());         //Imprime en la terminal "dirección IP asignada al microcontrolador
}


void reconnect() {                                   //función para la reconexión
  // Loop until we're reconnected
  while (!client.connected()) {                      //Espera hasta que se establezca la reconexión
    Serial.print("Esperando Conexión MQTT...");

    if (client.connect("ESP8266Cliente2")) {                //Se conecta como cliente con el nombre ESP8266Cliente
      Serial.println("connecteda");                        //Imprime en la terminal conectado con el cliente
      client.subscribe("mihogar2/actuadores");          //se suscribe al tópico invernadero/actuadores
    } else {                                               // si no logra conectarse
      Serial.print("falla, rc=");                            //Imprime en la terminal "falla, rc="
      Serial.print(client.state());                          //Imprime en la terminal el tipo de falla de conexión (un múmero)
      Serial.println("Intento de nuevo en 5 segundos");      //Imprime en la terminal "Intento de nuevo en 5 segundos"
      // Wait 5 seconds before retrying
      delay(5000);                                           //Espera 5 segundos
    }
  }
}

// MAIN FUNCTION

void loop() {

  if (!client.connected()) {                                  // Pregunta si el cliente  no se ha conectado
    reconnect();                                                 //Ejecuta la función de reconexión
  }
  client.loop();                                               //En caso de conexión exitosa ejecuta el bucle del cliente
  
  // Uncomment this secction to configure for receive sensor data  
  StaticJsonBuffer<200> JSONbuffer;                                 //Creamos un Objeto para el almacenamiento del JSON a crear
  JsonObject& JSONHom = JSONbuffer.createObject();                  //Declaramos el Objeto creado tipo JSON y lo asociamos a la variable JSONHom


  char JSONmessageBuffer[100];                                      //Declaramos una variable para el almacenamiento del JSON creado
  JSONHom.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));    //Creamos el JSON con los pares clave valor de los pasos anteriores


  long now = millis();                                              //función para el envío periodico de mensajes cada tiempo determinado usando la millis
  if (now - lastMsg > 3000) {                                       //Si el tiempo transcurrido es mayor a 2 segundos
    lastMsg = now;                                                  //reiniciamos el tiempo
    //Serial.println(msg);                                            //Imprimimos en la terminal el valor del msg
    //client.publish("mihogar2/sensores", JSONmessageBuffer);      //publicamos en el tópico invernadero/sensores el JSON creado con los valores de temp y hum
  }
}



void callback(char* topic, byte* payload, unsigned int length) {


  sr2 = "";                                                      //Limpio la variable donde se guardan los mensajes recibidos
 
  for (int i = 0; i < length; i++) {                            //Recorre el mensaje recibido de acuerdo a su longitud
    Serial.print((char)payload[i]);                             //Imprime en la terminal serial el mensaje recibido
    rec[i] = payload[i];                                       // guardo lo recibido en una variable auxiliar
    sr2 += rec[i];                                            // Concatena los mensajes enviados en la varible sr2
    inputString = sr2;                                        // guardo todo el mensaje en la variable inputString
  }
  
  Serial.println("Printing incoming JSON...");
  char msg2[inputString.length()];                             // Se crea una variable con el tamaño del valor de longitud del mensaje
  Serial.println(msg2);                                        // Se imprime el mensaje recibido
  inputString.toCharArray(msg2, inputString.length() + 1);     // Se convierte en una cadena de caracteres
  DynamicJsonBuffer jsonBuffer;                                // Se crea una buffer dinamico donde guardaremos el JSON recibido
  JsonObject &root = jsonBuffer.parseObject(msg2);             // Se Interpreta el mensaje recibido como un JSON y se guara en la variable root
  String state = root["state"];                                  // Se Extrae el valor asociado a la clave "on" y se guarda en la variable apertura_l
  
  int  luminosidad_l = root["brightness"];
  int brightness = map(luminosidad_l, 1, 255, 0, 255);
  
  
  if (state != "true") {
    
    for(int i=lastBright; i>=0; i--) {
      ledcWrite(ledChannel, i);
      Serial.println(i);
      delay(15);
    }
    lastBright =0;
  }
  else {
    
    if ( brightness > lastBright) {
      for(int i=lastBright; i<=brightness; i++){
        ledcWrite(ledChannel, i);
        
        delay(15);
      }
      lastBright = brightness;
    }
    else if ( brightness < lastBright) {
      for(int i=lastBright; i>=brightness; i--){
        ledcWrite(ledChannel, i);
        
        delay(15);
      }
      lastBright = brightness;
    }
    else if (brightness == lastBright ){
      for(int i=0; i<=brightness; i++){
        ledcWrite(ledChannel, i);
        
        delay(15);
      }
      lastBright = brightness;
    }
  }
  

}
