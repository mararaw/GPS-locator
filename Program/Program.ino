#include <SoftwareSerial.h>
#include <ArduinotechGSMShield.h>
#include <TinyGPS.h>
#define RX_GPS 16
#define TX_GPS 15
#define RX_SIM800 5
#define TX_SIM800 4
#define RX_BT 10
#define TX_BT 11

SoftwareSerial gpsSerial(RX_GPS, TX_GPS);
SoftwareSerial GSM_modul::SIM800(RX_SIM800,TX_SIM800);
SoftwareSerial bluetooth(RX_BT, TX_BT);
TinyGPS gps;
GSM_modul sim800l;

void GPSmodul();
void incomeCallOrSMS();
void outgoingSMS();
void setDistance(String number, String textSMS);
void setNumber(String textSMS, bool main);
void setUserPassword(String textSMS);
void getPosition(String  number, bool BT);
void BlueTooth();
void incomeBlueTooth(String BTdata);
void setDistanceBT();
void setDistanceBT_0(bool BT);
void setUserNumberBT(String BTdata);
void powerOut();
void boxOpen();
void interrupCorrection();
void batteryVoltage(bool Setup);
bool overPosition();
bool GPSsignal_OkOrNot();
String SMSposition();
String SMSGPSsignal();
String SMSvoltageBat();
String SMSextPower();
String SMSboxCloseOpen();
String GSMmodulInformBT();
                      //GLOBÁLNÍ PROMĚNNÉ//
bool GPSsignal;                             // = true, GPS signal je OK
bool myDrive = false;                       // = true, jízdní režim
bool sentOverPos = false;                   // = true, pokud se odešle varovná SMS o překročení vzdálenosti
bool sentGPSsignalLost = false;             // = true, pokud se odešle varovná SMS o ztrátě GPS signálu
bool sentBatWarning = false;                // = true, pokud se odešle varovná SMS o napětí akumulátorů
bool boxInterrupt = false;                  // \
bool boxInterrupt2 = false;                 // \\
bool powerInterrupt = false;                // - pomocné proměnné při zpracování přerušení
bool powerInterrupt2 = false;               // /
bool stopLoop = false;                      // na zastavení smyčky loop
byte GPSsignalStatus = 0;                   // pomocná proměnná při zjišťtování aktuálnosti dat z GPS (GPS signal)  
byte numberOverPos = 0;                     // počet překročení maximální povolené vzdálenosti
float lat, lon, latS, lonS;                 // hodnoty zem. šířky a délky (aktuální a z minulého cyklu)
float bat1, bat1Last, bat2, bat2Last;       // hodnoty napětí akumulátorů (aktuální a z minulého cyklu)
float set_distance = 0.03;                  // nastavená maximální povolená vzdálenost
float null_distance = 0.03;                 // nastavená vzdálenost pro hodnotu 0 (je GPS signal)
float null_distanceNoSignal = 0.06;         // nastavená vzdálenost pro hodnotu 0 (není GPS signal)
unsigned long timeGPSsignalLost;            // čas, kdy došlo ke ztrátě GPS signálu (millis)
unsigned long timePowerOut;                 // čas, kdy došlo k odpojení napájení (millis)
unsigned long timeBoxOpen;                  // čas, kdy došlo k otevření krabičky (millis)
unsigned long timeBatteryRead;              // čas, kdy došlo k měření napětí akumulátorů (millis)
String mainNumber = "420775163548";         // hlavní tel. číslo
String userNumber = "420775163548";         // uživatelské tel. číslo
String mainPassword = "12345";              // heslo na nastavení hlavního tel. čísla
String userPassword = "00000";              // heslo na nastavení uživatelského tel. čísla
String GPStime;                             // aktuální čas z GPS
String GPStimeLast;                         // čas z GPS z minulého cyklu



