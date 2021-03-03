/*
 * SPIFFSWebServer.ino
 * 
 * Se implementa una página web dinámica en ESP32
 * 
 * Gabriel Pool
 * Febrero 2021
*/

// Bibliotecas necesarias para el SPIFFS
#include "FS.h"
#include "SPIFFS.h"

// Bibliotecas necesarias para el sensor DHT22
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <time.h>        // Biblioteca requerida para manejo de la hora local
#include <WiFi.h>        // Biblioteca requerida para WiFi
#include <WiFiClient.h>  // Biblioteca requerida para manejo de clientes conectados al servidor
#include <WebServer.h>   // Biblioteca requerida para manejo de funciones del servidor WEB
#include <ESPmDNS.h>     // Biblioteca requerida para manejo de difución en la red.

const char *ssid = "IZZI-98C1-PB";     // Definición que establece el nombre de la red
const char *password = "135A87F298C1"; // Definición que establece la clave de la red

WebServer server(80);  // Instancia que controla las funciones del servidor WEB 
                       // y establece el puerto a 80 el cual es el Standart para las páginas WEB
const int led = LED_BUILTIN;  // Definición que establece el pin donde se conecta el LED incorporado en el módulo NodeMCU (algunos modelos NO lo incluyen)
String estadoLed = "false";

// Declaraciones necesarias para el sensor DHT22
#define DHTPIN 32     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)

// Constructor para el sensor DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// Declaración de las variables necesarias para el sensor DHT22
sensors_event_t event;  // Se almacena la instancia (direccion en memoria) de las funciones del sensor
char charTemp[7];  // Almacena el valor de la temperatura (limitado a dos decimales)
char charHum[7];  // Almacena el valor de la humedad (limitado a dos decimales)

// Declaraciones necesarias para configrar la hora local
long timezone = -6; // Zona horaria, Mexico se ubica en -6
byte daysavetime = 1;  // Número de horas de atrazo en horario de verano
struct tm tmstruct ;  // Variable que almacena los valores del tiempo
time_t now;  // Variable que almacena los valores del tiempo en formato "timestamp"

char responseDataGraphic [1800];

void sendDataTemp() {  // Método que envía la página WEB al cliente que lo solicite
  char temp[150];    // Se define una variable la cual, contendrá la página WEB en forma de texto
  //get time
  time(&now);
  localtime_r(&now, &tmstruct);

  
  snprintf(temp, 150, \
  /**************************************** INICIA EL FORMATO XML ************************************/
  "<response>\n"\
  "\t<TEMP>%s</TEMP>\n"\
  "\t<HUM>%s</HUM>\n"\
  "\t<ledBuiltin>%s</ledBuiltin>\n"\
  "\t<TIME>%02d:%02d:%02d</TIME>\n"\
  "</response>\n",
/**************************************** TERMINA EL FORMATO XML ************************************/
/****************************** INICIA LISTA DE VARIABLES A SER ENVIADAS ****************************/
  charTemp, \
  charHum, \
  estadoLed, \
  tmstruct.tm_hour, \
  tmstruct.tm_min, \
  tmstruct.tm_sec  // Aquí se calcula del tiempo en que inicia la ejecución del código y los valores se incrustan en la línea 61.
/*********************** TERMINA INICIA LISTA DE VARIABLES A SER ENVIADAS ***************************/
          );
  server.send(200, "text/xml", temp);  // Sentencia que envía la página web al cliente
}  // Termina método que envia la página WEB al cliente que lo solicitó.

void controlLed (void){  // Metodo que controla el estado del led incorporado en elNodeMCU
  for ( uint8_t i = 0; i < server.args(); i++ ) { // Se analiza el número de argumentos
    Serial.print (server.argName ( i ));  // Se imprime en el monitor serial el nombre del argumento recibido
    Serial.print (": ");  // Se imprime el separador de nombre y valor
    Serial.println (server.arg ( i ));  // Se imprime al valor del argumento
    if ( server.argName ( i ).equalsIgnoreCase("ledBuiltin") ) {  // Se compara cual es el Led que se desea controlar
      if (server.arg ( i ).equalsIgnoreCase("false")){  // Si nombre del argumento es ledBuitin se verifica el valor del argumento
        digitalWrite(led, LOW);  // Si el nombre y el valor de argumento es el solicitado se cambia de estado el led que el usuario eligió
        estadoLed = "false";
      }
      else{
        digitalWrite(led, HIGH); // Si el nombre y el valor de argumento es el solicitado se cambia de estado el led que el usuario eligió
        estadoLed = "true";
      }
    }
  }
}

