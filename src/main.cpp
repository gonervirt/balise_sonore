/*
  Decode868_Prod_ESP32_AP_v2025
    le 15 JANVIER 2025
    ESP32 Dev Module
*********
VALEUR A MODIFIER PAR L'INTERFACE WEB
 Volume 
 dureeInhnib 
***********

*/

#include "Arduino.h"

#include <WiFi.h>
#include <NetworkClient.h> 
#include <WiFiAP.h>

//**********CONFIGURATION INTERFACE WIFI**************************************
const char *ssid = "BALISESONORE";
const char *password = "BaliseSonore_Betton_Mairie";
IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);
NetworkServer server(80);

//*************CONFIGURATION DES BROCHE RX ET TX POUR LECTURE MP3
#define RXD2 16  // RX2 pour communiquer avec MP3
#define TXD2 17  // TX2 pour communiquer avec MP3

//*****************VARIABLES************************
const byte interruptPin = 15;      // Broche entrée, sortie du recepteur
const byte ledMessageEnCours = 2;  // Led Message en cours				LED VERTE
const byte ledLanceMessage = 13;   // Led Demande de lecture du message  LED JAUNE

#define VOLUM 32     // pin Réglage du volume
#define DUREESON 35  // pin Réglage du temps

bool lanceTexte = false;
bool diffEnCour = false;
bool inhib = false;



int dureeLedLanceMessage = 200;  // durée allumage de la ledLanceMessage
unsigned long tempsLedLanceMessage = 0;

//*************************REBOUT SI PB******************************
double timeoutCounter = 0;

//****************TABLEAUX POUR DECODE TRAME *************************
byte MyBits[100];  // tableau des valeurs de code
int MyInts[100];   // tableau des durées


//************VARIABLES POUR DECODE TRAME
unsigned long previousMicros = 0;
unsigned long memoMicros = 0;
int largeur = 0;
int syncro1 = 610;  // durée de l'impulsion de synchro
int data0 = 350;    // durée de l'impulsion 0
int data1 = 570;    // durée de l'impulsion 1
int valdata = 0;
int compteur = 0;
String chaine1 = "1100001100000000001001001000000000000000";  //Motif à trouver (40)
//String chaine1 = "000000";                                                  //Motif à trouver (40)
String chaine2 = "004000011000011000000000010010010000000000000000000000";  // valeurs a éffacer, utilisées pour comprendre
int poschaine = 0;


//*************************CONFIGURATION ET VARIABLES LECTEUR MP3****************************
#include "DFRobotDFPlayerMini.h"  // lib V.1.0.6 par DFRobot
DFRobotDFPlayerMini myMP3player;  // instanciation du player
void printDetail(uint8_t type, int value);
int volume = 15;                // volume sonore // VALEUR A MODIFIER PAR L'INTERFACE WEB
int volmax = 30;                // volume maxi possible
int volmin = 0;                 // volume minimum lors du réglage du volume son
double dureeInhnib = 2000;      // durée pendant la quelle il est impossible de recevoir une nouvelle info du recepteur // VALEUR A MODIFIER PAR L'INTERFACE WEB
double dureeInhibMini = 1000;   //( 1s)
double dureeInhibMaxi = 60000;  //( 60) 60000
double tempsInhib = 0;          // mesure du temps inhib


//*******************FLAGS ET NUMEROE LECTURE MESSAGE**************************
byte numeroMessage = 1;
bool flagTemps = false;
bool interruptionActive = true;

void IRAM_ATTR Mesure() {
  if (interruptionActive) {
    compteur++;                             // impulsion suivante
    previousMicros = micros();              // on stocke le moment du changement en Microseconde
    largeur = previousMicros - memoMicros;  // on calcul la durée de l'impulsion en retirant le moment mémorisé précédemment
    memoMicros = previousMicros;            // on mémorise le moment pour l'impulsion suivante
    MyInts[compteur] = largeur;             // on enregistre la durée
  }
}



// ---------------------------------------------
void ajustvol() {
  volume = constrain(volume, volmin, volmax);
  myMP3player.volume(volume);  // réglage du volume MP3
  //***************DEBUG INFO PARAM A RETIRER POUR PROD************************
  Serial.print("volume : ");
  Serial.println(volume);  // partie à retirer, utilisée pour comprendre
  //************************************************************************************
}

void ajustdureeInhib() {
  dureeInhnib = constrain(dureeInhnib, dureeInhibMini, dureeInhibMaxi);
  //***************DEBUG INFO PARAM A RETIRER POUR PROD************************
  Serial.print("dureeInhnib : ");
  Serial.println(dureeInhnib);
  //************************************************************************************
}




