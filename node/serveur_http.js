
var express = require('express')
var USB = require("./usb.js").USB

var date = require("./date.js")

var config = require('./config.js')
var m_avancer_obstacle = require('./m_avancer_obstacle.js')
var longer = require('./longer.js')
var tour = require('./tour.js')
var tourner_mur = require('./tourner_mur.js')
var base = config.base
var port_http = config.port_http
var tty_xbee = config.tty_xbee


/***************************************************************************************************/
/***************************************** état du robot *******************************************/
/***************************************************************************************************/

var etat_arduino = null
var mission_exec = null

// objet contenant des tableaux
// m : tableau vitesse des moteurs [gauche, droit]
// d : tableau des distances [avant_mm, avant_fiabilite, gauche_mm, gauche_fiabilite, arriere_mm, arriere_fiabilite, droit_mm, droit_fiabilite]

// positions dans les tableaux

const vitesse_gauche = 0    // positif marche avant, négatif arrière
const vitesse_droit = 1     // positif marche avant, négatif arrière

const avant_mm = 0
const avant_fiabilite = 1
const gauche_mm = 2
const gauche_fiabilite = 3
const arriere_mm = 4
const arriere_fiabilite = 5
const droit_mm = 6
const droit_fiabilite = 7

// exemple de contenu
// {"m":[200,-200],"d":[320,3,1299,0,542,0,237,2]}

/***************************************************************************************************/
/*************************************** envoi des commandes au robot ******************************/
/***************************************************************************************************/

// Il faut centraliser l'envoi des commandes
// Sinon elles vont se mélanger pendant l'envoi par XBee
// quand une commande arrive par HTTP, elle est placée en file d'attente dans lcmd

// la fonction check envoie une commande à l'Arduino toutes les 200 ms
// s'il n'y a pas de commande dans lcmd, on envoie la commande NOP
// les réponses de l'Arduino arrivent dans la fonction de xbee.on('data', ...)
// si la réponse est un objet JSON, elle va dans etat_arduino
// si la réponse n'est pas un objet JSON, elle va dans larduino

var lcmd = []   // liste des commandes en attente
var larduino = []   // liste des messages de l'Arduino

var check_periode = 200     // communication avec le robot toutes les 200 ms

var time_check = new Date().getTime()

setTimeout(() => {
    check()
}, 1000);

function check() {
    if (new Date().getTime() > time_check + 10000) { //exécution toutes les 10 secondes
        console.log("check")
        time_check = new Date().getTime()
    }

    if (mission_exec != null) {
        let res = mission_exec(etat_arduino) // retourne la prochaine commande à envoyer à  l'Arduino
        if (res == null) {
            console.log("mission terminée")
            mission_exec = null
        }
        else {
            if (res.length > 0) lcmd.push(res)
        }
    }

    envoi_cmd()

    setTimeout(() => {
        check()
    }, check_periode);

}

function envoi_cmd() {

    if( lcmd.length == 0) {
        lcmd.push("[[n 0]]") // s'il n'y a aucune commande en attente, on envoie [[n 10]] pour obtenir l'état du robot
    }

    for (;;) {
        if (lcmd.length == 0) {
            break
        }
        var c = lcmd[0]
        lcmd = lcmd.slice(1)
        if (xbee == null) {
            console.log("Erreur envoi_cmd (xbee null)")
        }
        else {
            xbee.port.write(c)
        }
        //console.log("envoyé : "+c)
    }
}
/***************************************************************************************************/
/********************************************** XBee ***********************************************/
/***************************************************************************************************/

var xbee = new USB(tty_xbee, {
    callback:
      function (s) {
        //console.log("USB : " + s)
        try {
            var o = JSON.parse(s)
            //larduino.push(res + " [" + date.HMS() + "]")
            //dist = o.dist
            etat_arduino = o
          }
          catch (err) {
            // ca doit être un message non JSON de l'Arduino
            larduino.push(s + " [" + date.HMS() + "]")
          }
    
      }
  })



/***************************************************************************************************/
/******************************************** serveur HTTP *****************************************/
/***************************************************************************************************/

var app = express();
app.use("/", express.static(base+"/html", { index: 'index.html' }))
app.use(express.json({ limit: '50mb' }))
app.use(express.urlencoded({ extended: true }))

var server = app.listen(port_http, function () {
    console.log('Express server listening on port ' + port_http)
});

// test
app.get("/test", function (req, res) {
    console.log("GET test")
    res.send({ mess: "Réponse du serveur : requête test reçue" })
})

app.post("/cmd", function (req, res) {
    //console.log(util.inspect(req.body))
    console.log("POST cmd val = "+req.body.val)
    lcmd.push(req.body.val) // commande mise en file d'attente
    //xbee.write(req.body.val)
    res.send({ mess: "Commande envoyée au robot : "+ req.body.val, etat : etat_arduino, mess_arduino : larduino}) // on retourne systématiquement l'état de l'Arduino
    larduino = []
})

app.post("/etat_arduino", function(req, res){
    console.log("POST etat_arduino")
 
    res.send({ etat : etat_arduino}) 
	// on retourne l'état de l'Arduino
})

app.post("/mission", function (req, res) {

    console.log("POST mission")
    var nom = req.body.nom      // nom de la mission
    var param1 = req.body.param1  // paramètre 1 de la mission
    var param2 = req.body.param2  // paramètre 2 de la mission

    if (nom == "avancer_obstacle") {
        m_avancer_obstacle.init(parseInt(param1))
        mission_exec = m_avancer_obstacle.exec
    }
    if (nom == "longer") {
        longer.init(parseInt(param1))
        mission_exec = longer.exec
    }
    if (nom == "tourner_mur") {
        tourner_mur.init(parseInt(param1))
        mission_exec = tourner_mur.exec
    }
    if (nom == "tour"){
        tour.init(parseInt(param1))
        mission_exec = tour.exec
    }
    else {
        console.log("mission inconnue : "+nom)
    }

    res.send({ mess: "mission en cours d'exécution : " +nom}) 
    
})