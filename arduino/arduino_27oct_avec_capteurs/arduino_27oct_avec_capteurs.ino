
// GESTION DES SÉQUENCES DE COMMANDES PAR L'ARDUINO

// les commandes ont toujours un paramètre de type entier

// [[mda 255]] : moteur droit vers l'avant vitesse 255
// [[mdr 255]] : moteur droit vers l'arrière vitesse 255
// [[mga 255]] : moteur gauche vers l'avant vitesse 255
// [[mgr 255]] : moteur gauche vers l'arrière vitesse 255
// [[t 1000]] : timeout, arrêt des moteurs au bout de 1000 ms
// [[n 0]] : nop, no operation, permet d'envoyer l'état de l'arduino

// exemple : faire avancer le robot pendant 1000 ms à la vitesse 255
// [[t 1000][mda 255][mga 255]]

// exemple : faire reculer le robot pendant 1000 ms à la vitesse 255
// [[t 1000][mdr 255][mgr 255]]

// codes des commandes
#define ERREUR 0	// ne pas modifier la valeur 0
#define MOTEUR_G_AVANT 1
#define MOTEUR_G_ARRIERE 2
#define MOTEUR_D_AVANT 3
#define MOTEUR_D_ARRIERE 4
#define MOTEUR_STOP 5
#define DIST 6
#define TIMEOUT 7
#define NOP 8

#define RESET 1000

#define BUF_IN 200	// taille max du buffer de lecture
#define LCMD 50		// taille max du tableau de la liste de commandes

#define SENSOR_COUNT 4	// 4 capteurs de distance
#define SENSOR_TIMEOUT 100	// une lecture toutes les 100 ms
#define DIST_AVANT 0	// position dans le tableau des distances
#define DIST_GAUCHE 2	// position dans le tableau des distances
#define DIST_ARRIERE 4	// position dans le tableau des distances
#define DIST_DROIT 6	// position dans le tableau des distances

char buf_in[BUF_IN];	// buffer de lecture
int buf_in_count = 0;	// taille courante du buffer

int lcmd[LCMD]; 	// tableau de la liste des commandes à exécuter
// toujours 2 entiers par commande, le 1er pour la commande, le 2e pour le paramètre)
int lcmd_count = 0;	// taille courante du tableau de la liste des commandes
int lcmd_pos = 0;	// position de la commande courante en cours d'exécution

unsigned long time_sensor = 0;		// si 0 : timeout d'arrêt des moteurs désactivé

/**************************************************************************************/
/******************************** Etat du robot ***************************************/
/**************************************************************************************/

int dist[SENSOR_COUNT+SENSOR_COUNT]; // tableau des distances, 8 valeurs : 4 x (distance en mm + fiabilité mesure)

int vitesse_gauche = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255), positif si vers l'avant, négatif si vers l'arrière
int vitesse_droit = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255)

unsigned long time = 0;		// si 0 : timeout d'arrêt des moteurs désactivé
unsigned long timeout = 0;	// durée de fonctionnement des moteurs

/**************************************************************************************/
/************************************ setup *******************************************/
/**************************************************************************************/

void setup() {
	Serial.begin(9600);
	//Serial.begin(115200);

	delay(2000);

	moteur_init();
	vl53l1x_init();

	init_buf_in();
	init_lcmd();
	init_etat();

	time = 0;	// timeout d'arrêt des moteurs désactivé
	time_sensor = millis();	// timer des mesures de distance

	delay(2000);

	Serial.println("Arduino OK");


}

/**************************************************************************************/
/************************************* loop *******************************************/
/**************************************************************************************/


int c = -1;		// caractère courant lu
int c_prec = -1;	// caractère précédent lu

void loop() {

	if (time_sensor+ SENSOR_TIMEOUT < millis()) { // lecture des capteurs de distance
		vl53l1x_read();
		time_sensor = millis();
	}

	if (Serial.available()) { // lecture des commandes en provenance du PC
		c_prec = c;
		c = Serial.read();
		if ((c == '[') && (c_prec == '[')) {
			// nouvelle lecture d'une séquence de commandes
			init_buf_in();

			buf_in[0] = '[';
			buf_in_count = 1;
		}
		else if ((c == ']') && (c_prec == ']')) {
			// fin lecture, nouvelle séquence de commandes reçue
			// Serial.print("lu = "); Serial.println(buf_in);
			decoder_seq_commandes();
		}
		else if (c == '\n') {
		}
		else {
			buf_in[buf_in_count++] = c;
		}
	}

	if (lcmd_count != 0) { // exécution des séquences de commandes
		exec_cmd();
	}

	if (time != 0) { // timeout d'arrêt des moteurs
		if (time + timeout < millis()) {
			moteur_stop();
		}
	}

}


