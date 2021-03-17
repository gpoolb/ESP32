/*
 * GoogleDocs.ino
 * 
 * Se implementa un cliente basado en ESP32
 * para enviar datos a un spreadsheet de Google
 * 
 * Gabriel Pool
 * Marzo 2021
*/

// Bibliotecas necesarias para el sensor DHT22
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <time.h>        // Biblioteca requerida para manejo de la hora local
#include <WiFi.h>        // Biblioteca requerida para WiFi
#include <ESPmDNS.h>     // Biblioteca requerida para manejo de difusión en la red.
#include "HTTPSRedirect.h"  // Biblioteca requerida para manejo de redireccionamiento en google sheets

const char *ssid = "IZZI-98C1-PB";     // Definición que establece el nombre de la red
const char *password = "135A87F298C1"; // Definición que establece la clave de la red

// Constantes necesarias para enviar datos a Google
const char* host = "script.google.com";  // Direccion donde se envian los documentos
const char *GScriptId = "myIdScript"; // Id del proyecto generado al momento de configurar los scripts (ver documentacion)
const int httpsPort = 443;  // Puerto correspondiente a las peticiones https
//Para obtener el fingerprint, teclee en linux el comando "echo | openssl s_client -connect script.google.com:443 | openssl x509 -fingerprint -noout" (sin las comillas)
const uint8_t fingerprint[20] = {0xC0, 0xBA, 0x1C, 0x62, 0xFD, 0x85, 0xD7, 0x99, 0xDA, 0xD8, 0x7D, 0xFE, 0x1D, 0x02, 0x66, 0x51, 0xA1, 0x50, 0x07, 0x12}; // Huella Digital del sitio script.google.com
                                
const int led = LED_BUILTIN;  // Definición que establece el pin donde se conecta el LED incorporado en el módulo NodeMCU (algunos modelos NO lo incluyen)

String payload;  // Se utiliza para almacenar el response en formato estandar que será enviada al servidor de Google

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

void EnviaDatosGoogle (void) {  // Este metodo se encarga de enviar los datos del sensor al spreadsheet de Google

  static HTTPSRedirect* clientGoogle = nullptr;  // Almacena la instancia del cliente que se conecta a Google
  String url = String("/macros/s/") + GScriptId + "/exec"; // Almacena el recurso que se solicitará al servidor
  static int httpCodeRequest;  // Almacena el código de respuesta del servidor
  static bool flag = false;  // Auxiliar para la creación de la instancia del cliente.
  static int i, retval;  // Auxiliares para los intentos de conexión del cliente hacia el servidor.

  if (!flag){  // Se crea la instancia del cliente y se le asignan los valores preliminares
    clientGoogle = new HTTPSRedirect(httpsPort);  // Se crea la instancia del cliente
    clientGoogle->setInsecure();  // Se asigna la propiedad "inseguro"
    flag = true;  // Se marca la bandera de que la instancia del cliente ha sido creado exitosamente
    clientGoogle->setPrintResponseBody(true);  // Ayuda a visualizar los datos que envía el servidor en el puerto serie
    clientGoogle->setContentTypeHeader("application/json");  // Se utilizará responsive en formato json
  }

  if (clientGoogle == nullptr){ // Si la instancia del cliente no fué creada se interrumpe la ejecución del método
    Serial.println("Error creating client object!");  // Se muestra la leyenda en el puerto serial indicando el status del cliente.
    flag = false;  // Se marca con la bandera que la instanciación no tuvo éxito
    delete clientGoogle;  // Se borra la instancia.
    clientGoogle = nullptr;  // Se marca la instancia como "no inicializada".
    return;  // Se sale de este método
  }

  Serial.print("Connecting to ");  // Si la instancia tuvo exito ...
  Serial.println(host);            // ... se indica en el puerto serial

  // Try to connect for a maximum of 5 times
  flag = false;
  i = 0;
  for (i=0; i<5; i++){  // Se intenta la conexión 5 veces
    retval = clientGoogle->connect(host, httpsPort);  // Se inicia la conexión al servidor
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");  // En caso de fallar la conexión se indica en el puerto serie.
  }

  if (!flag){  // Si la conexión falló ...
    Serial.print("Could not connect to server: "); // ... se indica con un mensaje en el puerto serie ...
    Serial.println(host);
    Serial.println("Exiting...");
    delete clientGoogle;  // ... se borra la instancia creada ...
    clientGoogle = nullptr; // ... Se marca el puntero como "no inicializado"
    return;  // ... Se sale de este método.
  }
  
  Serial.println("Enviando datos al spreadsheet de Google");  // Si la ejecución del programa llega hasta este punto ...
  Serial.println("======================================="); // ... indica que el cliente ha logrado establecer conexión con el servidor.

  //este es el envio estandar de parametros
  //payload = "tag=Update&Temp=" + String (charTemp) + "&Hum=" + String (charHum);

    // Try to connect for a maximum of 5 times
  i = 0;
  for (i=0; i<5; i++){
    //un-comment this line to GET request
    clientGoogle->GET(url + "?" + payload, host); // Se envia el response generado en el metodo leeSensor() al servidor de Google
    // for POST request
    clientGoogle->POST(url, host, payload);  // Se envia el response por el metodo POST al servidor de Google (en el script de Google el metodo POST no está implementado)
    httpCodeRequest = clientGoogle->getStatusCode();  // Se obtiene el código del resultado del envio de parámetros
    if (httpCodeRequest > 0) {  // el código esperado es 200, si el código es un número negativo, indica que ocurrió una falla para enviar la información al servidor de Google
      Serial.print("[HTTP] GET... ");
      Serial.println(httpCodeRequest);  // Se muestra el código del resultado del envio de parámetros.
      break;                            // Si se recibe un código de 200 en el resultado de envio de parámetros 
    }                                   // se considera que el envío fué exitoso
    else
      Serial.println("Connection failed. Retrying...");
  }
  Serial.println();
  // Se reinician las variables que apoyan para la instancia del cliente.
  delete clientGoogle;
  clientGoogle = nullptr;
  flag = false;

}

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
  configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");  // Se inicializa la configuración del tiempo (hora y fecha)
    delay(2000);
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);  // Se obtiene el tiempo actual (si el tiempo no ha sido actualizado desde el servidor de tiempo se espera 5 segundos)
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
  Serial.println("");

  if (MDNS.begin("esp32")) {  // Se configura la instancia para difución del nombre del dispostivo en la red.
    Serial.println("MDNS responder started");  // Si la configuración fué un éxito se mostrará esta leyenda en el monitor del puerto serie.
    MDNS.addService("http","tcp",80);  // Se anuncia el servicio en la red, para que el programa Service Browser pueda ubicarlo.
  }  // Si la configuración falló no se mostrará lo anterior en el monitor del puerto serie.

  // Inicializando el sensor DHT22
  dht.begin();

  // Se inicializa el response en formato estandar
  payload = "tag=Update&Temp=-N/A-&Hum=-N/A-&Time=---N/A--";

}

