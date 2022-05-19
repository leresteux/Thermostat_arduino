// non utilisées
/*#include <Chrono.h>
  //https://github.com/SofaPirate/Chrono
  Chrono Chrono_relais;
*/
//Libraries
#include <DHT.h>;
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <everytime.h>
//https://github.com/fesselk/everytime
//pas top


#include <DS3231.h>// 
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/

//Constants


//hardware
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//Pins
#define DHTPIN 7
#define RELAISPIN 2
#define LedRPIN 13
#define LedVPIN 12
#define POTAPIN A0
#define Btn 3
// temperature et horaire
#define tempmini 12
#define tempmax 24
//semaine
#define starttime1 6
#define starttime2 16
#define ecotime1 8
#define ecotime2 14
#define ecotime3 23
//week-end

#define WE_starttime1 9
//Fonctions

DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display
// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
// Init a Time-data structure
Time  t;

//Variables



float temp, lasttemp, lasttempwanted, valpota, lastvalpota; //Stores temperature value
float tempwanted = 20;

float tempauto = 20;
float tempeco = 16;

unsigned int old_t_sec = 0;
unsigned int val_menu = 0;
unsigned int pausse = 1000;
unsigned int ajustementtemp = 1;
float tolerancetemp = 0.5;
boolean etatchaudiere = false;
boolean etatchaudierewanted;
boolean mode_auto_up = false ;
boolean mode_auto_down = false;
boolean modemanuel = false;

