// Inicio del código de la máquina de estados
// constants won't change. Used here to set a pin number:
// Conecte los leds como sigue:
const int ledR =  12;// the number of the LED pin (rojo)
const int ledG =  14;// the number of the LED pin (verde)
const int ledB =  27;// the number of the LED pin (azul)

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 100;           // interval at which to blink (milliseconds)
int estadoLed = 0;

void setup() {
  // set the digital pin as output:
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    estadoLed++;
    if (estadoLed > 3)  // Los valores que puede tomar la variable 
       estadoLed = 0;   // estadoLed son: 0, 1, 2 y 3.

    // do something different depending on the range value:
  switch (estadoLed) {
    case 0:    // todos los leds apagados
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      break;
    case 1:    // sólo el led rojo se enciende
      digitalWrite(ledR, LOW);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      break;
    case 2:    // sólo el led verde se enciende
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, LOW);
      digitalWrite(ledB, HIGH);
      break;
    case 3:    // sólo el led azul se enciende
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, LOW);
      break;
    default:    // Sólo se ejecuta cuando 0 > estadoLed > 3
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      estadoLed = 0;
     break;
  }

  }
} //Fin del código de la maquina de estados
