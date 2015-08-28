#ifndef AMX2JX_HPP
#define AMX2JX_HPP
#include "jx.hpp"
#include "../jxcore/jx.h"
#include "../jxcore/jx_result.h"
#include "../sampgdk/interop.h"

namespace amx2jx {

	// This class is used to convert values from PAWN to JavaScript (e.g. public calls)
	class amx2jx{
		AMX *amx_;
	public:
		amx2jx(AMX *amx, cell *params) : amx_(amx) {

		}

		void operator()(JXValue *func, JXValue *output) {

		}
	};
}

#endif