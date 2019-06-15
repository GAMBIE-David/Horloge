// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       horloge.ino
    Created:	26/05/2019 11:14:17
    Author:     DESKTOP-MNVU0P6\David-Fixe
	Remarque : Changer la façon d'analyser les messages de la com web
			 : Gérer la luminosite de l'affichage en fonction de l'heure


*/

// Define User Types below here or use a .h file
//

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>



//**************************MATRICE-LED*********************************************
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW // FC16_HW pour l'avoir dans le bon sens
#define MAX_DEVICES 5
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D6
#define PAUSE_TIME    2000
#define SCROLL_SPEED  50
#define TEMPO_MSSG 60000

// Hardware SPI connection
//MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);



// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//



// Constante du projet :
const char* ssid = "SFR_92E8"; // remplacer par le SSID de votre WiFi
const char* password = "voabalnevexecbinwem5"; // remplacer par le mot de passe de votre WiFi
WiFiServer server(80); // on instancie un serveur ecoutant sur le port 80
#define BUF_SIZE 240 // taille du buffer pour le tableau de caractere
char chaine[BUF_SIZE];
bool maintient_message = false;

//variable global :
String message = "";
bool ete = false;

//Pour récupérer l'heure sur un serveur de temps
WiFiClient espClient;
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "ntp.midway.ovh", 3600, 60000);



// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(115200);

	// on demande la connexion au WiFi
	WiFi.begin(ssid, password);
	Serial.println("Demarrage");
	// on attend d'etre connecte au WiFi avant de continuer
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	// on affiche l'adresse IP qui nous a ete attribuee
	Serial.println("");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	// on definit ce qui doit etre fait lorsque la route /bonjour est appelee
	// ici on va juste repondre avec un "hello !"
	
	
	// on commence a ecouter les requetes venant de l'exterieur
	server.begin();

	//Debut du sereur ntp
	timeClient.begin();


	//Init de la matrice a led :
	P.begin();

}


// Add the main program code into the continuous loop() function
void loop()
{
	// a chaque iteration, on appelle handleClient pour que les requetes soient traitees
	//server.handleClient();
	timeClient.update();//update de l'heure
	//Serial.println(timeClient.getFormattedTime());



	//Récupération de l'heure dans des variables 
	int heure = 0;
	int minute = 0;
	int seconde = 0;
	
	String strHeure = Heure_str(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds(), ete);
	//String reception = Communication_web();
	
	Luminosite(timeClient.getHours());
	AffichageHeure(strHeure);

	switch (decode(Communication_web())) {
	case 1 :
		AffichageText("Maintient OK", 5000, false);
		//maintient_message = !maintient_message;
		break;

	case 2 :
		AffichageText("Ete OK", 5000, false);
		//ete = !ete;
		break;

	case 3 :
		AffichageText(message, TEMPO_MSSG, maintient_message);
		break;

	default:
		break;
	}
	//Serial.println(reception);

	


}

int decode(String reception) {
	char recept[2];
	char decode[4] = "GET";

	//Vérification de la réception :
	for (int i = 0; i < 3; i++) {
		recept[i] = reception[i];
		//Serial.print("i : ");
		//Serial.println(i);
		//Serial.print("recept[i] : ");
		//Serial.println(recept[i]);
		//Serial.print("reception[i] : ");
		//Serial.println(reception[i]);
		if (recept[i] != decode[i]) {
			return 0;
		}
		if (i == 2) {//Si on arrive à 3 c'est qu'on à la présence d'un GET
			Serial.println("###################OK################");
			Serial.print(reception);

			//Analyse du message reçu
			if (reception.indexOf("param") != -1) {
				//AffichageText("Parametrage", 1000);//Si c'est un message de parametrage
				int start = reception.indexOf(':');
				int stop = reception.lastIndexOf('*');
				String param = reception.substring(start + 1, stop);
				//AffichageText(param, 2000, false);
				if (param == "maintient") {
					/*
					AffichageText("Maintient OK", 5000, false);
					maintient_message = true;*/
					maintient_message = !maintient_message;
					return 1;
				}

				if (param == "ete") {

					//Serial.println("ete recu");
					ete = !ete;
					return 2;
				}
			}
			else {
				int start = reception.indexOf("/");
				int stop = reception.lastIndexOf('*');

				message = reception.substring(start + 1, stop);
				message.replace('_', ' ');
				Serial.print("Le message reçu est : ");
				Serial.println(message);
				return 3;
				//AffichageText(message, TEMPO_MSSG, maintient_message);
			}

		}
	}

}


int Luminosite(int heure) {
	//Fonction pour gérer la luminosité de l'affichage en fonction de l'heure

	if (heure > 8 && heure < 19)
	{
		P.setIntensity(7);
		return 0;
	}

	else
	{
		P.setIntensity(1);
		return 1;
	}

}

String Heure_str(int heure, int minute , int seconde , bool ete) {
	String strHeure = "";
	String strMinute = "";
	String strSeconde = "";


	if (minute < 10) {
		strMinute = "0" + String(minute);
	}

	else {
		strMinute = String(minute);
	}


	if (ete) {

		strHeure = String(heure + 1);
	}

	else {
		strHeure = String(heure);
	}

	if (seconde < 10) {

		strSeconde = "0" + String(seconde);
	}

	else {

		strSeconde = String(seconde);
	}




		return strHeure + ":" + strMinute + ":" + strSeconde;
}


void AffichageHeure(String heure) {
	heure.toCharArray(chaine, BUF_SIZE);//conversion en tableau de caractere
	P.displayText(chaine, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
	P.displayAnimate();

}

void AffichageText(String text, int tempo , bool maintient) {
	text.toCharArray(chaine, BUF_SIZE);//conversion en tableau de caractere
	//P.displayText(chaine, PA_CENTER, SCROLL_SPEED, PAUSE_TIME, PA_PRINT, PA_SCROLL_LEFT);
	//P.displayAnimate();
	unsigned long oldTime = millis();
	unsigned long currentTime = millis();


	if (maintient == true) {
		while (maintient == true)
		{
			if (P.displayAnimate()) {
				P.displayText(chaine, PA_LEFT, 50, 200, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
			}
			if (decode(Communication_web()) != 0)
				break;
		}
	}
	else
	{
		while (currentTime <= (oldTime + tempo)) {
			//Serial.print("current time : ");
			//Serial.println(currentTime);
			if (P.displayAnimate()) {
				P.displayText(chaine, PA_LEFT, 50, 200, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
			}
			currentTime = millis();
			yield();
		}
	}

		//P.displayScroll(chaine, PA_LEFT, PA_SCROLL_LEFT, SCROLL_SPEED);
		//P.displayAnimate();
}

String Communication_web() {

	WiFiClient client = server.available();
	if (!client) {
		return "";
	}

	// Wait until the client sends some data
	Serial.println("new client");
	while (!client.available()) {
		delay(1);
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	//String te = "";
	//te = client.readString();
	//Serial.print("Req : ");
	//Serial.println(req);
	//Serial.print("te : ");
	//Serial.println(te);
	client.flush();
	
	int val;
	if (req.indexOf("GET") != -1)
		val = 0;

	//if (req.indexOf("param:") != -1)
		//val = 1;

	else {
		Serial.println("invalid request");
		client.stop();
		return "";
	}

	String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nCommande recu";

	s += "</html>\n";
	client.print(s);
	//Serial.println(s);
	delay(1);
	Serial.println("Client disonnected");
	return req;

}

