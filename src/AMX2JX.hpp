#ifndef AMX2JX_HPP
#define AMX2JX_HPP
#include "jx.hpp"
#include <jxcore/jx.h>
#include <jxcore/jx_result.h>
#include <sampgdk/interop.h>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

namespace amx2jx {

	enum class amxDataType {
		type_int, type_float
	};

	// This class converts an amx value to a js value

	class amx2jsValueBase {
	public:
		virtual JXValue value() = 0;

	protected:
		virtual ~amx2jsValueBase() { };
	};

	template<amxDataType dataType>
	class amx2jsValue: public amx2jsValueBase { };


	template<>
	class amx2jsValue < amxDataType::type_int > : public amx2jsValueBase {
		JX::ScopedValue jxValue_;
	public:
		amx2jsValue(cell p) {
			JX_SetInt32(&jxValue_, p);
		}

		JXValue value() {
			return jxValue_;
		}
	};

	template<>
	class amx2jsValue < amxDataType::type_float > : public amx2jsValueBase {
		JX::ScopedValue jxValue_;
	public:
		amx2jsValue(cell p) {
			JX_SetDouble(&jxValue_, amx_ctof(p));
		}

		JXValue value() {
			return jxValue_;
		}
	};
	
	class amx2jxValueParser {
		std::istream& is_;
	public:
		amx2jxValueParser(std::istream& is) : is_(is) {

		}

		std::shared_ptr<amx2jsValueBase> readFromStream(cell param) {
			if (!is_.good())
				return nullptr;
			
			char c = 0;
			is_ >> c;

			switch (c) {
			case 'i':
			case 'd':
				return std::make_shared<amx2jsValue<amxDataType::type_int>>(param);
			case 'f':
				return std::make_shared<amx2jsValue<amxDataType::type_float>>(param);
			}
			return nullptr;
		}
		
	};

	// This class is used to convert values from PAWN to JavaScript (e.g. public calls)
	class amx2jx {
		AMX *amx_;
		std::vector<std::shared_ptr<amx2jsValueBase>> jxValues_;

	public:
		amx2jx(AMX *amx, const std::string& format, cell *params) : amx_(amx) {
			std::stringstream ss(format);
			amx2jxValueParser parser(ss);

			for (size_t i = 1; i < (params[0] / sizeof(cell)) + 1; i++) {
				jxValues_.push_back(parser.readFromStream(params[i]));
			}
		}

		bool operator()(JXValue *func, JXValue *firstParams, size_t numberOfFirstParams, JXValue *output) {
			std::vector<JXValue> params;

			for (size_t i = 0; i < numberOfFirstParams; i++)
				params.push_back(firstParams[i]);

			for (auto value : jxValues_)
				params.push_back(value->value());

			return JX_CallFunction(func, params.data(), params.size(), output);
		}

		bool operator()(JXValue *func, JXValue *output) {
			return (*this)(func, nullptr, 0, output);
		}
	};
}

#endif