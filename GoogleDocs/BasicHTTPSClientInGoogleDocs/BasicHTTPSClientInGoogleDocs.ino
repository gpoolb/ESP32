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

#include <WiFi.h>        // Biblioteca requerida para WiFi
#include <WiFiMulti.h>   // Biblioteca que administra la conectividad en la red WiFi
#include <ESPmDNS.h>     // Biblioteca requerida para manejo de difusión en la red.
#include <HTTPClient.h>  // Biblioteca requerida para manejo de redireccionamiento en google sheets
#include <WiFiClientSecure.h>

const char *ssid = "mi_red_wifi"; // Definición que establece el nombre de la red
const char *password = "mi_password_de_red_wifi"; // Definición que establece la clave de la red

// Constantes necesarias para enviar datos a Google
const char *GScriptId = "myIdScript"; // Id del proyecto generado al momento de configurar los scripts (ver documentacion)
const int ledPin = LED_BUILTIN;  // Definición que establece el pin donde se conecta el LED incorporado en el módulo NodeMCU (algunos modelos NO lo incluyen)

String payload;  // Se utiliza para almacenar el response en formato estandar que será enviada al servidor de Google
char buffer[1024 + 1] = {};

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
byte daysavetime = 1;  // Número de horas de atrazo en horario de verano en México es 1
struct tm tmstruct ;  // Variable que almacena los valores del tiempo

// Declaración de la instancia para administrar las redes WiFi
WiFiMulti WiFiMulti;

bool EnviaDatosGoogle (void) {  // Este metodo se encarga de enviar los datos del sensor al spreadsheet de Google

  WiFiClientSecure *clientGoogle = new WiFiClientSecure;
  String url = String("https://script.google.com/macros/s/") + GScriptId + "/exec?" + payload; // Almacena el recurso que se solicitará al servidor
  int httpCodeGoogle = -1;  // Almacena el código de respuesta del servidor
  bool flag = false;  // Auxiliar para la creación de la instancia del cliente.
  int retval = 0;  // Auxiliares para los intentos de conexión del cliente hacia el servidor.
  String payloadGoogle = "";

  if(clientGoogle) {
    //clientGoogle -> setCACert(rootCACertificate);
    clientGoogle->setInsecure();

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      // Se crea la instancia del cliente
      HTTPClient httpsGoogle;
  
      httpsGoogle.setRedirectLimit(10);  // Se limita a 10 redirecciones 
      httpsGoogle.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);  // Se activa el redireccionamiento
      Serial.print("[HTTPS] begin...\n");
      if (httpsGoogle.begin(*clientGoogle, url)) {  // HTTPS Se envía la petición al servidor
        Serial.print("[HTTPS] GET...\n");
        Serial.print("[HTTPS] URL: ");
        Serial.println(url);
        // start connection and send HTTP header
        httpCodeGoogle = httpsGoogle.GET();  // Se espera el código resultante de la conexión con el servidor

        // httpCode will be negative on error
        // Si el código es positivo indica que el servidor ha respondido.
        if (httpCodeGoogle > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCodeGoogle);
  
          // file found at server
          // Si el código es 200 ó 302 indica que se logró la interacción con el servidor
          if (httpCodeGoogle == HTTP_CODE_OK || httpCodeGoogle == HTTP_CODE_MOVED_PERMANENTLY) {
            String payloadGoogle = httpsGoogle.getString();  // Se obtiene la respuesta del servidor
            Serial.println(payloadGoogle);
            flag = true;
          }
        } else
          Serial.printf("[HTTPS] GET... failed, error: %s\n", httpsGoogle.errorToString(httpCodeGoogle).c_str());
  
        httpsGoogle.end();  // Se cierra la conexión
      } else
        Serial.printf("[HTTPS] Unable to connect\n");


      httpsGoogle.~HTTPClient();  // Se borran las instancias
      // End extra scoping block
    }

  } else
    Serial.println("Unable to create client");

  // Se borran las instancias del cliente.
  clientGoogle->~WiFiClientSecure();  
  delete clientGoogle;
  clientGoogle = nullptr;

  return flag;
}

