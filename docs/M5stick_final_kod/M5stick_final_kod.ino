#include <M5StickCPlus.h>

// Noen nødvendige variabel
float accX, accY, accZ, magnitude; 
float maxVerdi, minVerdi;
float dif;

// Grensen for differansen mellom den største verdien og den minste verdien. Når denne grensen overskrides, går enheten fra inaktiveT til aktiv modus.
const float difGrense = 0.05; 
int count;
unsigned long timer = 0;   // Tidsteller for perioden for endring fra inaktiv til aktiveT modus
unsigned long mellomTid = 10000; // Tidsintervallet i ms, til å bytte fra inaktiv til aktive modus
unsigned long tid = 0;
unsigned long aktiveT = 0; // Tid ut senga
unsigned long inaktiveT = 0; // Tid i senga

bool aktivModus = false;  
bool systemStart = false;


// Denne funksjonen beregner differansen mellom den høyeste og laveste verdien av akselerometeret for 1000 avlesninger. 
float finneDifAcc(){
  // Den henter akselerometerdata fra IMU-en og beregner en enkelt verdi som representerer den totale akselerasjonshastigheten målt i akselerometeret.
  // Dette gjøres ved å beregne kvadratroten av summen av kvadratene til accX, accY og accZ.
  M5.IMU.getAccelData(&accX, &accY, &accZ);  
  magnitude = sqrt(accX * accX + accY * accY + accZ * accZ); 
  maxVerdi = magnitude;
  minVerdi = magnitude;
  count=0 ;

  // Starter en løkke som vil kjøre 1000 ganger for å ta tusen avlesninger og finner den høyeste og laveste verdien.
  while(count<1000){
    sjekkeKnapp(); // sjekker om det er noen knappetrykk
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    magnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
    if (magnitude > maxVerdi) {
      maxVerdi = magnitude;}
    if (magnitude < minVerdi) {
      minVerdi = magnitude;}
      count++;
      delay(1);
  }

  // return deferensen mellom den største og minste verdien.
  return maxVerdi - minVerdi;
}

// Den tar to parametere den aktive og inaktiveTe tiden i millisekunder, 
// og konverterer den til timer, minutter og sekunder, og viser den på skjermen
void printResultat(unsigned long Aktiv,unsigned long inaktiveT){
 
  // Bestem størrelsen og fargen på skriften og hvor den aktive tiden skal skrives ut på skjermen
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setCursor(7, 20);
  M5.Lcd.printf("Akt %02d:%02d", Aktiv/3600000, ((Aktiv/1000)%3600) / 60);

  // Bestem størrelsen og fargen på skriften og hvor den inaktive tiden skal skrives ut på skjermen
  M5.Lcd.setTextColor(RED, BLACK);
  M5.Lcd.setCursor(7, 70);
  M5.Lcd.printf("InA %02d:%02d", inaktiveT/3600000, ((inaktiveT/1000)%3600) / 60);


  // Blinke  
  if (((Aktiv/1000)%2) == 0 && aktivModus){
    M5.Lcd.fillRect(155, 20, 15, 50, BLACK);
  }
  if (((inaktiveT/1000)%2) == 0 && !aktivModus){
    M5.Lcd.fillRect(155, 70, 15, 50, BLACK);
  }
}

// sjekker om det er noen knappetrykk (BtnA og BtnB )
void sjekkeKnapp(){
  M5.update();
  //  Hvis knapp_A trykkes, skal systemet ta pause til at den trykkes engang til.
  if (M5.BtnA.wasReleased()){
    if(systemStart){
      systemStart = false;
      M5.Lcd.setTextSize(3);
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.setCursor(60, 103);
      M5.Lcd.print("pause ");
    }
    else {
      systemStart = true;
      M5.Lcd.fillRect(50, 100, 200, 60, BLACK);
    }
  }
  
  // Hvis knapp_A trykkes holdes inne i 3 sekunder, skal den resette aktiv og inaktiveT tiden til null
  else if (M5.BtnA.wasReleasefor(3000)) {  
    aktiveT = 0;
    inaktiveT = 0;} 

}


void setup() {
  // utfører nødvendig initialisering for å bruke M5Stack-enheten
  M5.begin();             
  M5.Imu.Init();         
  M5.Lcd.setRotation(3);  // Setter ønsket rotasjonen for skjermen (3 * 90 grader)
  M5.Lcd.fillScreen(BLACK); // Fyller hele LCD-skjermen med svart farge for å tømme skjermen
  M5.Lcd.setTextSize(4); // størrelsen på teksten
  delay(100);
  systemStart = true;
}


void loop() {
  sjekkeKnapp();
  if (systemStart){
    tid = millis();
    dif = finneDifAcc(); // kaller funksjonen som finner akselerometeret differansen

    // Denne løkkeen kjører så lenge verdien av differansen er mindre enn difGrense. 
    // Tiden som brukt på denne løkken beregnes og legges til aktiv tiden. (Pasienten er ikke i seng)
    while(dif < difGrense && systemStart){
      aktivModus = true;
      aktiveT += millis() - tid;
      tid = millis();
      printResultat(aktiveT, inaktiveT);
      dif = finneDifAcc();
    }

    // løkken kjører så lenge verdien av timer er mindre enn mellomTid (tiden som enheten skal vente for den bytter til aktiv modus).
    // Tiden som brukt på denne løkken beregnes og legges til inaktiveT tiden. (Pasienten er i seng)
    while(timer < mellomTid && systemStart){
      aktivModus = false;
      // Når enheten leser en uvanlig bevegelse, betyr det at pasienten fortsatt er i sengen, og timer vil starte igjen fra null
      if(dif > difGrense){
          timer = 0;}
      else{
        timer += millis() - tid;
      }
      inaktiveT += millis() - tid;
      tid = millis();
      printResultat(aktiveT, inaktiveT);
    dif = finneDifAcc();
    }

    // Tiden som enheten ventet for den bytter til aktiv modus skal trykkes av inaktiveT tid og addere den til aktivtid.
    inaktiveT = inaktiveT - timer;
    aktiveT = aktiveT + timer;
    timer = 0;
  }
}