bool loadFromSdCard(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  else if(path.endsWith(".svg")) dataType = "image/svg+xml";

  Serial.println("path: ");
  Serial.println(path.c_str());

  File dataFile = SPIFFS.open(path.c_str(), "r");

  if (!dataFile)
    return false;

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}


void handleNotFound() {  //Metodo que devuelve al cliente el aviso de que no se encontró el recurso solicitado.
  if(loadFromSdCard(server.uri())) return;
  String message = "File Not Found\n\n";  // Definición de la variable que contiene la respuesta que se envía al cliente.
  message += "URI: ";  
  message += server.uri();  // Muestra el recurso solicitado
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";  // Muestra el tipo de petición realizada por el cliente: GET ó POST
  message += "\nArguments: ";
  message += server.args();  // Muestra el número parámetros enviados
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";  // muestra el nombre y el valor de los parámetros enviados
  }

  server.send(404, "text/plain", message);  // Se envía la respuesta al cliente que lo solicitó.
}  // Fin del método que devuelve al cliente el aviso de que no se encontró el recurso solicitado.

void setup(void) {  // Método principal para configuración de periféricos y funciones
  pinMode(led, OUTPUT);  // Se declara el número de pin asignado en la variable led como salida
  digitalWrite(led, 0);  // Se apaga el led incorporado en su módulo NodeMCU
  Serial.begin(115200);  // Se configura la velocidad de transferencia del puerto serie
  WiFi.mode(WIFI_STA);  // Se cofigura el modo de WIFI que se va a utilizar (Modo estación: el dispositivo solicitará conectarse a una red WiFi)
  WiFi.begin(ssid, password);  // Se proporcionan las credenciales del WiFi
  Serial.println("");  // Se imprime un salto de línea en el monitor de puerto serial.
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {  // Se verifica si ya se está conectado a la red WiFi ...
    delay(500);                            // ... cada medio segundo y mientras no se conecte ...
    Serial.print(".");                     // ... se imprimira un punto en el monitor de puerto serie.
  }  // Al salir de éste bucle, no indica que la conexión a la red WiFi fué un EXITO!!!

  Serial.println("");  // Se imprime un salto de línea en el monitor de puerto serial.
  Serial.print("Connected to ");  // Se imprime la leyenda en el monitor de puerto serial.
  Serial.println(ssid);  // Se imprime el nombre de la red en el monitor de puerto serial.
  Serial.print("IP address: ");  // Se imprime la leyenda en el monitor de puerto serial.
  Serial.println(WiFi.localIP());  // Se imprime la direccion IP en el monitor de puerto serial.

  Serial.println("Contacting Time Server");
  configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
    delay(2000);
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
  Serial.println("");
  time(&now);
  localtime_r(&now, &tmstruct);

    
  if(!SPIFFS.begin()){
      Serial.println("Card Mount Failed");
      return;
  }

  if (MDNS.begin("esp32")) {  // Se configura la instancia para difución del nombre del dispostivo en la red.
    Serial.println("MDNS responder started");  // Si la configuración fué un éxito se mostrará esta leyenda en el monitor del puerto serie.
    MDNS.addService("http","tcp",80);  // Se anuncia el servicio en la red, para que el programa Service Browser pueda ubicarlo.
  }  // Si la configuración falló no se mostrará lo anterior en el monitor del puerto serie.
  server.on("/dataGraphics.xml", HTTP_GET, sendDataGraph);  // Se establece el nombre del método a ejecutar cuando el cliente introduzca la direcion IP en el navegador.
  server.on("/temp.xml", HTTP_GET, sendDataTemp  );
  server.on("/led", HTTP_GET, controlLed);  // Se añade un nuevo recurso para controlar el led incorporado del NodeMCU
  server.onNotFound(handleNotFound);  // Se establece el nombre del método a ejecutar cuando el cliente introduzca la dirección IP seguido de un recurso NO declarado anteriormente.
  server.begin();  // Se inicia las funciones del servidor.
  Serial.println("HTTP server started");  // Se imprime la leyenda en el monitor de puerto serial.

  // Inicializando el sensor DHT22
  dht.begin();

  //Se inicializa el response (contiene los últimos 24 valores del sensor) en formato xml
  int apuntador = 0;
  snprintf(&responseDataGraphic[apuntador], 1800,"<response>\n");
  apuntador += 11;  // Se apunta al inicio de la segunda línea
  for(int i = 0; i < 24; i++){
    snprintf(&responseDataGraphic[apuntador], 1800, \
    /**************************************** INICIA EL FORMATO XML ************************************/
    "\t<TEMP%02d>-N/A-</TEMP%02d>\n"\
    "\t<HUM%02d>-N/A-</HUM%02d>\n"\
    "\t<TIME%02d>HH:MM:SS</TIME%02d>\n",
    /**************************************** TERMINA EL FORMATO XML ************************************/
    i, i, \
    i, i, \
    i, i
    );
    apuntador += 73;  // Se apunta al inicio de la linea de la siguiente serie de datos.
  }
  snprintf(&responseDataGraphic[apuntador], 1800,"</response>\n");
}