void loop(void) {  // Método principal que se ejecuta indefinidamente

  // variables locales necesarias para establecer un intérvalo de tiempo (analogo a un delay())
  unsigned long currentMillis = millis(); // Almacena el valor actual del tiempo
  unsigned int interval = 60000; // Es el intérvalo de tiempo en el que se actualiza el valor del sensor y el contenido del response(en este caso, cada minuto).
  static unsigned int previousMillis = 0; // Almacena el último valor de tiempo en que fué ejecutada el contenido del "if",
                                          // es decir, la lectura del sensor y el envio del response.
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (leeSensor())  // Se lee el sensor y se prepara el response para ser enviado
      EnviaDatosGoogle();  // Se envia el response (con los datos del sensor) al servidor
  }

}  // Fin del método principal que se ejecuta indefinidamente

bool leeSensor (void) {  // Lee el sensor y prepara el response para su envio.

  char charTime[10];    // Almacena el valor actual del tiempo

  // Get temperature event and print its value.
  dht.temperature().getEvent(&event);  // Se captura el valor de la temperatura instantánea
  if (isnan(event.temperature)) {
    snprintf (charTemp, sizeof(charTemp), "-N/A-");
    Serial.println("Error reading temperature!");
    return false; // Se indica que falló la lectura del sensor
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
    Serial.println("Error reading humidity!");
    return false;  // Se indica que falló la lectura del sensor
  } else {
    //Serial.print("Humidity: ");
    dtostrf(event.relative_humidity, 3, 2, charHum); // Se tranfiere el valor de la humedad relativa a una variable
    //Serial.print(charHum);  
    //Serial.println("%");
  }

  // get time
  getLocalTime(&tmstruct, 0);  // Se obtiene el tiempo local almacenado en memoria (sin esperar a una actualización).
  snprintf(charTime, sizeof(charTime), "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec ); // Se tranfiere el valor del tiempo a una variable

  // Se prepara el response en formato estandar para su envio al servidor
  // tag=Update&Temp=XX.XX&Hum=XX.XX&Time=XX:XX:XX
  // de donde X son los caracteres numéricos

  payload = "tag=Update&Temp=" + String (charTemp) + "&Hum=" + String (charHum)+ "&Time=" + String (charTime);
  Serial.println(payload);  // Se observa en el puerto serial el response en formato estandar
  return true;  // Se indica que la lectura del sensor fué exitosa
}
