
function YMDHMS(millis) {
    var date = null

    if (typeof millis === 'undefined') { // comparaison type et valeur
        date = new Date()
    }
    else {
        date = new Date(millis)
    }
 
    var d =
        date.getFullYear() + "_" +
        ("0" + (date.getMonth()+1)).slice(-2) + "_" +
        ("0" + date.getDate()).slice(-2) +  "__" +
        ("0" + date.getHours()).slice(-2) + "_" +
        ("0" + date.getMinutes()).slice(-2) + "_" +
        ("0" + date.getSeconds()).slice(-2)

    return d

}

function YMDHMS_tirets(millis) {
    var date = null

    if (typeof millis === 'undefined') { // comparaison type et valeur
        date = new Date()
    }
    else {
        date = new Date(millis)
    }
 
    var d =
        date.getFullYear() + "-" +
        ("0" + (date.getMonth()+1)).slice(-2) + "-" +
        ("0" + date.getDate()).slice(-2) +  " " +
        ("0" + date.getHours()).slice(-2) + ":" +
        ("0" + date.getMinutes()).slice(-2) + ":" +
        ("0" + date.getSeconds()).slice(-2)

    return d

}

function YMDHMS(millis) {
    var date = null

    if (typeof millis === 'undefined') { // comparaison type et valeur
        date = new Date()
    }
    else {
        date = new Date(millis)
    }
 
    var d =
        date.getFullYear() + "_" +
        ("0" + (date.getMonth()+1)).slice(-2) + "_" +
        ("0" + date.getDate()).slice(-2) +  "__" +
        ("0" + date.getHours()).slice(-2) + "_" +
        ("0" + date.getMinutes()).slice(-2) + "_" +
        ("0" + date.getSeconds()).slice(-2)

    return d

}

function YMDHMS(millis) {
    var date = null

    if (typeof millis === 'undefined') { // comparaison type et valeur
        date = new Date()
    }
    else {
        date = new Date(millis)
    }
 
    var d =
        date.getFullYear() + "_" +
        ("0" + (date.getMonth()+1)).slice(-2) + "_" +
        ("0" + date.getDate()).slice(-2) +  "__" +
        ("0" + date.getHours()).slice(-2) + "_" +
        ("0" + date.getMinutes()).slice(-2) + "_" +
        ("0" + date.getSeconds()).slice(-2)

    return d

}

function HMS() {
    var date = new Date()
 
    var d = "" +
        ("0" + date.getHours()).slice(-2) + ":" +
        ("0" + date.getMinutes()).slice(-2) + ":" +
        ("0" + date.getSeconds()).slice(-2)

    return d

}


function log() {
    // 12/Apr/2022:14:19

    const monthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]

    var date = new Date()

    var d = []
    var d0 = ("0" + date.getDate()).slice(-2)+"/"+monthNames[date.getMonth()]+"/"+date.getFullYear()+":"+("0" + date.getHours()).slice(-2) + ":"
    d.push(d0 + ("0" + date.getMinutes()).slice(-2))
    d.push(d0 + ("0" + (date.getMinutes()-1)).slice(-2))

    return d
}

// test

// console.log("YMDHMS() : "+YMDHMS())
// console.log("YMDHMS(1000) : "+YMDHMS(1000))
console.log("dateLog : "+log())

module.exports = {
    YMDHMS : YMDHMS,
    YMDHMS_tirets : YMDHMS_tirets,
    HMS : HMS,
    log : log,
}
