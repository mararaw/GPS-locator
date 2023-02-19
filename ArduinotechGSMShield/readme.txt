Popis knihovny ArduinotechGSMShield
Tato knihovna je ur�en� pro tento shield:
http://www.arduinotech.cz/produkt/gsm-shield-arduinotech/, podrobnosti naleznete v �l�nku:
http://www.arduinotech.cz/inpage/jak-jednoduse-na-gsm-v-dil-arduinotech-gsm-shield/

T��da AGS
N�zev t��dy: AGS (zkratka ArduinotechGSMShield)
Vstupn� parametry: debug m�d -> 1=zapnut�, 0=vypnut�, debug m�d vypisuje podrobnosti na s�riov� kan�l u n�ter�ch funkc�.
V�stupn� parametry: ��dn�
P��klad: 
vytvo�en� instance t��dy ve sketchi:
AGS modul(1); //debug m�d povolen
AGS modul(0); //debug m�d zak�z�n

begin();
Popis: inicializuje shield, definuje softwareSerial na portech RX=2, TX=3, rychlost 9600 b/s, inicializuje s�riov� kan�l Arduina na rychlosti 9600b/s, spou�t� konfigura�n� proces SIM800Init()
Vstupn� parametry: ��dn�
V�stupn� parametry: ��dn�
P��klad: 
modul.begin();
	
SIM800Init();
Popis: Konfigura�n� procedura pro SIM800 modul, lze pou��t samostatn� nap�. pro inicializaci v p��pad� zji�t�n� nekorektn�ho chov�n� v ur�it�ch pas��ch k�du. Tato procedura je zahrnuta do funkce begin();
Vstupn� parametry: ��dn�
V�stupn� parametry: ��dn�
P��klad: 
modul.SIM800Init();

checkCallAndSMS();
Popis: Prov�d� otestov�n�, zda je p��tomen p��choz� hovor nebo nov� SMSka. Doporu�uji vlo�it do hlavn� smy�ky
Vstupn� parametry: ��dn�
V�stupn� parametry: 0 = ��dn� hovor nebo nov� SMS, 1 = nov� hovor, 2 = nov� SMS
P��klad: 
loop()
{
	infoStatus = modul.checkCallAndSMS();
if (infoStatus == 1)
{
	//akce p�i detekci vol�n�
}
if (infoStatus == 2)
{
	//akce p�i detekci SMS
}	
}


callEnd();
Popis: Vy�le p��kaz do SIM800 pro ukon�en� st�vaj�c�ho hovoru � zav�en�
Vstupn� parametry: ��dn�
V�stupn� parametry: ��dn�
P��klad:
modul.callEnd();

makeCall(String callNumber);
Popis: Provede vyto�en� ��sla � sestav� hlasov� hovor
Vstupn� parametry: String telefonn� ��slo v n�rodn�m nebo mezin�rodn�m tvaru bez +
V�stupn� parametry: ��dn�
P��klad:
modul.makeCall(�739822476�);

getNumber();
Popis: Vr�t� �et�zec s telefonn�m ��slem volj�c�ho nebo ��astn�ka, kter� poslal SMS. Obvykle je tento dotaz realizov�n bezprost�edn� po vyhodnocen� stavu checkCallAndSMS() funkce.
Vstupn� parametry: ��dn�
V�stupn� parametry: String telefonn� ��slo
P��klad:
infoStatus = modul.checkCallAndSMS();
if (infoStatus == 1) Serial.println(�Volajici ucastnik:� + modul.getNumber());
if (infoStatus == 2) Serial.println(�SMS od:� + modul.getNumber());


getSMSContent();
Popis: Zji�t�n� obsahu p�ijat� SMS
Vstupn� parametry: ��dn�
V�stupn� parametry: String obsah SMS
P��klad:
if (infoStatus == 2) 
{
Serial.println(�SMS od:� + modul.getNumber());
Serial.println(�Obsah SMS:� + modul.getSMSContent());
}


sendSMS(String number,String sms);
Popis: funkce pro odesl�n� SMS na dan� ��slo s dan�m obsahem
Vstupn� parametry: String ��slo p��jemce a String obsah SMS
V�stupn� parametry: ��dn�
P��klad:
modul.sendSMS(�73982246�,�Ahoj!�);


getProviderName();
Popis: Vrac� �et�zec se jm�nem oper�tora GSM, kter� je prezentov�no v r�mci GSM s�t�
Vstupn� parametry: ��dn�
V�stupn� parametry: String jm�no oper�tora
P��klad:
Serial.println(�GSM operator:� + getProviderName());

getQuality();
Popis: Vrac� �et�zec s kvalitou GSM sign�lu dle moment�ln�ho m��en�. Kvalita sign�lu je parametr vypo�ten� ze s�ly, latence a jin�ch parametr�, nen� to jen s�la sign�lu! Tento parametr se pohybuje od 1 do 30, 0 = nen� sign�l, 31 = p�ebuzen� sign�l
Vstupn� parametry: ��dn�
V�stupn� parametry: String kvalita sign�lu
P��klad:
Serial.println(�GSM signal quality:� + getQuality());

timeStamp();
Popis: Vrac� �et�zec s �asovou zna�kou obdr�enou ze s�t� GSM, tedy s pom�rn� p�esn�m �asem
Vstupn� parametry: ��dn�
V�stupn� parametry: String �asov� zna�ka
P��klad:
Serial.println(�GSM time:� + timeStamp());

GPRSInit(String APN)
Popis: Inicializuje GPRS dle zadan�ho APN
Vstupn� parametry: String APN
V�stupn� parametry: bool, 1 = �sp�n� attach GPRS
P��klad:
modul.GPRSInit("internet.t-mobile.cz");

sendDataGPRS(String dataToSend)
Popis: ode�le data pomoc� GPRS metodou HTTP GET
Vstupn� parametry: String data k odesl�n� - nej�ast�j� link na HTTP
V�stupn� parametry: String s vr�cen�mi daty po HTTP GET, pokud se komunikace nezda�ila, bude n�vratov� hodnota COMMUNICATION FAILURE
P��klad:
modul.sendDataGPRS("api.thingspeak.com/update?api_key=54DLXE3I1PETR61C&field1=25");


