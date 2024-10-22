/*el circutio correspondiente a este código se puede encontrar en el siguiente repositorio: 
https://github.com/leonardoaranda1981/tallerNubes-y-antenas
*/
//importación de las librerias utilizadas por el programa
#include <TinyGPSPlus.h>
#include "WiFi.h"
#include <SPI.h>
#include <SD.h>
#include "FS.h"

///////////////variables GPS////////////////////
static const int RXPin = 16, TXPin = 17;//pines de comunicación serial entre el GPS y el microcontrolador
static const uint32_t GPSBaud = 4800;//velocidad de comunicación
TinyGPSPlus gps;//nueva instancia de la libreria GPS
HardwareSerial gpsSerial(2);//Creación de un objeto de comuicación serial

//////////variables de asignación de pines para los LEDs//////
int led_fail = 26;
int led_wifi = 27;
int led_gps = 14;
int led_sd = 12;


void setup() {
  /*setput de los pines para los LEDS como outputs de corriente*/
  pinMode(led_fail, OUTPUT);
  pinMode(led_wifi, OUTPUT);
  pinMode(led_gps, OUTPUT);
  pinMode(led_sd, OUTPUT);

  Serial.begin(115200);//inicalización de la comunicación serial entre el microcontrolador y el monitor de Arduni
  while (!Serial) {
    delay(10);
  }

   

  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);//inicialización del GPS

  delay(5000);//Espera 5 segundos

  WiFi.mode(WIFI_STA);//inicilización del wi-fi dentro del microcontrolador
  WiFi.disconnect();//desconectarse de conexiones previas
  delay(100);//espera 100 milisegundo
  Serial.println("Wifi Setup done");//imprime a consola el mensaje "wifi setup done"


#ifdef REASSIGN_PINS ////inicialización del protocolo SPI por donde estableceremos comunicación con la tarjeta SD
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {//intenta iniciar la tarjeta SD
#else
  if (!SD.begin()) {//intenta iniciar la tarjeta SD por otro metodo
#endif
    Serial.println("Card Mount Failed");//imprime el mensaje "card mount failed si falla la inicialización
    return;l
  }

}