/*******************************************************************************/
/********************* décodage de la séquence de commandes ********************/
/*******************************************************************************/

void decoder_seq_commandes() {
	// analyse de la séquence de commandes reçue dans buf_in
	// résultat dans lcmd (2 entiers par commande, numéro commande et paramètre)
	
	init_lcmd();

	int pos = 0;
	int p1;
	for (;;) { // décodage d'une commande à chaque itération
		if (pos >= buf_in_count) break;

		if (buf_in[pos] == '[') {
			pos++;
			if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'g') && (buf_in[pos+2] == 'a')) {
				lcmd[lcmd_count++] = MOTEUR_G_AVANT;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'g') && (buf_in[pos+2] == 'r')) {
				lcmd[lcmd_count++] = MOTEUR_G_ARRIERE;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == 'a')) {
				lcmd[lcmd_count++] = MOTEUR_D_AVANT;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == 'r')) {
				lcmd[lcmd_count++] = MOTEUR_D_ARRIERE;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 's') && (buf_in[pos+2] == 't')) {
				lcmd[lcmd_count++] = MOTEUR_STOP;
				pos += 3;
			}
			else if (buf_in[pos] == 't') {
				lcmd[lcmd_count++] = TIMEOUT;
				pos++;
			}
			else if (buf_in[pos] == 'r') {
				lcmd[lcmd_count++] = RESET;
				pos ++;
			}
			else if (buf_in[pos] == 'n') {
				lcmd[lcmd_count++] = NOP;
				pos ++;
			}
			else {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}

			if (buf_in[pos] != ' ') {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}
			pos++;
			if ((buf_in[pos] < '0') || (buf_in[pos] > '9')) {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}
			p1 = pos; // début du paramètre (entier)
			for (;;) {
				pos++;
				if ((buf_in[pos] < '0') || (buf_in[pos] > '9')) {
					break;
				}
			}
			if (buf_in[pos] == ']') {
				buf_in[pos] = 0;
				lcmd[lcmd_count++] = atoi(buf_in+p1);
				pos++;
			}
			else {
				lcmd[lcmd_count-1] = ERREUR;
				break;
			}
			


		}
		else {
			lcmd[lcmd_count++] = ERREUR;
			break;
		}
	}
}

/*******************************************************************/
/********************* moteurs - shield DRI0009 ********************/
/*******************************************************************/

//Arduino PWM Speed Control：
int E1 = 5;  
int M1 = 4; 
int E2 = 6;                      
int M2 = 7;                        

//Arduino PWM Speed Control：
//int E1 = 10;
//int M1 = 12;
//int E2 = 11;
//int M2 = 13;

void moteur_init() {
	pinMode(M1, OUTPUT);   
	pinMode(M2, OUTPUT); 
	moteur_stop();
}

