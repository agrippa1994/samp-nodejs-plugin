#include "sampsdk/plugin.h"
#include "Publics.hpp"
#include "JX.hpp"

typedef void(*logprintf_t)(char* format, ...);

logprintf_t logprintf;
extern void *pAMXFunctions;

void uncaughtException(JXResult *args, int argc) {
	logprintf("Exception %s", JX_GetString(&args[0]));
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	Publics::init(ppData);
	
	JX_Initialize("", NULL);
	JX_InitializeNewEngine();

	JX_DefineExtension("setPublicCallHook", Publics::Internal::JavaScriptBinder::setPublicCallHook);
	JX_DefineExtension("uncaughtException", uncaughtException);

	JX_DefineFile("samp", R"(
		// Bind natives to the module
		module.exports = process.natives;

		// Fetch all uncaught exceptions
		process.on("uncaughtException", function(e) {
			process.natives.uncaughtException(e);
		});

		// Support for hooks of public functions
		var callbackEntries = [];
		module.exports.onPublic = function(name, format, callback) {
			var idx = -1;
			callbackEntries.forEach(function(entry, i) {
				if(entry.name == name && entry.format == format)
					idx = i;
			});

			if(idx == -1)
				callbackEntries.push({name: name, format: format, callbacks: [callback]});
			else
				if(callbackEntries[idx].callbacks.indexOf(callback) == -1)
					callbackEntries[idx].callbacks.push(callback);

			process.natives.setPublicCallHook(callbackEntries);
		};

		module.exports.removePublicListener = function(name, callback) {
			callbackEntries.forEach(function(entry) {
				if(name != entry.name)
					return;

				var idx = -1;
				if((idx = entry.callbacks.indexOf(callback)) != -1)
					entry.callbacks.splice(idx, 1);
			});

			process.natives.setPublicCallHook(callbackEntries);
		};

		module.exports.removeAllPublicListeners = function(name) {
			var idx = -1;
			callbackEntries.forEach(function(entry, i) {
				if(entry.name == name)
					idx = i;
			});

			if(idx != -1)
				callbackEntries.splice(idx, 1);

			process.natives.setPublicCallHook(callbackEntries);
		};

		setInterval(function() { }, 100);
	)");

	JX_StartEngine();
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	Publics::exit();
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	JX_LoopOnce();
}