void loop() {
/*apaga los LEDS al inicio de cada iteración del programa*/
  digitalWrite(led_fail, LOW);
  digitalWrite(led_wifi, LOW);
  digitalWrite(led_gps, LOW);
  digitalWrite(led_sd, LOW);

  while (gpsSerial.available() > 0) {//revisa la disponibilidad del GPS
    if (gps.encode(gpsSerial.read())) {//lee el mensaje actual del GPS y revisa si es posible decodificarlo
      if (gps.location.isValid()) {//revisa si las coordenadas obtenidas del GPS son validas
        digitalWrite(led_gps, HIGH);//en caso de pasar todas las revisiones, enciende el led_gps (pin 14)
        
        Serial.println("Scan start");//escribe el mensaje "scan start" en consola
        int initialRSSI = -100;//crea una varibale con el valor minimo posible para el RSSI
        int n = WiFi.scanNetworks();//escanea todas las redes wi-fi disponibles y regresa el valor del numero de redes encontradas

        if (n == 0) {//si las redes encontradas es = 0
          Serial.println("no networks found");//imprime a consola el mensaje "no networks found"
          digitalWrite(led_wifi, LOW);//apaga el led_wifi
          digitalWrite(led_fail, HIGH);//enciende el led_fail
        } else {//de lo contrario
          digitalWrite(led_wifi, HIGH);//enciende el led_wifi
          digitalWrite(led_fail, LOW);//apaga el led_fail

          String mensaje = "";//crea una cadena de texto vacia
          mensaje + '\n';
          double latitud = gps.location.lat();//extrae la latitud del GPS
          double longitud = gps.location.lng();//extrae la longitud del GPS

          mensaje += String (latitud, 6);//añade la latitud a la cadena de texto
          mensaje += ",";//añade una coma a la cadena de texto
          mensaje += String (longitud, 6);//añade la longitud a la cadena de texto
          mensaje += ",";//añade una coma a la cadena de texto

          char sz[32];//crea una lista vacia de 32 caracteres
          sprintf(sz, "%02d/%02d/%02d", gps.date.month(), gps.date.day(), gps.date.year());//llena la cadena de caracteres con el mes, día y fecha del gps
          mensaje += sz;//añade la cadena de caracteres a la cadena de texto del mensaje
          mensaje += ",";//aña una coma a la cadena de texto

          char st[32];//crea una lista vacia de 32 caracteres
          sprintf(st, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());//llena la cadena de caracteres con la hora, minuto y segundo del gps
          mensaje += st;//añade la cadena de caracteres a la cadena de texto del mensaje
          mensaje += ",";//añade una coma a la cadena de texto

          for (int i = 0; i < n; ++i) {//crea una ciclo iterativo que se repetira tantas veces como redes haya encontrado
            int RSS = WiFi.RSSI(i);//extrae el valor de RSSI de cada red encontrada
            if (RSS > initialRSSI) {//compara el valor actual de RSSI con el valor de la variable initialRSSSI
              initialRSSI = RSS;//Si el valor es mayor, substituye el valor de initialRSSI por el valor actual de RSSI
            }
            // al final del ciclo iterativo habremos obtenido el valor máximo de RSSI 
            delay(10);//espera 10 milisegundo
          }
          mensaje += initialRSSI;//añade el valor máximo de RSSI a la cadena de texto

          int mensaje_len = mensaje.length() + 1;//guarda el total de caracteres de nuestro mensaje de texto
          char mensaje_array [mensaje_len];//crea una lista vacia de caracteres con el numero de caracteres del mensaje
          mensaje.toCharArray(mensaje_array, mensaje_len);//llena la lista de caracteres con la cadena de texto del mensaje
          appendFile(SD, "/datalog.csv", mensaje_array);//invoca la función appendFile para añadir nuestro mensaje al archivo datalog.csv
          Serial.println(mensaje);//imprime en la consola el mensaje generado
        }

        WiFi.scanDelete();//borra las redes encontradas
      } else {
        Serial.println(F("INVALID"));//en caso de que la localización del GPS nos se valida, imprime el texto "INVALID"
        digitalWrite(led_gps, LOW);//apaga el led_gps
        digitalWrite(led_fail, HIGH);//enciende el led_fail
      }
    }
  }
  delay(5000);//espera 5 segundos

  if (millis() > 5000 && gps.charsProcessed() < 10) {//si el programa se ha ejectudado por 5 segundo y loscaracteres recibidos del GPS son menor a 10
    Serial.println(F("No GPS data received: check wiring"));//imprime el mensaje "No GPS data received"
    digitalWrite(led_gps, LOW);//apaga el led_gps
    digitalWrite(led_fail, HIGH);//enciende el led_fail
    while (true);
  }
  //Serial.println("-------------------------------");
}



void appendFile(fs::FS &fs, const char *path, const char *message) {//decalra la función appenFile
  Serial.printf("Appending to file: %s\n", path);//imprime el mensaje "appending to file" + el nombre del archivo

  File file = fs.open(path, FILE_APPEND);//abre el archivo
  if (!file) {//si no logra abrir el archivo
    Serial.println("Failed to open file for appending");//imprime el mensaje "failed to open dile for appending"
    digitalWrite(led_sd, LOW);//apaga el led_sd
    digitalWrite(led_fail, HIGH);//enciende el led_fail
    return;
  }
  if (file.print(message)) {//si logra escribir la linea en el archivo
    file.print('\n');//salta de linea
    Serial.println("Message appended");//imprime a consola el mensaje "message appended"
    digitalWrite(led_sd, HIGH);//apaga el led_sd
    digitalWrite(led_fail, LOW);//enciende el led_fail
  } else {//de lo contrario
    Serial.println("Append failed");//imprime el mensaje "append failed" a consola
    digitalWrite(led_sd, LOW);//apaga el led_sd
    digitalWrite(led_fail, HIGH);//enciende el led_fail
  }
  file.close();//cierra el archivo
}

/*
Este programa crea una archivo csv a partir de la lectura de las coordenadas de un GPS y la lectura de señales wifi, creando lineas como la siguiente: 
19.424767,-99.128190,10/13/2024,00:31:32,-67
*/