void setup() {
  Serial.begin(9600);
  GSM_modul::SIM800.begin(9600);
  gpsSerial.begin(9600);
  bluetooth.begin(9600);
  analogReference(INTERNAL);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(12, INPUT);       // bluetooth  STATE
  int i = 0;
  while(i == 0 || i == 1){
    if(digitalRead(2) == HIGH && i == 1) i = 2;
    else i = 0;
    if(digitalRead(2) == HIGH && i == 0) i = 1;
    delay(1000);
  }
  bluetooth.println("Krabicka zavrena");
  sim800l.SIM800Init();
  bluetooth.println("GSM modul: OK");
  while(lat == 0||lon == 0){
    GPSmodul();
    delay(1000);
  }
  GPStimeLast = GPStime;
  for(int i = 0; i < 5; i++){
    GPSmodul();
    if(GPStimeLast == GPStime) i = 0;
    GPStimeLast = GPStime;
    delay(500);
  }
  bluetooth.println("GPS signal: OK");
  batteryVoltage(true);
  latS = lat;
  lonS = lon;
  attachInterrupt(0,boxOpen,FALLING);
  attachInterrupt(1,powerOut,FALLING);
  delay(100);
  bluetooth.println("Setup: OK");
}

void loop() {
  batteryVoltage(false);
  GPSmodul();
  GPSsignal = GPSsignal_OkOrNot();
  incomeCallOrSMS(); 
  while(stopLoop == true){
    detachInterrupt(0);
    detachInterrupt(1);
  }
  outgoingSMS();
  BlueTooth(); 
  interrupCorrection();
  GPStimeLast = GPStime;
}

void incomeCallOrSMS(){
  uint8_t infoStatus = sim800l.checkCallAndSMS();
  sim800l.callEnd();
  if (infoStatus == 1) {sim800l.callEnd();}
  else     
  if (infoStatus == 2){  
    String number = sim800l.getNumber();
    String textSMS = sim800l.getSMSContent();
    if (number == mainNumber || number == userNumber){ 
      if (textSMS == "Pozice?") getPosition(number, false);
      else if (textSMS == "Napeti baterii?") sim800l.sendSMS(number, SMSvoltageBat());
      else if (textSMS == "Napajeni?") sim800l.sendSMS(number, SMSextPower());
      else if (textSMS == "Krabicka?") sim800l.sendSMS(number, SMSboxCloseOpen());
      else if (textSMS == "GPS signal?") sim800l.sendSMS(number, SMSGPSsignal());
      else if (textSMS.substring(0,19) == String("Nastav vzdalenost: ")) setDistance(number, textSMS);
      else if (textSMS == userPassword) userNumber = number;
      else if (number == mainNumber){
              if (textSMS == "Uzivatelske cislo?") sim800l.sendSMS(mainNumber, userNumber);
              else if (textSMS == "Uzivatelske heslo?") sim800l.sendSMS(mainNumber, userPassword);
              else if (textSMS == "Udrzba") stopLoop = true;
              else if (textSMS.substring(0,26) == String("Nastav uzivatelske heslo: ")) setUserPassword(textSMS);
              else if (textSMS.substring(0,21) == String("Nastav hlavni cislo: ")) setNumber(textSMS, true);
              else if (textSMS.substring(0,26) == String("Nastav uzivatelske cislo: ")) setNumber(textSMS, false);
              else sim800l.sendSMS(mainNumber, "Chybny prikaz");
      }
      else sim800l.sendSMS(number, "Chybny prikaz");
    }else{
      if(textSMS == mainPassword) mainNumber = number;
      else if(textSMS == userPassword) userNumber = number;
      else{
        textSMS += " od: ";
        textSMS += number;
        sim800l.sendSMS(mainNumber,textSMS);
        textSMS = "";
      }
    } 
  }
}

