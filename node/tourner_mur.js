
// mission tourner mur
// si la distance devant est trop faible on tourne à gauche
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

function init(d) {

    console.log("init tourner_mur")
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
    
    console.log("exec tourner_mur")

    let cmd = "" // commande Ã  envoyer au robot

    if (time == null) {
        console.log("erreur tourner_mur time = null")
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
            cmd = "[[t 100][mda 250][mga 250]]" // distance suffisante, on avance pendant 100 ms
        }
        else {
            cmd = "[[t 600][mda 0][mga 200]]" // distance trop faible, on tourne à droite, mission terminée
            count = 0
        }
        
    
    }


    return cmd
}

module.exports = {
    init : init,
    exec : exec,
}