void setup(void) {  // Método principal para configuración de periféricos y funciones
  pinMode(ledPin, OUTPUT);  // Se declara el número de pin asignado en la variable led como salida
  digitalWrite(ledPin, HIGH);  // Se apaga el led incorporado en su módulo NodeMCU
  Serial.begin(115200);  // Se configura la velocidad de transferencia del puerto serie
  WiFi.mode(WIFI_STA);  // Se cofigura el modo de WIFI que se va a utilizar (Modo estación: el dispositivo solicitará conectarse a una red WiFi)
  WiFiMulti.addAP(ssid, password);  // Se proporcionan las credenciales del WiFi
  Serial.println("");  // Se imprime un salto de línea en el monitor de puerto serial.
  
  // Wait for connection
  while (WiFiMulti.run() != WL_CONNECTED) {  // Se verifica si ya se está conectado a la red WiFi ...
    delay(500);                            // ... cada medio segundo y mientras no se conecte ...
    Serial.print(".");                     // ... se imprimira un punto en el monitor de puerto serie.
  }  // Al salir de éste bucle, no indica que la conexión a la red WiFi fué un EXITO!!!

  Serial.println("");  // Se imprime un salto de línea en el monitor de puerto serial.
  Serial.print("Connected to ");  // Se imprime la leyenda en el monitor de puerto serial.
  Serial.println(WiFi.SSID());  // Se imprime el nombre de la red en el monitor de puerto serial.
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
  int counter = 0;                       // es decir, la lectura del sensor y el envio del response.

  if ((currentMillis - previousMillis >= interval) && (WiFiMulti.run() == WL_CONNECTED)) {
    previousMillis = currentMillis;

    if (leeSensor()){  // Se lee el sensor y se prepara el response para ser enviado
      while (!EnviaDatosGoogle() && counter < 3)
        counter++;
    }
    counter = 0;
  }

}  // Fin del método principal que se ejecuta indefinidamente

bool leeSensor (void) {  // Lee el sensor y prepara el response para su envio.

  char charTime[12];    // Almacena el valor actual del tiempo
  String stringTemp;

  // get time
  getLocalTime(&tmstruct, 0);  // Se obtiene el tiempo local almacenado en memoria (sin esperar a una actualización).
  snprintf(charTime, sizeof(charTime), "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec ); // Se tranfiere el valor del tiempo a una variable
  
  // Get temperature event and print its value.
  dht.temperature().getEvent(&event);  // Se captura el valor de la temperatura instantánea
  if (isnan(event.temperature)) {
    snprintf (charTemp, sizeof(charTemp), "-N/A-");
    Serial.println("Error reading temperature!");
    return false; // Se indica que falló la lectura del sensor
  } else {
    //Serial.print("Temperature: ");
    dtostrf(event.temperature, 5, 2, charTemp); // Se tranfiere el valor de la temperatura a una variable
    for (int i=0; i < String(charTemp).length(); i++)
      if (charTemp[i] == ' ')
        charTemp[i] = '0';
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
    dtostrf(event.relative_humidity, 5, 2, charHum); // Se tranfiere el valor de la humedad relativa a una variable
    for (int i=0; i < String(charHum).length(); i++)
      if (charHum[i] == ' ')
        charHum[i] = '0';
    //Serial.print(charHum);  
    //Serial.println("%");
  }

  // Se prepara el response en formato estandar para su envio al servidor
  // tag=Update&Temp=XX.XX&Hum=XX.XX&Time=XX:XX:XX
  // de donde X son los caracteres numéricos
  payload = "tag=Update&Temp=" + String (charTemp) + "&Hum=" + String (charHum)+ "&Time=" + String (charTime);
  Serial.println(payload);  // Se observa en el puerto serial el response en formato estandar
  
  return true;  // Se indica que la lectura del sensor fué exitosa
}
