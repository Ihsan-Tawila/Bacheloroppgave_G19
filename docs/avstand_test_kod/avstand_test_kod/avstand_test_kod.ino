// Koden er ikke komplett ennå. Vi sluttet å utvikle koden etter at vi fikk kommentarer fra sykepleiere på sykehuset
// om at de foretrakk M5 Stick. Koden mangler funksjonaliteten til knappene. Ellers fungerer alt som det skal.

// Importer de nødvendige bibliotekene for å definere avstandssensoren og OLED-skjermen og deres relaterte funksjoner
#include "Adafruit_VL53L0X.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
#define RST_PIN -1

// Definere avstandssensoren og OLED-skjermen
SSD1306AsciiWire oled;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

//Noen nødvendige variabler for å finne avstand og gjennomsnitt
int gjennomsnitt = 0;
int terskel = 0; // den grense avstand som skal bestemme om det finnes noen på senge eller ikke
const int antalAvlesninger = 100;  // antall avlesninger som skal brukes i rullende gjennomsnitt
int totalAvstand = 0;   
int n = 0;        // current index in sample array

//Noen nødvendige variabler for å finne inaktiv og aktiv tid
unsigned long tid = 0;
unsigned long aktiv = 0;
unsigned long inaktiv = 0;

// Den tar to parametere den inaktiv og aktiv tiden i millisekunder, 
// og konverterer den til timer, minutter og sekunder, og viser den på skjermen
void printResultat(unsigned long inaktiv,unsigned long aktiv){
  oled.clear();
  oled.setFont(TimesNewRoman13);
  oled.setCursor(20, 20);
  oled.println("Activity Time:");
  oled.setFont(TimesNewRoman16_bold);
  oled.setCursor(35, 40);
  oled.printf("%02d:%02d:%02d", aktiv/3600000, ((aktiv/1000)%3600) / 60, (aktiv/1000) % 60);

  oled.setCursor(1, 60);
  oled.println("");

  oled.setFont(TimesNewRoman13);
  oled.setCursor(20, 60);
  oled.println("Bed-Rest Time: ");
  oled.setFont(TimesNewRoman16_bold);
  oled.setCursor(35, 80);
  oled.printf("%02d:%02d:%02d", inaktiv/3600000, ((inaktiv/1000)%3600) / 60, (inaktiv/1000) % 60);
  }

// den tar /antalAvlesninger/ avlesning av sensoren og finner gjennomsnitten 
int gjennomsnitt_avstand(){
  totalAvstand = 0;
  n = 0;
  while(n < antalAvlesninger){
    totalAvstand += lox.readRange();
    n++;
    delay(1);
  }
  return totalAvstand / n;
}


// Den kjører en gang ved oppstart av programmet
// Den sette opp og initialisere forskjellige komponenter og variabler som vi trenger for å kjøre programmet.
void setup() {
    // Vent på seriell tilkobling før programmet starter
  Serial.begin(115200);
    while (! Serial) {
    delay(1);
  }
  Serial.println("Adafruit VL53L0X test.");

  // Start LIDAR-sensor og sjekk at den fungerer
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  lox.startRangeContinuous();

  // Initialiser OLED-skjermen
  Wire.begin();
  Wire.setClock(400000L);
  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0

  oled.setFont(TimesNewRoman16_bold);
  oled.setLetterSpacing(2);
  oled.clear();
  oled.println("..... vent ...");

//finn terskel for avstand
  int n = 0;
  int sum = 0;
  while(n < 300){
      sum += lox.readRange();
      n++;
  }
  terskel = sum/n;
}


void loop() {
  printResultat(inaktiv, aktiv);
  tid = millis();
  gjennomsnitt = gjennomsnitt_avstand();  

  // Så lenge avstanden mellom sengen og gulvet er større enn terskelen, skal telle aktivitetstiden (pasienten er ute av sengen)
  while (gjennomsnitt > terskel-5) {
    gjennomsnitt = gjennomsnitt_avstand();
    aktiv += millis() - tid;
    tid = millis();
    printResultat(inaktiv, aktiv);
  }
    
  //  Så lenge avstanden mellom sengen og gulvet er mindre enn terskelen, skal telle inaktivitet (pasienten er i sengen)
  while (gjennomsnitt <= terskel-5) {
    gjennomsnitt = gjennomsnitt_avstand();
    inaktiv += millis() - tid;
    tid = millis();
    printResultat(inaktiv, aktiv);
  }
  }