void loop(void) {  // Método principal que se ejecuta indefinidamente
  server.handleClient();  // Se atiende a los clientes conectados (si los hay)
  leeSensor();
}  // Fin del método principal que se ejecuta indefinidamente

void sendDataGraph() {  // Método que dibuja una gráfica tipo Scalable Vector Graphics (Gráfica vectorial escalable)

  server.send(200, "text/xml", responseDataGraphic);  // Sentencia que envía la página web al cliente
  //Serial.println("Response: ");
  //Serial.println(responseDataGraphic);


} // Fin del Método que dibuja una gráfica tipo Scalable Vector Graphics (Gráfica vectorial escalable)

void leeSensor (void) {

  // variables locales necesarias para establecer un intérvalo de tiempo (analogo a un delay())
  unsigned long currentMillis = millis(); // Almacena el valor actual del tiempo
  unsigned int interval = 1000; // Es el intérvalo de tiempo en el que se actualiza el valor del sensor y el contenido del response.
  static unsigned int previousMillis = 0; // Almacena el último valor de tiempo en que fué ejecutada el contenido del "if",
                                          // es decir, la lectura del sensor y la actualización del response.
  // variables requeridas para la actualización del response
  unsigned int apuntadorResponse = 0;  // Ayuda a insertar/respaldar los valores en el response
  char charTime[10];    // Almacena el valor actual del tiempo
  char firstCharTemp[7];  // Almacena el valor anterior de la temperatura (limitado a dos decimales)
  char firstCharHum[7];   // Almacena el valor anterior de la humedad (limitado a dos decimales)
  char firstCharTime[10]; // Almacena el valor anterior del tiempo (limitado a HH:MM:SS)
  char secondCharTemp[7];  // Almacena el valor anterior "del anterior" (o sea, )de la temperatura (limitado a dos decimales)
  char secondCharHum[7];   // Almacena el valor anterior de la humedad (limitado a dos decimales)
  char secondCharTime[10]; // Almacena el valor anterior del tiempo (limitado a HH:MM:SS)
  signed int i;  // Ayuda en el bucle a apuntar a cada serie de datos en el response.

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Get temperature event and print its value.
    dht.temperature().getEvent(&event);  // Se captura el valor de la temperatura instantánea
    if (isnan(event.temperature)) {
      snprintf (charTemp, sizeof(charTemp), "-N/A-");
      //Serial.println("Error reading temperature!");
    } else {
      //Serial.print("Temperature: ");
      dtostrf(event.temperature, 3, 2, charTemp); // Se tranfiere el valor de la temperatura a una variable
      //Serial.print(charTemp);
      //Serial.println("°C");
    }
    
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);  // Se captura el valor de la humedad instantánea
    if (isnan(event.relative_humidity)) {
      snprintf (charHum, sizeof(charHum), "-N/A-");
      //Serial.println("Error reading humidity!");
    } else {
      //Serial.print("Humidity: ");
      dtostrf(event.relative_humidity, 3, 2, charHum); // Se tranfiere el valor de la humedad relativa a una variable
      //Serial.print(charHum);  
      //Serial.println("%");
    }

    // get time
    time(&now);
    localtime_r(&now, &tmstruct);

    snprintf(charTime, sizeof(charTime), "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec );

    /************* Construcción del response para la gráfica que se muestra en la página web *******************
    Cada serie de datos son compuestos por tres ítems: temperatura, humedad y tiempo ...
    por ser un archivo xml, se requiere un formato predeterminado y cada valor está
    representado por tags así como sigue:
    
      \t<TEMPXX>XX.XX</TEMPXX>\n
      \t<HUMXX>XX.XX</HUMXX>\n
      \t<TIMEXX>XX:XX:XX</TIMEXX>\n
    
    ... de donde:

    X : es un valor numérico entre 0 y 9.
    \t: es la representación del caracter TAB (tabulador). Es utilizado para la identación del formato xml.
    \n: es la representación del salto de línea.
    TEMP: Corresponde al TAG de valores de temperatura
    HUM : Corresponde al TAG de valores de humedad
    TIME: Corresponde al TAG de valores de tiempo.
    *********************************************************************************************************/
    // Se inicia la actualización del response, se escribe el encabezado
    snprintf( responseDataGraphic, sizeof(responseDataGraphic), "<response>\n");

    // Se copian los datos del sensor en una variable para ser anexados 
    // en la primera posición del response en la primera iteracion.
    // para evitar errores en el tamaño del formato se le añaden los parámetros siguientes:
    // "%*s", 5 indica que solamente se va a copiar 5 caracteres, en caso de que charTemp sea menor de 5 caracteres
    // se rellenará con espacios en blanco.
    snprintf(firstCharTemp, 6,"%*s", 5, charTemp);  // Se copian 5 caracteres de la temperatura (XX.XX) + el caracter indicador de fin de linea
    snprintf(firstCharHum, 6,"%*s", 5, charHum);  // Se copian 5 caracteres de la humedad (XX.XX) + el caracter indicador de fin de linea
    snprintf(firstCharTime, 9,"%*s", 8,charTime);  // Se copian 8 caracteres del tiempo (XX:XX:XX) + el caracter indicador de fin de linea

    for(i = 0; i < 24; i++){   // Se desplazan las series de datos un lugar hacia abajo en la serie.

      if (i < 23) {  // En la última serie, los datos ya no son "respaldados"
        
        apuntadorResponse = (73 * i) + 11;  // Se apunta al inicio de la serie (la posición es representada por la variable i).
                                            // con i = 1 se está en la posición de la segunda serie de datos (recuerde que anteriormente ya se ha respaldado la primera serie)
                                            // En este caso cada serie de datos tiene 73 caracteres de longitud (incluye saltos de linea y tabulador)
                                            // y el encabezado del respose tiene 11 caracteres de longitud (incluye el salto de línea)
        // Se respalda la temperatura registrada para ser desplazada un lugar hacia abajo en la sicuiente iteracción del "for"
        apuntadorResponse += 9;  // Se apunta al número que representa las decenas de la temperatura: \t<TEMPXX>xx.xx</TEMPXX>\n -> total: 24 caracteres
        // Se copian 5 caracteres de la temperatura (XX.XX) + el caracter indicador de fin de linea
        snprintf( secondCharTemp, 6, "%s", &responseDataGraphic[apuntadorResponse]);
       
         // Se respalda la humedad registrada para ser desplazada un lugar hacia abajo en la sicuiente iteracción del "for"
        apuntadorResponse += 23;  // Se apunta al número que representa las decenas de la humedad: \t<HUMXX>xx.xx</HUMXX>\n -> total: 22 caracteres
        // Se copian 5 caracteres de la humedad (XX.XX) + el caracter indicador de fin de linea
        snprintf( secondCharHum, 6, "%s", &responseDataGraphic[apuntadorResponse]);
        
         // Se respalda el tiempo registrado para ser desplazado un lugar hacia abajo en la sicuiente iteracción del "for"
        apuntadorResponse += 23;  // Se apunta al número que representa las decenas de las horas: \t<TIMEXX>xx:xx:xx</TIMEXX>\n -> total: 27 caracteres
        // Se copian 8 caracteres del tiempo (XX:XX:XX) + el caracter indicador de fin de linea
        snprintf( secondCharTime, 9, "%s", &responseDataGraphic[apuntadorResponse]);
      }
      
      apuntadorResponse = (73 * i) + 11;  // Se apunta de nuevo al inicio de la serie.

      // Se copia los datos "mas recientes" en el response
      snprintf(&responseDataGraphic[apuntadorResponse], 1800, \
      /*************** INICIA EL FORMATO XML *****************/
      "\t<TEMP%02d>%s</TEMP%02d>\n"\
      "\t<HUM%02d>%s</HUM%02d>\n"\
      "\t<TIME%02d>%s</TIME%02d>\n",
      /************* TERMINA EL FORMATO XML ******************/
      /******* INICIA LISTA DE VARIABLES A SER ENVIADAS ******/
      i, firstCharTemp, i, \
      i, firstCharHum, i, \
      i, firstCharTime, i  
      /** TERMINA INICIA LISTA DE VARIABLES A SER ENVIADAS **/
      );

      if (i == 23) break; // los últimos datos ya no son respaldados.
      
      // Se copian los datos extraídos del response para ser desplazados 
      // un lugar en la siguiente iteración.
      snprintf(firstCharTemp, sizeof(firstCharTemp),"%s", secondCharTemp);
      snprintf(firstCharHum, sizeof(firstCharHum),"%s", secondCharHum);
      snprintf(firstCharTime, sizeof(firstCharTime),"%s", secondCharTime);

    }
    // Se escribe el cierre del TAG del response (el final del response).
    snprintf(&responseDataGraphic[1763], 1800,"</response>\n");  // El final del response (73 * 24) + 11
    /************************** Fin de la construcción del response *****************************************************/
    //Serial.println("Response: ");
    //Serial.println(responseDataGraphic);

  }
}
