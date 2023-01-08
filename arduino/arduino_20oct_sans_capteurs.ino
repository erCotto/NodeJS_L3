
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


char buf_in[BUF_IN];	// buffer de lecture
int buf_in_count = 0;	// taille courante du buffer

int lcmd[LCMD]; 	// tableau de la liste des commandes à exécuter
// toujours 2 entiers par commande, le 1er pour la commande, le 2e pour le paramètre)
int lcmd_count = 0;	// taille courante du tableau de la liste des commandes
int lcmd_pos = 0;	// position de la commande courante en cours d'exécution

/**************************************************************************************/
/******************************** Etat du robot ***************************************/
/**************************************************************************************/

int vitesse_gauche = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255)
int vitesse_droit = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255)

int direction_gauche = 0;	// 0 : vers l'arrière ; 1 : vers l'avant
int direction_droit = 0;	// 0 : vers l'arrière ; 1 : vers l'avant

unsigned long time = 0;		// timeout d'arrêt des moteurs désactivé
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

	delay(2000);

	Serial.println("Arduino OK");


}

/**************************************************************************************/
/************************************* loop *******************************************/
/**************************************************************************************/


int c = -1;		// caractère courant lu
int c_prec = -1;	// caractère précédent lu

void loop() {


	if (Serial.available()) {
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

	if (lcmd_count != 0) { // si séquence de commandes en cours d'exécution
		exec_cmd();
	}

	if (time != 0) { // si timeout activé
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


void moteur_init() {
}

void moteur_gauche_avant(int speed) {
 Serial.println("moteur_gauche_avant");
  vitesse_gauche = speed;
  direction_gauche = 1;
}

void moteur_gauche_arriere(int speed) {
  vitesse_gauche = speed;
  direction_gauche = 0;
}

void moteur_droit_avant(int speed) {
  vitesse_droit = speed;
  direction_droit = 1;
}

void moteur_droit_arriere(int speed) {
  vitesse_droit = speed;
  direction_droit = 0;
}

void moteur_stop() {
  vitesse_gauche = 0;
  direction_gauche = 0;
  vitesse_droit = 0;
  direction_droit = 0;
}





/*******************************************************************/
/******************* VL53L1X Capteurs de distance ******************/
/*******************************************************************/


void vl53l1x_init() {
}

void vl53l1x_print() {
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
	direction_gauche = 0;
	vitesse_droit = 0;
	direction_droit = 0;

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
  Serial.print("\"vitesse_gauche\" : ");
  Serial.print(vitesse_gauche);
  Serial.print(", \"direction_gauche\" : ");
  Serial.print(direction_gauche);
  Serial.print(", \"vitesse_droit\" : ");
  Serial.print(vitesse_droit);
  Serial.print(", \"direction_droit\" : ");
  Serial.print(direction_droit);
  Serial.println("}");
}



