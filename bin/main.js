var samp = require("samp");

process.on("uncaughtException", function() {

});

samp.setPublicCallHandler(function(name) {
	console.log("SAMP has triggered a public callback: " + name);
});

setInterval(function() {
}, 100);
