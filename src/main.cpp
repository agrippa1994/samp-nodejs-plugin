#include <stdio.h>
#include <string.h>

#include <sampgdk/core.h>
#include <sampgdk/sdk.h>
#include <sampgdk/interop.h>
#include <plugin/jx.hpp>
#include <plugin/amx2jx.hpp>
#include <plugin/jx2amx.hpp>

JXResult publicCallHandler;
void setPublicCallHandler(JXResult *args, int argc) {
	if (argc > 0) {
		if (JX_IsFunction(&args[0])) {
			JX_MakePersistent(&args[0]);

			sampgdk::logprintf("Stored: %d", publicCallHandler.data_);
			publicCallHandler = args[0];
			sampgdk::logprintf("Stored: %d", publicCallHandler.data_);
		}
	}
}

void unsetPublicCallHandler(JXResult *args, int argc) {
	if (publicCallHandler.data_ == NULL)
		return JX_SetBoolean(args + argc, false);

	JX_ClearPersistent(&publicCallHandler);
	publicCallHandler.data_ = NULL;

	return JX_SetBoolean(args + argc, true);
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPublicCall(AMX *amx, const char *name, cell *params, cell *retval) {

	JX::ScopedValue jxName;

	JX_SetString(&jxName, name);

	JXValue out;
	JX_CallFunction(&publicCallHandler, &jxName, 1, &out);

	bool skipPublic = false;
	if (JX_IsObject(&out)) {
		JXValue jxSkipPublic, jxSetReturnValueTo;
		JX_GetNamedProperty(&out, "skipPublic", &jxSkipPublic);
		JX_GetNamedProperty(&out, "setReturnValueTo", &jxSetReturnValueTo);

		if (JX_IsBoolean(&jxSkipPublic))
			skipPublic = JX_GetBoolean(&jxSkipPublic);

		if (JX_IsInt32(&jxSetReturnValueTo) && retval)
			*retval = JX_GetInt32(&jxSetReturnValueTo);
	}

	// Return true when the public is allowed to be executed
	return !skipPublic;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return sampgdk::Supports() | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	bool loadResult = sampgdk::Load(ppData);

	JX_Initialize("", NULL);
	JX_InitializeNewEngine();

	JX_DefineExtension("setPublicCallHandler", setPublicCallHandler);
	JX_DefineExtension("unsetPublicCallHandler", unsetPublicCallHandler);
	JX_DefineFile("samp", R"(
		module.exports = process.natives;
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