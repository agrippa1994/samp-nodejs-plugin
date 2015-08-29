var samp = require("samp");

samp.onPublic("OnGameModeInit", "", function() {
	console.log("OnGameModeInit!");
});

samp.onPublic("AddIntsInJS", "dd", function(a, b) {
	return { setReturnValueTo: a+b };
});
