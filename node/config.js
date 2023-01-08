
var fs = require('fs')

var base = "chemin_d_acces"
var port_http = 3200


var tty_xbee = "/dev/ttyACM0" // XBee Arduino
//var tty_xbee = "/dev/ttyUSB0" // USB Arduino
// sous Windows sélectionner un port COM (voir chgport)
console.log("config : base = "+base)
console.log("config : port_http = "+port_http)
console.log("config : tty_xbee = "+tty_xbee)

if (!fs.existsSync(base)) {
    console.log("Erreur config : le dossier " + base + " n'existe pas")
    console.log("Modifier la variable base dans robot/node/config.js")
    console.log("La variable base doit contenir un chemin absolu ")
    console.log("Le programme a été arrêté ")
    process.exit(0)
}

module.exports = {
    base : base,
    port_http : port_http,
    tty_xbee : tty_xbee,
}
