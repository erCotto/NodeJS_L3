
// mission tour piece
// on fait avancer le robot pendant 100 ms
// on attend 900 ms qu'il s'arrÃªte
// si la distance à gauche est inférieure à la distance gauche précédente, tourne à droite
// si la distance avant est trop faible, tourne à gauche
// sinon avance

const D_AVANT = 0   // constantes pour accÃ©der au tableau des distances
const D_GAUCHE = 2
const D_ARRIERE = 4
const D_DROITE = 6

const MAXITER = 100   // nombre max d'itÃ©rations

var dist = null    // distance d'arrÃªt en mm

var time = null     // gestion du temps (initialisÃ© par Date.now(), nombre de ms depuis 1970)
var periode = 1000  // envoi d'une commande au robot toutes les 1000 ms

var count = MAXITER   // nombre max de commandes pouvant Ãªtre envoyÃ©es au robot

var dist_p = 0
var it = 0 // compte le nombre ditérations depuis lesquelles on a mesuré une distance plus grande que la précédente sur la gauche

function init(d) {

    console.log("init tour_piece")
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
        console.log("erreur avancer_obstacle time = null")
        return null // erreur, fin de la mission
    }

    if (Date.now() > time + periode) { // on passe ici une fois par seconde (cas periode = 1000)
        time = Date.now()
        
        console.log("distances : "+JSON.stringify(etat_arduino))
        console.log("distance gauche = "+etat_arduino.d[D_GAUCHE])
        //console.log("distance precedente = "+dist_p)
        console.log("it = "+it)

        count--

        if (count <= 0) {
            cmd = null // nombre d'itÃ©rations trop Ã©levÃ©, mission terminÃ©e
        }
        else if ( etat_arduino.d[D_AVANT] < dist ) {
            //dist_p = etat_arduino[D_GAUCHE];
            cmd = "[[t 700][mda 200][mga 0]]" // mur, on tourne à gauche pendant 700 ms
            //it = 0;
        } 
        if( it >= 5 ){
            it = 0;
            //dist_p = etat_arduino[D_GAUCHE];
            cmd = "[[t 700][mda 0][mga 200]]" // ouverture, on tourne à droite pendant 700 ms
        } else {
            console.log("dist = "+dist)
            if( etat_arduino[D_GAUCHE] > dist ){
                it++;
            } else {
                console.log("top")
                it = 0;
            }
            if( it == 0 ){ // mise à jour de la distance précédente
                dist_p = etat_arduino[D_GAUCHE];
            }
            cmd = "[[t 100][mda 255][mga 255]]" // distance suffisante, on avance pendant 100 ms
        }
        
    
    }


    return cmd
}

module.exports = {
    init : init,
    exec : exec,
}
