/**************************************************************************
 Este ejemplo fué desarrollado para comprender
 la implementación de dos hardware direrentes:
 el sensor DHT22 y la pantalla oled SSD1306
 **************************************************************************/

// Bibliotecas necesarias para la pantalla
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Bibliotecas necesarias para el sensor DHT22
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Declaraciones necesarias para la pantalla
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_DC     21  // Número de PIN donde se conecta la terminal DC de la pantalla
#define OLED_CS     22  // Número de PIN donde se conecta la terminal CS de la pantalla
#define OLED_RESET  4   // Número de PIN donde se conecta la terminal RST de la pantalla


// Declaraciones necesarias para el sensor DHT22
#define DHTPIN 32     // Digital pin connected to the DHT sensor 

#define DHTTYPE    DHT22     // DHT 22 (AM2302)

/*
 * Los constructores son los accesos a las funciones contenidas
 * en las bibliotecas. Para poder ver las funciones contenidas
 * en las bibliotecas es necesario ver el *.h de la biblioteca 
 * correspondiente
 */
// Constructor para la pantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

// Constructor para el sensor DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// Declaración de las variables necesarias para el sensor DHT22
sensors_event_t event;  // Se almacena la instancia (direccion en memoria) de las funciones del sensor
float Temperatura;  // Almacena el valor de la temperatura (limitado a dos decimales)
float Humedad;  // Almacena el valor de la humedad (limitado a dos decimales)

// Declaración de las variables necesarias para la máquina de estados
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 250;         // interval at which to change state (milliseconds)
unsigned int estadoLed;            // Almacena el número de estado (0 ~ 3)

void setup() {

  // Inicializando el puerto serie
  Serial.begin(9600);

  // Inicializando el display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Se muestra el LOGO del fabricante en pantalla
  display.display();
  delay(2000); // Pause for 2 seconds

  // Inicializando el sensor DHT22
  dht.begin();

  // Se borra el display
  display.clearDisplay();

  // Se muestra el nombre del sensor en la parte superior de la pantalla
  // display.fillRect(CoordEjeX, CoordEjeY, AnchoCaracter * NumCaracter * TamanoTexto, AlturaCaracter * TamanoTexto, Color);
  /*  No olvidar que el tamaño del texto standart es 5 * 7 pixeles,
   *  se anade un pixel adicional por la separación de caracteres 
   *  quedando en 6 pixeles de ancho * 8 pixeles de alto
   */
  display.fillRect(22, 0, 6 * 5 * 1, 8 * 1, SSD1306_BLACK);  // Se borra el texto anterior de 14 caracteres
  //display.setFont(&FreeMono9pt7b);
  display.setTextSize(1); // Se elige el tamaño del texto (3X)
  display.setTextColor(SSD1306_WHITE); // Se elige el color del texto (blanco)
  display.setCursor(22, 0);  // Se elige las coordenadas donde se coloca el texto

  display.print("SENSOR DIGITAL");  // Se coloca en memoria el texto
  display.display();      // Se muestra el texto en pantalla


}

void loop() {
  
  // variables locales necesarias para la máquina de estados
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    estadoLed++;
    if (estadoLed > 3)  // Los valores que puede tomar la variable 
       estadoLed = 0;   // estadoLed son: 0, 1, 2 y 3.
      // do something different depending on the range value:
    switch (estadoLed) {
      case 0:  // Se obtiene la temperatura
        // Get temperature event and print its value.
        dht.temperature().getEvent(&event);  // Se captura el valor de la temperatura instantánea
        if (isnan(event.temperature)) {
          Temperatura = -273;
          Serial.println(F("Error reading temperature!"));
        } else {
          Serial.print(F("Temperature: "));
          Temperatura = event.temperature;  // Se tranfiere el valor de la temperatura a una variable
          Serial.print(Temperatura);
          Serial.println(F("°C"));
        }
        
        break;
      case 1:  // Se obtiene la humedad
        // Get humidity event and print its value.
        dht.humidity().getEvent(&event);  // Se captura el valor de la humedad instantánea
        if (isnan(event.relative_humidity)) {
          Humedad = 0.0;
          Serial.println(F("Error reading humidity!"));
        } else {
          Serial.print(F("Humidity: "));
          Humedad = event.relative_humidity;  // Se tranfiere el valor de la temperatura a una variable
          Serial.print(Humedad);  
          Serial.println(F("%"));
        }
  
        break;
      case 2:  // Se muestra la temperatura en pantalla
        // display.fillRect(CoordEjeX, CoordEjeY, AnchoCaracter * NumCaracter * TamanoTexto, AlturaCaracter * TamanoTexto, Color);
        /*  No olvidar que el tamaño del texto standart es 5 * 7 pixeles,
         *  se anade un pixel adicional por la separación de caracteres 
         *  quedando en 6 pixeles de ancho * 8 pixeles de alto
         */
        display.fillRect(19, 16, 6 * 5 * 3, 8 * 3, SSD1306_BLACK);  // Se borra el texto anterior de 5 caracteres 
        //display.setFont(&FreeMono9pt7b);                          // dos enteros + punto decimal + dos decimales
        display.setTextSize(3); // Se elige el tamaño del texto (3X)
        display.setTextColor(SSD1306_WHITE); // Se elige el color del texto (blanco)
        display.setCursor(19, 16);  // Se elige las coordenadas donde se coloca el texto
  
        display.print(Temperatura);  // Se coloca en memoria el texto
        display.display();      // Se muestra el texto en pantalla
   
        break;
      case 3:    // Se muestra la humedad en pantalla
        // display.fillRect(CoordEjeX, CoordEjeY, AnchoCaracter * NumCaracter * TamanoTexto, AlturaCaracter * TamanoTexto, Color);
        /*  No olvidar que el tamaño del texto standart es 5 * 7 pixeles,
         *  se anade un pixel adicional por la separación de caracteres 
         *  quedando en 6 pixeles de ancho * 8 pixeles de alto
         */
        display.fillRect(49, 48, 6 * 5 * 1, 8 * 1, SSD1306_BLACK);  // Se borra el texto anterior de 5 caracteres
        //display.setFont(&FreeMono9pt7b);                          // dos enteros + punto decimal + dos decimales
        display.setTextSize(1); // Se elige el tamaño del texto (3X)
        display.setTextColor(SSD1306_WHITE); // Se elige el color del texto (blanco)
        display.setCursor(49, 48);  // Se elige las coordenadas donde se coloca el texto
  
        display.print(Humedad);  // Se coloca en memoria el texto
        display.display();      // Se muestra el texto en pantalla
        break;
      default:    // Sólo se ejecuta cuando 0 > estadoLed > 3
        estadoLed = 0;
       break;
    }  // Fin del switch (estadoLed)
  }  // Fin del condicional if (currentMillis - previousMillis >= interval)
}  // Fin del método loop()