void setup() {
  //*****************************ACTIVATION DE LA LED "LED MESSAGE EN COUR" **********************
  pinMode(ledMessageEnCours, OUTPUT);
  digitalWrite(ledMessageEnCours, HIGH);


  //*****************************PORT SERIE POUR DEBUG**********************
  Serial.begin(115200);


  //*****************************PORT SERIE CONTROLE LECTEUR MP3**********************
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);


  //*****************************INFO SUR L'AVANCEMENT DU SETUP**********************
  Serial.println(F("Début du programme"));


  //*****************************ACTIVATION DE LA BROCHE DE LEDLANCEMESSAGE**********************
  pinMode(ledLanceMessage, OUTPUT);  // Visualisation de la commande du lecteur MP3


  //*****************************INFO ET TEST POUR COMPRENDRE, A SUPPRIMER DANS LA VERSION FINALE**********************
  poschaine = chaine2.indexOf(chaine1);  // partie à retirer, utilisée pour comprendre

  Serial.print(F("Chaine trouvée à :"));  // partie à retirer, utilisée pour comprendre
  Serial.println(poschaine);              // partie à retirer, utilisée pour comprendre
  chaine2 = "";


  //*****************************LECTEUR MP3 OU WAV**********************
  if (!myMP3player.begin(Serial2, /*isACK = */ true, /*doReset = */ true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }
  myMP3player.enableDAC();
  Serial.println(F("DFPlayer Mini online."));
  myMP3player.volume(30);  //Set volume value. From 0 to 30


  //*****************************WIFI**********************
  Serial.println();
  Serial.println("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("Server started");


  //***********APRES 1,5 SECONDE, ON ETEINT LA LED "LED MESSAGE EN COURS" QUE L'ON AVAIT ALLUMEE AU DEBUT DU SETUP********************
  delay(1500);
  digitalWrite(ledMessageEnCours, LOW);


  //******ACTIVATION DE LA BROCHE INTERRUPTION APRES 1,5 SECONDE POUR EVITER UNE ERREUR DANS L'ESP SI ACTIVE LA BROCHE TROP TOT*****
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(interruptPin, Mesure, CHANGE);  // on lancera "Mesure" à chaque changement d'état. (0 = broche "2" sur nano ou uno)


  //*****************************FIN DE DU SETUP**********************
}


void loop() {
  // on bloque la suite de la boucle dès le début de la diffusion du message
  // et ce tantque le son est diffusé et que le temps inhib n'est pas terminé.
  // puis on redonne la  main a l'interruption

  //******************PARTIE A MODIFIER POUR ADAPTER LE CONTROLE WIFI****************
  NetworkClient client = server.accept();  // listen for incoming clients
  if (client) {                            // if you get a client,
    //Serial.println("New Client.");         // print a message out the serial port

    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected()) {  // loop while the client's connected
      if (client.available()) {   // if there's bytes to read from the client,
        char c = client.read();   // read a byte, then
        //Serial.write(c);                   // print it out the serial monitor
        if (c == '\n') {  // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/1\">here</a> ALLUME LA LED ET PREPARE LE PREMIER MESSAGE ENTREE MAIRIE ET POSTE.<br>");
            client.print("Click <a href=\"/2\">here</a> ALLUME LA LED ET PREPARE LE SECOND MESSAGE POSTE FERMEE.<br>");
            client.print("Click <a href=\"/3\">here</a> ALLUME LA LED ET PREPARE LE TROISIEME MESSAGE MAIRIE FERMEE.<br>");
            client.print("Click <a href=\"/0\">here</a> ETEINT LA LED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Analyse la réponse du client : "GET /0" or "GET /1" or "GET /1" or "GET /3":
        if (currentLine.endsWith("GET /0")) {
          digitalWrite(ledMessageEnCours, LOW);
          Serial.println("Extinction de la led");
        }
        if (currentLine.endsWith("GET /1")) {
          digitalWrite(ledMessageEnCours, HIGH);
          numeroMessage = 1;
          Serial.println("Messsage 1, allumage LED");
        }
        if (currentLine.endsWith("GET /2")) {
          digitalWrite(ledMessageEnCours, HIGH);
          numeroMessage = 2;
          Serial.println("Messsage 2, allumage LED");
        }
        if (currentLine.endsWith("GET /3")) {
          digitalWrite(ledMessageEnCours, HIGH);
          numeroMessage = 3;
          Serial.println("Messsage 3, allumage LED");
        }
      }
      interruptionActive = false;
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");

  } else {

    if (diffEnCour) {  // la demande de diffusion est faite

      if (myMP3player.available()) {
        if (myMP3player.readType() == DFPlayerPlayFinished) {
          if (!inhib) {
            inhib = true;
            tempsInhib = millis();
          }

          if (((millis() - tempsInhib) > dureeInhnib) && inhib) {
            Serial.println(F(" Inhib est terminé"));
            digitalWrite(ledMessageEnCours, LOW);  // on éteint la led
            diffEnCour = false;
            //***************DEBUG INFO LECTURE MP3 A RETIRER POUR PROD************************
            Serial.print(F("Number:"));
            Serial.print(myMP3player.read());
            Serial.println(F(" Play Finished!"));
            //*****************************************************************************************
          }
        }
        // POUR LA LED
        if ((millis() - tempsLedLanceMessage) > dureeLedLanceMessage) {
          digitalWrite(ledLanceMessage, LOW);  // on eteint la led apres 200 milisecondes
        }
      }
    } else {

      //**********************LE MESSAGE A ETE DIFFUSE, ON ECOUTE LE RECEPTEUR AM*******************
      timeoutCounter = 0;

      if (!interruptionActive) {
        attachInterrupt(interruptPin, Mesure, CHANGE);  // on relance l'interruption si pas lancée
        interruptionActive = true;
      }

      //*********LES TRAMES ARRIVENT EN PERMANANCE, ON ATTTEND D'AVOIR 60 IMPULTIONS**********************
      if (compteur > 60) {  // avec 60, on a suffisamment de données


        //**********ON ARRETE LA PRISE EN COMPTE DES TRAMES ARRIVANT DU RECEPTEUR AM
        interruptionActive = false;
        compteur = 0;

        //*******************POUR DEBUG A RETIRER*******************
        // Serial.println("DImp;");
        // for (int i = 0; i < 60; i++) {
        //   Serial.print(MyInts[i]);
        //   Serial.print(";");
        // }
        //**********************************************************


        // on parcourt le tableau de valeurs de durées
        for (int i = 0; i < 60; i++) {
          largeur = MyInts[i];
          // on détermine l'etat du bit reçu
          if (largeur < data0) {  //= 350
            valdata = 0;
          } else {
            if (largeur < data1) {  // = 570
              valdata = 1;
            } else {
              if (largeur < syncro1) {  // = 610
                valdata = 4;
              }
            }
          }
          MyBits[i] = valdata;          //on enregistre la valeur du bit reçu pour une visualisation si besoin sur le port série
          chaine2 = chaine2 + valdata;  // on ajoute la valeur du bit à la chaine dans la quelle on cherchera le motif souhaité.
        }


        //*******************POUR DEBUG A RETIRER*******************
        // Serial.println("");
        // for (int i = 0; i < 60; i++) {
        //   Serial.print(MyBits[i]);
        //   Serial.print(";");
        // }
        // Serial.println("");
        //***********************************************************


        poschaine = chaine2.indexOf(chaine1);  // on recherche le motif (chaine1) dans la chaine (chaine2) de 60 valeurs.


        //**************************************LA CHAINE A ETE TROUVE, ON LANCE LE MESSAGE**************************
        if (poschaine > -1) {  // on a trouvé le motif


          //**********ON ARRETE LA PRISE EN COMPTE DES TRAMES ARRIVANT DU RECEPTEUR AM
          compteur = 0;
          detachInterrupt(interruptPin);  // on arrête le controle de l'interruption car on a assez de données
          interruptionActive = false;


          //*******************POUR DEBUG A RETIRER*******************
          Serial.println(F("j'ai trouvé"));
          //**********************************************************


          //*******ON AJUSTE LE VOLUME ET LA DUREE D'INHIBITION DU MESSAGE AUTOMATIQUE OU NON***********************
          ajustvol();
          ajustdureeInhib();


          //*********ON LANCE LE MESSSAGE**************************
          myMP3player.play(numeroMessage);  //on lance le message, par defaut le "2", sinon changé par le web
          if (myMP3player.available()) {
            myMP3player.readType();
          }


          //**********ALLUMAGE DES LEDS DE CONTROLE*****************************************
          digitalWrite(ledLanceMessage, HIGH);    // on allue la led de lancement du message
          tempsLedLanceMessage = millis();        // on mémorise le moment de l'allumage
          digitalWrite(ledMessageEnCours, HIGH);  // on allume la led ledMessageEnCours

          //****************MODIFICATION DE ETAT DES VARIABLES**************************
          diffEnCour = true;
          inhib = false;
        }

        chaine2 = "";
      }
    }
  }
}