void outgoingSMS(){
  String text;
  if(overPosition() == true && myDrive == false && sentOverPos == false){
    text = F("Prekrocena nastavena vzdalenost. ");
    text += SMSposition();
    sim800l.sendSMS(userNumber,text);
    //Serial.println(text);
    text = "";
    sentOverPos = true;
  }
  if(GPSsignal == false && myDrive == false && sentGPSsignalLost == false){
    text = F("Ztracen GPS signal. ");
    text += SMSposition();
    sim800l.sendSMS(userNumber,text);
    //Serial.println(text);
    sentGPSsignalLost = true;
    text = "";
  }
  if(((bat1 < 3 && bat1Last < 3) || (bat2 < 3 && bat2Last < 3)) && (sentBatWarning == false)){
      sim800l.sendSMS(userNumber,F("Napeti baterie kleslo pod 3 V. "));
      //Serial.println(F("Napeti baterie kleslo pod 3 V. "));
      sentBatWarning = true;
  }
}

void getPosition(String  number, bool BT){
  String text = F("Posledni znama pozice v case: ");   
  if(GPSsignal == true){
    if(BT == false) sim800l.sendSMS(number,SMSposition());
    if(BT == true) bluetooth.println(SMSposition());
    //Serial.println(SMSposition());
  }  
  if(GPSsignal == false){
    text += GPStime;
    text += " " ;
    text += SMSposition();  
    if(BT == false) sim800l.sendSMS(number,text);
    if(BT == true) bluetooth.println(text);
    //Serial.println(text);
    }
}

bool GPSsignal_OkOrNot(){
  if(GPSsignalStatus == 0 && GPStimeLast == GPStime){
    timeGPSsignalLost = millis();
    GPSsignalStatus = 1;
    return true;
  }
  if(GPSsignalStatus == 0 && GPStimeLast != GPStime){
    GPSsignalStatus = 0;
    return true;
  }
  if(GPSsignalStatus == 1 && (millis() - timeGPSsignalLost) > 15000){
    if(GPStimeLast == GPStime){
      GPSsignalStatus = 2;
      return false;
    }
    GPSsignalStatus = 0;
    return true;
  }
  if(GPSsignalStatus == 1 && GPStimeLast == GPStime){
    GPSsignalStatus = 1;
    return true;
  }
  if(GPSsignalStatus == 1 && GPStimeLast != GPStime){
    GPSsignalStatus = 0;
    return true;
  }
  
  if(GPSsignalStatus == 2 && GPStimeLast == GPStime){
    GPSsignalStatus = 2;
    return false;
  }
  if(GPSsignalStatus == 2 && GPStimeLast != GPStime){
    GPSsignalStatus = 0;
    return true;
  }
}

void GPSmodul(){
  bool newData = false;
  unsigned long characters;
  unsigned short words, mistakes;
  int year;
  byte month, day, hour, minute, second;
  for (unsigned long start = millis(); millis() - start < 1000;) {
    gpsSerial.listen();
    while (gpsSerial.available()) {
        char c = gpsSerial.read();
        if (gps.encode(c)) {
        newData = true;
      }
    }
  }
  if (newData) {   
    gps.f_get_position(&lat, &lon);
    gps.stats(&characters, &words, &mistakes);
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second);
    char temp[12];
    char time[12];
    hour += 1;
    if(hour == 24)hour = 00;
    if(hour<10){
      strcpy(time, itoa( 0, temp, 10));
      strcat(time, itoa( hour, temp, 10));
    }else
      strcpy(time, itoa( hour, temp, 10));
      strcat(time,":");
    if(minute<10){
      strcat(time, itoa( 0, temp, 10));
      strcat(time, itoa( minute, temp, 10));
    }else
      strcat(time, itoa( minute, temp, 10));
      strcat(time,":");
    if(second<10){
      strcat(time, itoa( 0, temp, 10));
      strcat(time, itoa( second, temp, 10));
    }else
      strcat(time, itoa( second, temp, 10));
    GPStime = time;
    
    if (lat == TinyGPS::GPS_INVALID_F_ANGLE) lat = 0.0;
    if (lon == TinyGPS::GPS_INVALID_F_ANGLE) lon = 0.0;
    if (characters == 0) {
      //Serial.println("Error");
    }
  }
}

