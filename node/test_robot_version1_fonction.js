

var { SerialPort } = require('serialport')



var tty_xbee = "COM6" // port COM sous Windows (voir chgport))


/***************************************************************************************************/
/********************************************** XBee ***********************************************/
/***************************************************************************************************/

var xbee = new SerialPort({ // fonctionne pour USB et XBee
    path : tty_xbee,
    baudRate: 9600,
  });

var chunk = "" // pour stocker les données qui arrivent par XBee jusqu'à ce qu'on obtienne une ligne complète

xbee.on('open', function() {
    console.log('opened : '+tty_xbee);
    
    
});

xbee.on('error', function(err) {
    console.log('error ', err.message)
    console.log("Modifier la variable tty_xbee dans robot_l3/node/config.js")
    console.log("La variable tty_xbee doit contenir le nom du port")
    console.log("/dev/ttyACM0 sous Linux")
    console.log("COMx sous Windows où x est un numéro (COM3, COM4, ...)")
    console.log("Sous Windows taper chgport dans une invite de commande pour voir les ports COM utilisés")

    process.exit(0)
});

xbee.on('close', function () {
  console.log("closed ");
});

xbee.on('data', function(data) {
  //console.log("data = "+data)
  chunk += data;
  //console.log(typeof(port.data)) 
  for (;;) {
      var pos = chunk.indexOf("\n");
      if (pos < 0) break;
      //console.log(usb.chunk.substring(0,pos));
      var res = chunk.substring(0,pos)
      console.log("XBee : "+res)

      try {
        var o = JSON.parse(res)
        //larduino.push(res + " [" + date.HMS() + "]")
        //dist = o.dist
        etat_arduino = o
      }
      catch (err) {
        console.log("Erreur JSON.parse")
        // ca doit être un message de l'Arduino
        //larduino.push(res + " [" + date.HMS() + "]")
      }


      chunk = chunk.substring(pos+1);
  }
})

/**************************************************************************************/
/**************************************** test ****************************************/
/**************************************************************************************/

setTimeout(() => {
		xbee.write("[[t 1000][mda 200][mga 200]]")
	}, 4000);

	setTimeout(() => {
		xbee.write("[[t 1000][mdr 200][mgr 200]]")
	}, 6000);

	setTimeout(() => {
		process.exit(0)
	}, 8000);

		