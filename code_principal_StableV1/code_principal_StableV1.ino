/**
 * @file   code_principal_StableV1.ino
 * @author Emmanuel Lambert & Samuel Delage
 * @date   11 sept 2015
 * @brief  programme qui lit un module HX711 pour une lecture de poids et qui la transmet sur un SIM900 par commande AT
 *         pour le cours de préparation de projet, nom de projet: Surveillance de ruche à distance.
 *
 * @version 1.0 : Première version
 * Environnement: Arduino 1.6.5
 *
 * Matériel:
 *   Système cible: Arduino Nano
 *   Système de développement: PC avec Windows 10
 */
#include "HX711.h"
#include <SoftwareSerial.h>

#define SCALE1DOUT 2// pin DOUT du HX711  - pin #D2
#define SCALE1SCK 3 // pin PD_SCK du HX711 - pin #D3
#define BUTT1 5     // Bouton pour envoyer le poids - pin #D5
#define RX 9        // pin TX du SIM900 pour la communication serial
#define TX 10       // pin RX du SIM900
#define POWER 13    // pin pour allumer ou éteindre le SIM900


HX711 scale(SCALE1DOUT, SCALE1SCK);    // instensation de la librairie HX711
SoftwareSerial SIM900(RX, TX);  //pour la communication avec le SIM900
void sendSMS(float poids);
void calibre();


void setup() {
  SIM900.begin(9600);
  Serial.begin(9600); 
  Serial.println(" Projet SRD :");
  pinMode(BUTT1,INPUT);
  digitalWrite(BUTT1, LOW);

  scale.set_scale(22800.f);   // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
}

void loop() {
  Serial.print("lecture de la cellule de force:\t");
  Serial.print(scale.get_units(), 2);
  Serial.print("kg \t| moyenne de 5 mesures:\t");
  Serial.print(scale.get_units(5), 2);
  Serial.println("kg");   

  for (int i = 0;i<1000;i++)
  {
    if (digitalRead(BUTT1) == 1)
    {
      Serial.println("bouton appuye, mesure du poids...");
      float poids = scale.get_units(50);
      Serial.println("envoie du texto");
      sendSMS(poids);
      Serial.println("texto envoye");
    }
    delay(1);
  }
}

/**
 * @brief Fonction équivalent à appuyer sur le bouton 'Power' du Module GSM
 * @param aucun
 * @return aucun
 */
void SIM900power()
{
  digitalWrite(POWER, HIGH);
  delay(1000);
  digitalWrite(POWER, LOW);
  delay(5000);
  Serial.println("Power Done\r");
}

/**
 * @brief Fonction qui envoie des commandes AT au SIM900 pour envoyer le poids recu en paramètre par texto
 * @param poids (float)
 * @return aucun
 */
void sendSMS(float poids)
{
  SIM900power();                                // Alimentation du Module GSM
  delay(5000);                                  // module boot
  SIM900.print("AT\r");                         // pour booter le module
  delay(15000);                                 // laisse le temps au module de se connecter à un réseau
  SIM900.print("AT+CMGF=1\r");                  // Préparer l'envois du message
  delay(1000);
  SIM900.println("AT + CMGS = \"+18195789557\"");//Format international du numéro de téléphone
  delay(1000);
  SIM900.print("La Ruche pese ");               // message à envoyer  
  SIM900.print(poids, 2);        
  SIM900.println("Kg");
  delay(1000);
  SIM900.println((char)26);                     // CTRL + Z, Code ASCII 26, Besoin pour l'envois du Texto
  delay(15000);                                 // Donne Du temps au GSM d'envoyer le Texto
  SIM900power();                                // fermer module
}

/**
 * @brief Fonction bloquante qui affiche sur le serial les étapes à suivre pour calibre la balance
 * @param aucun
 * @return aucun
 */
void calibre()
{    
  int butt, buttFront = 0, etatCalib = 0;   //variable pour la détection de front et pour les étapes de calibration
  while (etatCalib < 3)
  {
    butt = digitalRead(BUTT1);              //lecture du bouton
    if (butt == 1 && buttFront == 0)        //pour la detection de front montant
    {
      if (etatCalib == 0)
      {
        Serial.println("Sequence de calibration: veuillez ne rien mettre sur la balance puis appuyez sur le bouton");
        delay(500);
      }
      else if(etatCalib == 1)
      {  
        Serial.println("reset en cours");
        scale.set_scale();                  //réinitialise l'échelle à 1
        scale.tare(50);                     //remise à zéro d'une moyenne de 50 mesure
        Serial.println("balance reinitialisee.\nVeuillez mettre une masse de 10 kg puis appuyez sur le bouton");
      }
      else if(etatCalib == 2)
      {
        Serial.println("Calibration...");
        scale.set_scale(scale.get_units(100)/10); //remet l'échelle au poids demandé
        Serial.println("Calibration terminee, merci bonne journee");
      }
      etatCalib++;
    }
    buttFront = butt;      
  }
  etatCalib = 0;
}