bool overPosition(){
  float real_distance;
  real_distance = gps.distance_between (lat, lon, latS, lonS) / 1000;
  if(real_distance > set_distance){
    if(GPStimeLast == GPStime){
      numberOverPos = 0;
    }
    numberOverPos++;
    if(numberOverPos == 10){
      numberOverPos = 0;
      return true;
    }
    return false;
  }
  else{
    numberOverPos = 0;
    return false;
  }
}

void setDistance(String number, String textSMS){
  if(textSMS.substring(19) == "neomezeno"){
    myDrive = true;
  }
  if(textSMS.substring(19) != "neomezeno"){
     if(textSMS.substring(19,20).compareTo("/") < 11 && textSMS.substring(19,20).compareTo("/") > 0){
        if(GPSsignal == true){
          set_distance = textSMS.substring(19).toFloat();
          if(set_distance < null_distance) set_distance = null_distance;
          latS = lat;
          lonS = lon;
          myDrive = false;
          sentOverPos = false;
          sentGPSsignalLost = false;
        }
        if(GPSsignal == false){
          String text = F("Kvuli ztrate GPS signalu, je vzdalenost nastavena vzhledem k posledni zname pozici: "); 
          set_distance = textSMS.substring(19).toFloat();
          if(set_distance < null_distanceNoSignal) set_distance = null_distanceNoSignal;
          latS = lat;
          lonS = lon;
          myDrive = false;
          sentOverPos = false;
          text += SMSposition();
          text += " v čase: ";
          text += GPStime;
          text += ".";
          sim800l.sendSMS(number, text);
        }
     }else
        sim800l.sendSMS(number, "Chyba");
  }
}

void setNumber(String textSMS, bool main){
  bool number = false;
  byte index;
  if(main == true) index = 21;
  if(main == false) index = 26;
  if(textSMS.substring(index).length() == 12){
    for(byte i = 0; i < textSMS.substring(index).length(); i++){
      if(textSMS.substring(index+i,index+1+i).compareTo("/") < 11 && textSMS.substring(index+i,index+1+i).compareTo("/") > 0)
      number = true;
      else{
        i = textSMS.substring(index).length();
        number = false;
      }
    }
    if(number == true){
      if(main == true){
        mainNumber = textSMS.substring(index);
        //Serial.println(mainNumber);
      }
      if(main == false){
        userNumber = textSMS.substring(index);
        //Serial.println(userNumber);
      }
    }else
    sim800l.sendSMS(mainNumber, "Chyba");
    //Serial.println("Error");
  }else
  sim800l.sendSMS(mainNumber, "Chyba");
  //Serial.println("Error");
}

void setUserPassword(String textSMS){
  bool password = false;
  if(textSMS.length() == 31){
    for(byte i = 0; i < textSMS.substring(26).length(); i++){
      if((textSMS.substring(26+i,27+i).compareTo("/") < 11 && textSMS.substring(26+i,27+i).compareTo("/") > 0 )||
         (textSMS.substring(26+i,27+i).compareTo("@") < 27 && textSMS.substring(26+i,27+i).compareTo("@") > 0 )||
         (textSMS.substring(26+i,27+i).compareTo("`") < 27 && textSMS.substring(26+i,27+i).compareTo("`") > 0 ))
      password = true;
      else{
        i = textSMS.substring(26).length();
        password = false;
      }
    }
    if(password == true) userPassword = textSMS.substring(26);
    else sim800l.sendSMS(mainNumber, "Chyba"); 
         //Serial.println("Error");
  }else sim800l.sendSMS(mainNumber, "Chyba");
        //Serial.println("Error");
}

void BlueTooth(){
  int i = 0;
  String BTdata = "";
  char b; 
  if(digitalRead(12)==HIGH){
    unsigned long start = millis();
    bluetooth.listen();
    bluetooth.println("->");
    while ((millis() - start) < 4000) {
      while (bluetooth.available() > 0){
        b=bluetooth.read();
        BTdata += b;
        if(b == '\n'){
          incomeBlueTooth(BTdata);
          BTdata = "";
        }
      }
    }
    bluetooth.println(".");
  }
  if(myDrive == true && digitalRead(12)==LOW){
    setDistanceBT_0(false);
  }
}