void setup() {
  // See http://playground.arduino.cc/Main/I2cScanner how to test for a I2C device.
  pinMode(RELAISPIN, OUTPUT);
  pinMode(LedRPIN, OUTPUT);
  pinMode(LedVPIN, OUTPUT);

  // Initialize the rtc object
  rtc.begin();

  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(SATURDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 4, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(01, 02, 2020);   // Set the date to January 1st, 2014

  dht.begin();
  Wire.begin();
  Wire.beginTransmission(0x27);
  Serial.begin(9600);

  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.noBlink();
  lcd.noCursor();

  delay (100);


  // Envoi du message
  lcd.setCursor(0, 0);
  lcd.print("Thermuino by");
  lcd.setCursor(0, 1);
  lcd.print("Fabricol.be ");
  delay(2000);

  //  Chrono_relais.restart(0); // pas utilisé
  //  printChrono();


}

void loop() {


  serialPrint();
  timer();
  delay(100);
  prisetemperature();
  delay(100);
  reglagetemp();
  delay(100);
  every(10000) {
    chaudiere();
  }
  delay(100);
  // Menu();//wip
  //delay(100);

  //info_auto_affichage();

  // Bouton();
}

void prisetemperature() {

  //Read data and store it to variables hum and temp
  temp = dht.readTemperature();
  //test aroundir à 0.5 voir : https://www.arduino.cc/reference/en/language/variables/data-types/float/
  temp = temp * 2;
  temp = round (temp);
  temp = (temp / 2) - ajustementtemp;
  //---
  if (temp != lasttemp) {
    lasttemp = temp;
    affichage();
  }
}
void serialPrint() {

  /*Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" / ");
    Serial.println(tempwanted);
    if (etatchaudiere) {
    Serial.println("Chaudiere: ON");
    }
    else {
    Serial.println("Chaudiere: OFF");
    }
  */
  Serial.print("Temperature rtc: ");
  Serial.print(rtc.getTemp());
  Serial.println(" C");
  Serial.print("Temperature dht: ");
  Serial.print(dht.readTemperature());
  Serial.println(" C");
  delay (1000);
}
void affichage() {

  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print(t.hour, DEC);
  lcd.print(":");
  lcd.print(t.min, DEC);
  lcd.print(" -");
  lcd.print(t.date, DEC);
  lcd.print("/");
  lcd.print(t.mon , DEC);
  lcd.print("-");
  
  lcd.print(t.dow);
  
  lcd.setCursor(0, 1);

  if (etatchaudiere) {
    lcd.print("ON  ");
  }
  else {
    lcd.print("OFF ");
  }
  lcd.print("T:");
  lcd.print(temp);
  lcd.setCursor(11, 1);
  lcd.print(" / ");
  lcd.print(int (tempwanted));

  lcd.noBlink();
  lcd.noCursor();
}

void chaudiere() {
  /*if (Chrono_relais.hasPassed(10000)) { //verifie toute les 10 secondes en test
    Chrono_relais.stop();
  */
  if ((temp + tolerancetemp) < tempwanted && etatchaudiere == false)
  { digitalWrite(LedRPIN, HIGH);
    digitalWrite(LedVPIN, LOW);
    digitalWrite(RELAISPIN, LOW);
    etatchaudiere = true;
  }

  else if ((temp - tolerancetemp) >= tempwanted && etatchaudiere == true) {
    digitalWrite(LedVPIN, HIGH);
    digitalWrite(LedRPIN, LOW);
    digitalWrite(RELAISPIN, HIGH);
    etatchaudiere = false;
  }
  // Chrono_relais.restart();
  affichage();
  //}
}

void reglagetemp() {
  valpota = analogRead(POTAPIN);

  if (valpota > lastvalpota + 100 || valpota < lastvalpota - 100) //si l'on touche le pota
  {
    lastvalpota = valpota;
    delay(100);
    tempwanted = map (valpota, 0, 1023, tempmini, tempmax);
    //test aroundir à 0.5 voir : https://www.arduino.cc/reference/en/language/variables/data-types/float/
    tempwanted = tempwanted * 2;
    tempwanted = round (tempwanted);
    tempwanted = tempwanted / 2;
    delay(100);
    if (tempwanted != lasttempwanted) {
      lasttempwanted = tempwanted;
      affichage();

    }
  }
}
void timer() {
  // Get data from the DS3231
  t = rtc.getTime();
  delay(100);
  //affichage toutes les secondes
  if (old_t_sec != t.sec) {
    affichage();
    Serial.println("nouvelle affichage");
    old_t_sec = t.sec;
  };
  //week end


  //passage en mode tempauto
  if (mode_auto_up == false) {
    //week-end
    if (t.dow == SATURDAY || t.dow == SUNDAY ) {
      Serial.println("it s week end");
      if (t.hour == WE_starttime1) {
        tempwanted = tempauto;
        modemanuel = false;
        mode_auto_up = true;
        mode_auto_down = false;
      }
    }
    //semaine
    else if (t.hour == starttime1 || t.hour == starttime2  ) {
      tempwanted = tempauto;
      modemanuel = false;
      mode_auto_up = true;
      mode_auto_down = false;
    }
  }
  else if (mode_auto_down == false) {
    if (t.hour == ecotime1 || t.hour == ecotime2 || t.hour == ecotime3) {
      tempwanted = tempeco;
      modemanuel = false;
      mode_auto_down = true;
      mode_auto_up = false;
    }
  }
}

/*
  void info_auto_affichage() {
    if (t.sec == 0) { // toutes les minutes
      lcd.home();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("StartTime: ");
      lcd.setCursor(0, 1);
      lcd.print(starttime1);
      lcd.print(" & ");
      lcd.print(starttime2);
      delay(5000);
      lcd.setCursor(0, 0);
      lcd.print("EcoTime: ");
      lcd.setCursor(0, 1);
      lcd.print(ecotime1);
      lcd.print(", ");
      lcd.print(ecotime2);
      lcd.print(" & ");
      lcd.print(ecotime3);
      delay(5000);
      affichage();
    }
  }

*/
// wip menu
/*void Bouton() {
  Serial.println("bouton fonction ok");
  if (digitalRead(Btn)) {
    Serial.println("btn push");
    // verifier
    delay (1000);
    if (digitalRead(Btn)) {
      lcd.home();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(".");
      delay (500);
      if (digitalRead(Btn)) {
        lcd.home();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("..");
        delay (500);
        if (digitalRead(Btn)) {
          lcd.home();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("...");
          delay (500);
          if (digitalRead(Btn)) {
            lcd.home();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("...ok");
            delay (500);
            val_menu++;
            if (val_menu > 2)// nombre sous-menu
            {
              val_menu = 0;
            };

          };
        };
      }
    }
  }
  Menu ();
  }


  void Menu () {
  if (val_menu == 1) {

    while (val_menu == 1) { // verifier
      lcd.home();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("semaine: 8h-14h-23h");
      lcd.setCursor(0, 1);
      lcd.print("mini: ");
      lcd.setCursor(6, 1);
      lcd.print(tempmini);
      change_val(tempmini);
      Bouton();

      // afficher menu reglage
    };
  }
  else if (val_menu == 2) {
    while (val_menu == 2) { // verifier
      lcd.home();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("semaine: 6h-16h");
      lcd.setCursor(0, 1);
      lcd.print("Max: ");
      lcd.setCursor(5, 1);
      lcd.print(tempmax);
      change_val(tempmax);
      Bouton();

    }
  };
  };

  void change_val(int val) {

  valpota = analogRead(POTAPIN);

  if (valpota > lastvalpota + 100 || valpota < lastvalpota - 100) //si l'on touche le pota
  {
    lastvalpota = valpota;

    val = map (valpota, 0, 1023, 8, 25);
    //test aroundir à 0.5 voir : https://www.arduino.cc/reference/en/language/variables/data-types/float/
    val = val * 2;
    val = round (val);
    val = val / 2;
  }
  }
*/
/*
  // code pour librairie chrono chrono
  if (Chrono_relais.hasPassed(10000)) {
  Serial.println("Stop 10 sec");
  Chrono_relais.stop();
  Chrono_relais.restart(0);
  }
*/
