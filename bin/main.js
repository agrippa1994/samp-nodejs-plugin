var samp = require("samp");

samp.onPublic("OnGameModeInit", "", function() {
	console.log("OnGameModeInit!");
});

samp.onPublic("AddIntsInJS", "dd", function(a, b) {
	console.log("AddIntsInJS in JS " + a + " " + b);
	return { setReturnValueTo: a+b };
});


samp.onPublic("StrlenInJS", "s", function(text) {
	console.log("PAWN wants to know, how long the given string is (" + text + ")");
    return { setReturnValueTo: text.length, skipPublic: true };
});

samp.onPublic("OnGameModeExit", "", function() {
    console.log("JavaScript execution is stopped!");
});

samp.onPublic("OnFilterScriptExit", "", function() {
    console.log("Filterscript exit!");
});