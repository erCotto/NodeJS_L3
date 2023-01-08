
// mission longer
// si distance gauche trop faible on s'éloigne du mur
// si distance gauche trop grand on se rapproche dur mur
// sinon on avance

const D_AVANT = 0   // constantes pour accÃ©der au tableau des distances
const D_GAUCHE = 2
const D_ARRIERE = 4
const D_DROITE = 6

const MAXITER = 100   // nombre max d'itÃ©rations

var dist = null    // distance d'arrÃªt en mm

var time = null     // gestion du temps (initialisÃ© par Date.now(), nombre de ms depuis 1970)
var periode = 1000  // envoi d'une commande au robot toutes les 1000 ms

var count = MAXITER   // nombre max de commandes pouvant Ãªtre envoyÃ©es au robot

var a_tourner = 0;
var sens = 0;  //  1 --> se rapproche | 2 --> s'éloigne | 0 --> avance
var distance_p = 0; // distance précédente

function init(d) {

    console.log("init longer_mur")
    console.log("dist = "+d)

    count = MAXITER

    dist = d

    time = Date.now()
}

function exec(etat_arduino) {

    // La fonction exec est appelÃ©e automatiquement toutes les 200 ms depuis serveur_http.js (voir la fonction check)
    // VÃ©rifier que dans check l'appel se fait de la faÃ§on suivante : mission_exec(etat_arduino)

    // la fonction retourne :
    // null (fin de la mission ou erreur)
    // une chaÃ®ne de caractÃ¨res non vide (la commande Ã  envoyer au robot)
    // une chaÃ®ne de caractÃ¨res vide (rien Ã  faire, pas de commande Ã  envoyer au robot)
    
    console.log("exec longer")

    let cmd = "" // commande Ã  envoyer au robot

    if (time == null) {
        console.log("erreur longer time = null")
        return null // erreur, fin de la mission
    }

    if (Date.now() > time + periode) { // on passe ici une fois par seconde (cas periode = 1000)
        time = Date.now()
        
        console.log("distances : "+JSON.stringify(etat_arduino))
        console.log("distance gauche = "+etat_arduino.d[D_GAUCHE])

        count--

        if (count <= 0) {
            cmd = null // nombre d'itÃ©rations trop Ã©levÃ©, mission terminÃ©e
        }
        else if ( (etat_arduino.d[D_GAUCHE] > dist + 100 ) && (a_tourner == 0) && (sens != 1) ) {
            cmd = "[[t 600][mda 200][mga 0]]" // distance trop grande, on tourne à gauche pendant 600 ms
            a_tourner = 1;
            sens = 1;
            //distance_p = etat_arduino.d[D_GAUCHE];
            //console.log("je dois tourner a gauche")
            //console.log("a touner "+a_tourner)
        }
        else if ( (etat_arduino.d[D_GAUCHE] < dist ) && (a_tourner == 0) && (sens != 2) ) {
            cmd = "[[t 600][mda 0][mga 200]]" // distance trop faible, on tourne à droite pendant 600 ms
            a_tourner = 1;
            sens = 2;
            //distance_p = etat_arduino.d[D_GAUCHE];
            //console.log("je dois tourner a droite")
            //console.log("a touner "+a_tourner)
        }
        else if ( (dist - 100 < etat_arduino.d[D_GAUCHE] < dist + 100) || (a_tourner == 1) ) {
            cmd = "[[t 100][mda 255][mga 255]]" // distance suffisante, on avance pendant 100 ms
            a_tourner = 0;
            //distance_p = etat_arduino.d[D_GAUCHE];
            //console.log("je dois avancer")
            //console.log("a touner "+a_tourner)
        }
        else {
            cmd = null
        }
        
    
    }


    return cmd
}

module.exports = {
    init : init,
    exec : exec,
}