void incomeBlueTooth(String BTdata){
  //Serial.println(BTdata);
  if (BTdata.substring(0,5) == "13751") getPosition("none", true);
  if (BTdata.substring(0,5) == "13752") bluetooth.println(SMSvoltageBat());
  if (BTdata.substring(0,5) == "13753") bluetooth.println(SMSextPower());
  if (BTdata.substring(0,5) == "13754") bluetooth.println(SMSboxCloseOpen());
  if (BTdata.substring(0,5) == "13755") bluetooth.println(SMSGPSsignal());
  if (BTdata.substring(0,5) == "13756") bluetooth.println("Uzivatel je: " + userNumber);
  if (BTdata.substring(0,5) == "13757") bluetooth.println(GSMmodulInformBT());
  if (BTdata.substring(0,5) == "13758") setDistanceBT();
  if (BTdata.substring(0,5) == "13759") setDistanceBT_0(true);
  if (BTdata.substring(0,5) == "13760") {
    bluetooth.println("Jizda");
    myDrive = true;
  }
  if (BTdata.substring(0,5) == "13761") setUserNumberBT(BTdata);
  BTdata = "";
}

void setDistanceBT(){
  String BTdata;
  byte i = 0;
  char b;
  unsigned long start = millis();
  bluetooth.listen();
  bluetooth.println("Napište vzdalenost:");
    while ((millis() - start) < 15000 && i == 0) {
      while (bluetooth.available() > 0){
        b=bluetooth.read();
        BTdata += b;
        if(b == '\n'){
          i = 1;
        }
      }
    }
  if(i == 0)bluetooth.println("Konec");
  if(BTdata.substring(0,1).compareTo("/") < 11 && BTdata.substring(0,1).compareTo("/") > 0){
    if(GPSsignal == true){
      set_distance = BTdata.substring(0).toFloat();
      if(set_distance < null_distance) set_distance = null_distance;
      latS = lat;
      lonS = lon;
      myDrive = false;
      sentOverPos = false;
      sentGPSsignalLost = false;
      BTdata = "";
      //Serial.println(set_distance,6);
    }
    if(GPSsignal == false){
      String text = F("Kvuli ztrate GPS signalu, je vzdalenost nastavena vzhledem k posledni zname pozici: ");
      set_distance = BTdata.toFloat();
      if(set_distance < null_distanceNoSignal) set_distance = null_distanceNoSignal;
      latS = lat;
      lonS = lon;
      myDrive = false;
      sentOverPos = false;
      text += String(SMSposition());
      text += " v čase: ";
      text += GPStime;
      text += ".";
      BTdata = "";
      //Serial.println(set_distance,6);
      bluetooth.println(text);
    }
    bluetooth.println("OK");
  }else
    bluetooth.println("Error");
}

void setDistanceBT_0(bool BT){
  if(GPSsignal == true){
    set_distance = null_distance;
    latS = lat;
    lonS = lon;
    myDrive = false;
    sentOverPos = false;
    sentGPSsignalLost = false;
    if(BT == true) bluetooth.println("Zabezpeceno");
  }
    if(GPSsignal == false){
      String text = F("Kvuli ztrate GPS signalu, je k zabezpeceni vyuzita posledni znama pozice: "); 
      set_distance = null_distanceNoSignal;
      latS = lat;
      lonS = lon;
      myDrive = false;
      sentOverPos = false;
      text += SMSposition();
      text += " v čase: ";
      text += GPStime;
      text += ".";
      if(BT == false) sim800l.sendSMS(userNumber, text);
      if(BT == true) bluetooth.println(text);
    }
}

