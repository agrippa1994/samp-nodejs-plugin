#ifndef AMX2JX_HPP
#define AMX2JX_HPP
#include "jx.hpp"
#include "../jxcore/jx.h"
#include "../jxcore/jx_result.h"
#include "../sampgdk/interop.h"
#include <string>
#include <sstream>
#include <vector>
#include <memory>

namespace amx2jx {

	// This class converts an amx value to a js value
	class amx2jxValue {
		enum type {
			type_unknown = 0, type_int, type_float, type_string
		};

		cell value_ = 0;
		type type_ = type_unknown;
		bool isReference = false;

	public:
		amx2jxValue(cell value) : value_(value) {

		}

		friend std::stringstream& operator>>(std::stringstream& is, amx2jxValue& val) {
			
		}
	};

	// This class is used to convert values from PAWN to JavaScript (e.g. public calls)
	class amx2jx{
		AMX *amx_;
		std::vector<std::shared_ptr<amx2jxValue>> jxValues_;

	public:
		amx2jx(AMX *amx, const std::string& format, cell *params) : amx_(amx) {
			std::stringstream ss(format);
			
			for (int i = 1; i < params[0] + 1; i++) {
				auto value = std::make_shared<amx2jxValue>(params[i]);
				ss >> *value;
			}
		}

		void operator()(JXValue *func, JXValue *output) {

		}
	};
}

#endif