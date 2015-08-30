

#include <sampgdk/core.h>
#include <sampgdk/sdk.h>
#include <sampgdk/interop.h>

#include "Publics.hpp"
#include "JX.hpp"
#include "AMX2JX.hpp"
#include "JX2AMX.hpp"
#include <stdio.h>
#include <string>
#include <map>

class PublicCallHookEntry {
	std::string name_;
	std::string format_;
	std::vector<JXValue> callbacks_;

public:
	~PublicCallHookEntry() {
		for (auto& i : callbacks_)
			JX_ClearPersistent(&i);
	}

	void setName(const std::string& name) {
		name_ = name;
	}

	void setFormat(const std::string& format) {
		format_ = format;
	}

	void addCallback(JXValue& func) {
		JX_MakePersistent(&func);
		callbacks_.push_back(func);
	}

	const std::string& name() const {
		return name_;
	}

	const std::string& format() const {
		return format_;
	}

	std::vector<JXValue>& callbacks() {
		return callbacks_;
	}
};

std::vector<std::shared_ptr<PublicCallHookEntry>> publicCallHookEntries;
void setPublicCallHook(JXResult *args, int argc) {
	if (argc == 0)
		return JX_SetBoolean(args + argc, false);

	publicCallHookEntries.clear();
	bool ret = false;
	for (int i = 0;; i++) {
		auto entry = std::make_shared<PublicCallHookEntry>();

		JX::ScopedValue jxCallParamsDesc;
		JX_GetIndexedProperty(&args[0], i, &jxCallParamsDesc);
		if (JX_IsNullOrUndefined(&jxCallParamsDesc))
			return JX_SetBoolean(args + argc, ret);
		
		JX::ScopedValue jxName, jxFormat, jxCallbacks;
		JX_GetNamedProperty(&jxCallParamsDesc, "name", &jxName);
		JX_GetNamedProperty(&jxCallParamsDesc, "format", &jxFormat);
		JX_GetNamedProperty(&jxCallParamsDesc, "callbacks", &jxCallbacks);

		if (!JX_IsString(&jxName) || !JX_IsString(&jxFormat) || !JX_IsObject(&jxCallbacks))
			return JX_SetBoolean(args + argc, false);

		entry->setName(jxName.stringValue());
		entry->setFormat(jxFormat.stringValue());

		for (int u = 0;; u++) {
			JXValue jxCallbackFunction;
			JX_GetIndexedProperty(&jxCallbacks, u, &jxCallbackFunction);
			if (!JX_IsFunction(&jxCallbackFunction))
				break;

			entry->addCallback(jxCallbackFunction);
		}

		publicCallHookEntries.push_back(entry);
	}
}

void uncaughtException(JXResult *args, int argc) {

	sampgdk::logprintf("Exception %s", JX_GetString(&args[0]));
}

bool OnPublic(AMX *amx, const char *name, cell *params, cell *retval) {
	cell *ph = 0;
	/*
	if (!strcmp("PrintStringInJS", name)) {
		amx_GetAddr(amx, params[2], &ph);
		std::cout << "asdfasdfasfasfd" << std::endl;
	}
	*/

	bool skipPublic = false;
	try {
		for (const auto entry : publicCallHookEntries) {
			if (std::string(name) == entry->name()) {
				JX::ScopedValue jxFunctionName;
				JX_SetString(&jxFunctionName, name);
				
				for (auto& callback : entry->callbacks()) {
					JX::ScopedValue jxFunctionReturnValue;
					AMX2JX::AMX2JX(amx, entry->format(), params)(&callback, &jxFunctionReturnValue);

					if (JX_IsObject(&jxFunctionReturnValue)) {
						JX::ScopedValue jxSkipPublic, jxSetReturnValueTo;
						JX_GetNamedProperty(&jxFunctionReturnValue, "skipPublic", &jxSkipPublic);
						JX_GetNamedProperty(&jxFunctionReturnValue, "setReturnValueTo", &jxSetReturnValueTo);

						if (JX_IsBoolean(&jxSkipPublic))
							if (JX_GetBoolean(&jxSkipPublic))
								skipPublic = true;

						if (JX_IsInt32(&jxSetReturnValueTo) && retval)
							*retval = JX_GetInt32(&jxSetReturnValueTo);
					}
				}
			}
		}
	}
	catch (const std::exception& e) {
		sampgdk::logprintf("Exception: %s", e.what());
	}
	catch (...) {
		sampgdk::logprintf("Unknown exception");
	}
	
	// Return true when the public is allowed to be executed
	return !skipPublic;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return sampgdk::Supports() | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	bool loadResult = sampgdk::Load(ppData);
	Publics::init(ppData);
	Publics::setPublicCallHandler(OnPublic);

	JX_Initialize("", NULL);
	JX_InitializeNewEngine();

	JX_DefineExtension("setPublicCallHook", setPublicCallHook);
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
	return loadResult;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	sampgdk::Unload();
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	JX_LoopOnce();
	sampgdk::ProcessTick();
}