void setUserNumberBT(String BTdata){
  bool number = false;
  if(BTdata.substring(5).length() == 14){
    for(byte i = 0; i < (BTdata.substring(5).length() - 2); i++){
      if(BTdata.substring(5+i,6+i).compareTo("/") < 11 && BTdata.substring(5+i,6+i).compareTo("/") > 0)
      number = true;
      else{
        i = BTdata.substring(5).length();
        number = false;
      }
    }
    if(number == true){
      userNumber = BTdata.substring(5,17);
      bluetooth.println("OK");
    }else
      bluetooth.println("Error");
  }else
    bluetooth.println("Error");
}

String GSMmodulInformBT(){
  String text = "GSM modul:";
  if(sim800l.checkModul() == true) text += " je OK\n";
  else text += " neni OK\n";
  text += "           ";
  if(sim800l.isConnected() == true) text += F("             je pripojen k siti");
  else text += F("             neni pripojen k siti");
  return text;
}

String SMSposition(){
  char buff[10];
  String googleMaps = F("http://maps.google.com/?q=");
  dtostrf(lat, 4, 6, buff);
  googleMaps += buff;
  googleMaps += ",";
  dtostrf(lon, 4, 6, buff);
  googleMaps += buff;
  return googleMaps;
}

String SMSvoltageBat(){
  char buff[10];
  String voltageText = "Bat.1: ";
  dtostrf(bat1, 4, 2, buff);
  voltageText += buff;
  voltageText += "V   Bat.2: ";
  dtostrf(bat2, 4, 2, buff);
  voltageText += buff;
  voltageText += "V";
  return voltageText;
}

String SMSextPower(){
  if(digitalRead(3)==HIGH)return "Pripojeno";
  if(digitalRead(3)==LOW)return "Odpojeno";
}

String SMSboxCloseOpen(){
  if(digitalRead(2)==HIGH)return "Zavrena";
  if(digitalRead(2)==LOW)return "Otevrena";
}

String SMSGPSsignal(){  
  if(GPSsignal == true){
    sentGPSsignalLost = true;
    return "OK"; 
  }
  if(GPSsignal == false)return "Neni";
}

void powerOut(){ 
  if(digitalRead(3) == LOW){
    if(powerInterrupt == false){
      powerInterrupt = true;
      powerInterrupt2 = true;
      timePowerOut = millis();
    } 
  }
}
void boxOpen(){
  if(digitalRead(2) == LOW){
    if(boxInterrupt == false){
      boxInterrupt = true;
      boxInterrupt2 = true;
      timeBoxOpen = millis();
    }
  } 
}

void interrupCorrection(){
  if(powerInterrupt2 == true && (millis()-timePowerOut) > 1000 && digitalRead(3) == LOW){
    sim800l.sendSMS(userNumber,F("Napajeni odpojeno!"));
    //Serial.println("Napajeni odpojeno!");
    powerInterrupt2 = false;
  }
  if(boxInterrupt2 == true && (millis()-timeBoxOpen) > 1000 && digitalRead(2) == LOW){
    sim800l.sendSMS(userNumber,F("Otevreni krabicky!"));
    //Serial.println("Otevreni krabicky!"); 
    boxInterrupt2 = false;
  }
  if(powerInterrupt == true && (millis()-timePowerOut) > 20000){
     powerInterrupt = false;          
  }
}

void batteryVoltage(bool Setup){
  if(Setup == true){
    timeBatteryRead = millis();
    float valueBat1 = analogRead(A4);
    float valueBothBat = analogRead(A3);
    bat1 = (valueBat1/1023*1.05)*40/10;
    float bothBat = (valueBothBat/1023*1.05)*64.2/8.2;
    bat2 = bothBat - bat1;
  }
  if(millis()-timeBatteryRead > 60000 && Setup == false){
    timeBatteryRead = millis();
    bat1Last = bat1;
    bat2Last = bat2;
    float valueBat1 = analogRead(A4);
    float valueBothBat = analogRead(A3);
    bat1 = (valueBat1/1023*1.05)*40/10;
    float bothBat = (valueBothBat/1023*1.05)*64.2/8.2;
    bat2 = bothBat - bat1;
  }
}

