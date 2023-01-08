
// mission avancer obstacle
// on fait avancer le robot pendant 100 ms
// on attend 900 ms qu'il s'arrÃªte
// si la distance vers l'avant est infÃ©rieure Ã  une valeur donnÃ©e, on arrÃªte la mission

const D_AVANT = 0   // constantes pour accÃ©der au tableau des distances
const D_GAUCHE = 2
const D_ARRIERE = 4
const D_DROITE = 6

const MAXITER = 100   // nombre max d'itÃ©rations

var dist = null    // distance d'arrÃªt en mm

var time = null     // gestion du temps (initialisÃ© par Date.now(), nombre de ms depuis 1970)
var periode = 1000  // envoi d'une commande au robot toutes les 1000 ms

var count = MAXITER   // nombre max de commandes pouvant Ãªtre envoyÃ©es au robot

function init(d) {

    console.log("init avancer_obstacle")
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
    
    console.log("exec avancer_obstacle")

    let cmd = "" // commande Ã  envoyer au robot

    if (time == null) {
        console.log("erreur avancer_obstacle time = null")
        return null // erreur, fin de la mission
    }

    if (Date.now() > time + periode) { // on passe ici une fois par seconde (cas periode = 1000)
        time = Date.now()
        
        console.log("distances : "+JSON.stringify(etat_arduino))
        console.log("distance avant = "+etat_arduino.d[D_AVANT])

        count--

        if (count <= 0) {
            cmd = null // nombre d'itÃ©rations trop Ã©levÃ©, mission terminÃ©e
        }
        else if (etat_arduino.d[D_AVANT] > dist) {
            cmd = "[[t 100][mda 255][mga 255]]" // distance suffisante, on avance pendant 100 ms
        }
        else {
            cmd = null  // distance trop faible, mission terminÃ©e
        }
        
    
    }


    return cmd
}

module.exports = {
    init : init,
    exec : exec,
}
