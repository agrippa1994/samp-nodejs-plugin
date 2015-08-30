#ifndef PUBLICS_HPP
#define PUBLICS_HPP
#include "detours.h"
#include <functional>
#include <iostream>
#include <string>
#include <vector>


namespace Publics {
	typedef std::function<bool(AMX *, const char *, cell *, cell *)> CallbackHandler;

	namespace Internal {
		Hook<convention_type::cdecl_t, int, AMX *, cell *, int> execHook;
		Hook<convention_type::cdecl_t, int, AMX *, const char *, int *> findPublicHook;
		CallbackHandler publicCallHandler;
		std::string currentPublic;
	}

	void init(void **ppData) {
		Internal::findPublicHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_FindPublic], [](AMX *amx, const char *name, int *idx) -> int {
			Internal::currentPublic = name;
			return Internal::findPublicHook.callOrig(amx, name, idx);
		});

		Internal::execHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_Exec], [](AMX *amx, cell *retVal, int idx) -> int {
			bool shouldExecuteCallback = false;

			auto hdr = (AMX_HEADER *)amx->base;
			auto dat = (amx->data != nullptr) ? amx->data : amx->base + (int)hdr->dat;
			auto stk = (cell *)(dat + amx->stk);

			std::vector<cell> params { amx->paramcount * (int)sizeof(cell) };
			for (int i = 0; i < amx->paramcount; i++)
				params.push_back(stk[i]);

			if (Internal::publicCallHandler) {
				shouldExecuteCallback = Internal::publicCallHandler(amx, Internal::currentPublic.c_str(), params.data(), retVal);
			
				if (!shouldExecuteCallback)
					return *retVal;
			}

			return Internal::execHook.callOrig(amx, retVal, idx);
		});
		
	}

	void exit() {
		Internal::execHook.remove();
	}

	void setPublicCallHandler(CallbackHandler callback) {
		Internal::publicCallHandler = callback;
	}
}

#endif