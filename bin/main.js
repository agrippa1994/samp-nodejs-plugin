var samp = require("samp");

samp.onPublic("OnGameModeInit", "", function() {
	console.log("OnGameModeInit!");
});

samp.onPublic("AddIntsInJS", "dd", function(a, b) {
	console.log("AddIntsInJS in JS " + a + " " + b);
	return { setReturnValueTo: a+b };
});

samp.onPublic("PrintStringInJS", "ds", function(d, text) {
	console.log("String from PAWN: " + text);
});
