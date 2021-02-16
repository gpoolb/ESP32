/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <WiFi.h>        // Biblioteca requerida para WiFi
#include <WiFiClient.h>  // Biblioteca requerida para manejo de clientes conectados al servidor
#include <WebServer.h>   // Biblioteca requerida para manejo de funciones del servidor WEB
#include <ESPmDNS.h>     // Biblioteca requerida para manejo de difución en la red.

const char *ssid = "IZZI-98C1-PB";     // Definición que establece el nombre de la red
const char *password = "135A87F298C1"; // Definición que establece la clave de la red

WebServer server(80);  // Instancia que controla las funciones del servidor WEB 
                       // y establece el puerto a 80 el cual es el Standart para las páginas WEB
const int led = LED_BUILTIN;  // Definición que establece el pin donde se conecta el LED

void handleRoot() {  // Método que envía la página WEB al cliente que lo solicite
  char temp[800];    // Se define una variable la cual, contendrá la página WEB en forma de texto
  int sec = millis() / 1000;  // Se define una variable que almacene los segundos a partir de que se inicia la ejecución del código
  int min = sec / 60; // Se define una variable que almacene el cálculo de los minutos
  int hr = min / 60;  // Se define una variable que almacene el cálculo de las horas
  snprintf(temp, 800, \
  /********************INICIA LA PAGINA WEB *********************************************************/
  "<html>\n\
  <head>\n\
    <meta http-equiv='refresh' content='5;URL=/'/>\n\
    <title>ESP32 Demo</title>\n\
    <style>\n\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\n\
    </style>\n\
  </head>\n\
  <body>\n\
    <h1>Hello from ESP32!</h1>\n\
    <p>Uptime: %02d:%02d:%02d</p>\n\
    <img src=\"/test.svg\" />\n\
    <p></p>\n\
    <form method=\"GET\" action=\"/led\" id=\"ledOnForm\">\n\
      <input type=\"submit\" name=\"ledBuiltin\" value=\"on\">\n\
    </form>\n\
    <form method=\"GET\" action=\"/led\" id=\"ledOffForm\">\n\
      <input type=\"submit\" name=\"ledBuiltin\" value=\"off\">\n\
    </form>\n\
  </body>\n\
</html>\n",
/******************************TERMINA LA PAGINA WEB ***********************************************/
           hr, min % 60, sec % 60  // Aquí se calcula del tiempo en que inicia la ejecución del código y los valores se incrustan en la línea 61.
          );
  server.send(200, "text/html", temp);  // Sentencia que envía la página web al cliente
}  // Termina método que envia la página WEB al cliente que lo solicitó.

void controlLed (void){  // Metodo que controla el estado del led incorporado en elNodeMCU
    for ( uint8_t i = 0; i < server.args(); i++ ) { // Se analiza el número de argumentos
    Serial.print (server.argName ( i ));  // Se imprime en el monitor serial el nombre del argumento recibido
    Serial.print (": ");  // Se imprime el separador de nombre y valor
    Serial.println (server.arg ( i ));  // Se imprime al valor del argumento
    if ( server.argName ( i ).equalsIgnoreCase("ledBuiltin") ) {  // Se compara cual es el Led que se desea controlar
      if (server.arg ( i ).equalsIgnoreCase("on"))  // Si nombre del argumento es ledBuitin se verifica el valor del argumento
        digitalWrite(led, LOW);  // Si el nombre y el valor de argumento es el solicitado se cambia de estado el led que el usuario eligió
      else
        digitalWrite(led, HIGH); // Si el nombre y el valor de argumento es el solicitado se cambia de estado el led que el usuario eligió
    }
    }
    handleRoot();  // El servidor nos devuelve a la página principal.
}

void handleNotFound() {  //Metodo que devuelve al cliente el aviso de que no se encontró el recurso solicitado.
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

  if (MDNS.begin("esp32")) {  // Se configura la instancia para difución del nombre del dispostivo en la red.
    Serial.println("MDNS responder started");  // Si la configuración fué un éxito se mostrará esta leyenda en el monitor del puerto serie.
  }  // Si la configuración falló no se mostrará lo anterior en el monitor del puerto serie.

  server.on("/", handleRoot);  // Se establece el nombre del método a ejecutar cuando el cliente introduzca la direcion IP en el navegador.
  server.on("/test.svg", drawGraph); // Se establece el nombre del método a ejecutar cuando el cliente introduzca la dirección IP seguido de "/test.svg"
  server.on("/inline", []() {  // Se establece el método a ejecutar cuando el cliente introduzca la dirección IP seguido de "/inline"
    server.send(200, "text/plain", "this works as well");  // Cuando el cliente solicite el recurso "/inline" se mostrará esta leyenda en el navegador WEB
  });  // Termina el método a ejecutar cuando el cliente introduzca la dirección IP seguido de "/inline"
  server.on("/led", controlLed);  // Se añade un nuevo recurso para controlar el led incorporado del NodeMCU
  server.onNotFound(handleNotFound);  // Se establece el nombre del método a ejecutar cuando el cliente introduzca la dirección IP seguido de un recurso NO declarado anteriormente.
  server.begin();  // Se inicia las funciones del servidor.
  Serial.println("HTTP server started");  // Se imprime la leyenda en el monitor de puerto serial.
}

void loop(void) {  // Método principal que se ejecuta indefinidamente
  server.handleClient();  // Se atiende a los clientes conectados (si los hay)
}  // Fin del método principal que se ejecuta indefinidamente

void drawGraph() {  // Método que dibuja una gráfica tipo Scalable Vector Graphics (Gráfica vectorial escalable)
  String out = "";  // Se define la variable que contendrá la gráfica vectorial en forma de texto.
  char temp[100];  // Se define una variable auxilar que "le dará formato de texto" a los valores contenidos en la variables x,y
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";  // Se define el encabezado de la gráfica vectorial
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n"; // Se define el rectángulo que contendrá la gráfica vectorial
  out += "<g stroke=\"black\">\n";  // Se define el inicio del paquete que contendrá las líneas de la gráfica
  int y = rand() % 130;  // Se define aleatoriamente un punto en el eje "Y" (punto inicial de la recta)
  for (int x = 10; x < 390; x += 10) {  // Inicia el bucle que establece valores al eje "X" y edita las información de cada línea en formato de texto
    int y2 = rand() % 130; // Se define aleatoriamente un punto en el eje "Y2" (punto final de la recta)
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);  // Se establece las coordenadas de cada línea en formato de texto
    out += temp;  // La variable auxiliar que contiene la información de una línea (calculada en la linea anterior) se suma a la variable que se vá a enviar.
    y = y2;  // Se asigna un nuevo valor para el punto "Y2"
  }  // Fin del bucle que establece valores al eje "X" y edita las información de cada línea en formato de texto
  out += "</g>\n</svg>\n";  // Se define el fin del encabezado de la gráfica vectorial

  server.send(200, "image/svg+xml", out);  // Se envia al cliente la gráfica vectorial con formato de texto.
} // Fin del Método que dibuja una gráfica tipo Scalable Vector Graphics (Gráfica vectorial escalable)