void moteur_gauche_avant(int speed) {
  	vitesse_gauche = speed;
	digitalWrite(M1,LOW);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_gauche_arriere(int speed) {
	vitesse_gauche = -speed;
	digitalWrite(M1,HIGH);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_droit_avant(int speed) {
	vitesse_droit = speed;
 	digitalWrite(M2,LOW);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteur_droit_arriere(int speed) {
	vitesse_droit = -speed;
	digitalWrite(M2,HIGH);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteur_stop() {
	analogWrite(E1, 0);   //PWM Speed Control
	analogWrite(E2, 0);   //PWM Speed Control
	vitesse_gauche = 0;
	vitesse_droit = 0;
}




/*******************************************************************/
/******************* VL53L1X Capteurs de distance ******************/
/*******************************************************************/


#include <Wire.h>
#include <VL53L1X.h>

// The number of sensors in your system.
const uint8_t sensorCount = 4;

// The Arduino pin connected to the XSHUT pin of each sensor.
const uint8_t xshutPins[sensorCount] = { 2,3, 8,9 };

VL53L1X sensors[sensorCount];


void vl53l1x_init() {
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  // Disable/reset all sensors by driving their XSHUT pins low.
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }

  // Enable, initialize, and start each sensor, one by one.
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    // Stop driving this sensor's XSHUT low. This should allow the carrier
    // board to pull it high. (We do NOT want to drive XSHUT high since it is
    // not level shifted.) Then wait a bit for the sensor to start up.
    pinMode(xshutPins[i], INPUT);
    delay(10);

    sensors[i].setTimeout(500);
    if (!sensors[i].init())
    {
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      while (1);
    }

    // Each sensor must have its address changed to a unique value other than
    // the default of 0x29 (except for the last one, which could be left at
    // the default). To make it simple, we'll just count up from 0x2A.
    sensors[i].setAddress(0x2A + i);

    sensors[i].startContinuous(50);
    sensors[i].setDistanceMode(VL53L1X::Long);
    //sensors[i].setDistanceMode(VL53L1X::Medium);
    //sensors[i].setDistanceMode(VL53L1X::Short);

  }

}

void vl53l1x_read() {
	int pos = 0;
	for (uint8_t i = 0; i < sensorCount; i++) {
		dist[pos++] = sensors[i].read();	// distance en mm
		dist[pos++] = (int)sensors[i].ranging_data.peak_signal_count_rate_MCPS; // fiabilité de la mesure
 	}
}


/*******************************************************************/
/********************* exécution d'une commande ********************/
/*******************************************************************/

void exec_cmd() {


	if (lcmd_pos >= lcmd_count) {
		init_lcmd();
		arduino_etat();	// on envoie l'état courant à la fin de chaque séquence de commande
		return;
	}

	int cmd = lcmd[lcmd_pos++];
	int param = lcmd[lcmd_pos++];

	// Serial.print("cmd = "); Serial.print(cmd); Serial.print(" param = "); Serial.println(param);

	if (cmd == MOTEUR_G_AVANT) moteur_gauche_avant(param);
	else if (cmd == MOTEUR_D_AVANT) moteur_droit_avant(param);
	else if (cmd == MOTEUR_G_ARRIERE) moteur_gauche_arriere(param);
	else if (cmd == MOTEUR_D_ARRIERE) moteur_droit_arriere(param);
	else if (cmd == MOTEUR_STOP) moteur_stop();
	else if (cmd == DIST) {
	}
	else if (cmd == NOP) {
	}
	else if (cmd == TIMEOUT) {
		time = millis();
		timeout = param;
	}
	else if (cmd == RESET) {
		reset_arduino();
	}


}

/***************************************************************************************/
/*************************************** init ******************************************/
/***************************************************************************************/

void init_buf_in() {
	for (int i=0 ; i<BUF_IN ; i++) {
		buf_in[i] = 0;
	}
	buf_in_count = 0;
}

void init_lcmd() {
	for (int i=0 ; i<LCMD ; i++) {
		lcmd[i] = 0;
	}
	lcmd_count = 0;	// taille courante du tableau de la liste des commandes
	lcmd_pos = 0;	// position de la commande courante en cours d'exécution
}

void init_etat() {
	vitesse_gauche = 0;
	vitesse_droit = 0;

	for (uint8_t i = 0; i < (SENSOR_COUNT+SENSOR_COUNT); i++) {
		dist[i] = 0;
	}

	time = 0;
	timeout = 0;
}

/*******************************************************************/
/************************ reset Arduino ****************************/
/*******************************************************************/

void reset_arduino() { 
  //int pin=12; 
  // active la broche en sortie (OUTPUT)  
  //pinMode(12, OUTPUT); 
  // Déactive le reset forçant la sortie au niveau bas 
  //digitalWrite(12, LOW);
  asm volatile ("  jmp 0"); 
}

/*******************************************************************/
/******* Arduino état - Envoi de l'état de l'Arduino en JSON *******/
/*******************************************************************/

void arduino_etat() {
  Serial.print("{");
  Serial.print("\"m\":[");
  Serial.print(vitesse_gauche);
  Serial.print(",");
  Serial.print(vitesse_droit);
  Serial.print("],\"d\":");
	Serial.print("[");
	for (uint8_t i = 0; i < (SENSOR_COUNT+SENSOR_COUNT); i++) {
		if (i > 0) {
			Serial.print(",");
		}
		Serial.print(dist[i]);
 	}
	Serial.print("]");

  Serial.println("}");
}
