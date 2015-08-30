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
#include <iostream>

namespace AMX2JX {
	namespace Internal {
		enum class DataType {
			type_int, type_float, type_string
		};

		// This class converts an amx value to a js value
		class ValueBase {
		public:
			virtual JXValue value() = 0;

		protected:
			virtual ~ValueBase() { };
		};

		template<DataType dataType>
		class Value : public ValueBase { };


		template<>
		class Value < DataType::type_int > : public ValueBase {
			JX::ScopedValue jxValue_;
		public:
			Value(AMX *amx, cell p) {
				JX_SetInt32(&jxValue_, p);
			}

			JXValue value() {
				return jxValue_;
			}
		};

		template<>
		class Value < DataType::type_float > : public ValueBase {
			JX::ScopedValue jxValue_;
		public:
			Value(AMX *amx, cell p) {
				JX_SetDouble(&jxValue_, amx_ctof(p));
			}

			JXValue value() {
				return jxValue_;
			}
		};

		template<>
		class Value < DataType::type_string > : public ValueBase {
			JX::ScopedValue jxValue_;
			cell *addr_ = 0;
			cell len_ = 0;
			char *text_ = nullptr;
		public:
			Value(AMX *amx, cell p) {
				
				std::cout << "STRING: " << std::endl;

				
		
				amx_StrLen(addr_, &len_);
				std::cout << "STRING: " << std::endl;

				if (!len_)
					return;
				
				text_ = new char[++len_];
				memset(text_, 0, len_);

				amx_GetString(text_, addr_, 0, len_);
				
				JX_SetString(&jxValue_, text_);

			}

			~Value() {
				if (text_) {
					delete[] text_;
				}
			}

			JXValue value() {
				return jxValue_;
			}
		};

		class ValueConverter {
			std::istream& is_;
			AMX *amx_;
		public:
			ValueConverter(AMX *amx, std::istream& is) : amx_(amx), is_(is) {

			}

			std::shared_ptr<ValueBase> readFromStream(cell param) {
				if (!is_.good())
					return nullptr;

				char c = 0;
				is_ >> c;

				switch (c) {
				case 'i':
				case 'd':
					return std::make_shared<Value<DataType::type_int>>(amx_, param);
				case 'f':
					return std::make_shared<Value<DataType::type_float>>(amx_, param);
				case 's':
					return std::make_shared<Value<DataType::type_string>>(amx_, param);
				}
				return nullptr;
			}

		};
	}

	// This class is used to convert values from PAWN to JavaScript (e.g. public calls)
	class AMX2JX {
		AMX *amx_;
		std::vector<std::shared_ptr<Internal::ValueBase>> jxValues_;

	public:
		AMX2JX(AMX *amx, const std::string& format, cell *params) : amx_(amx) {
			std::stringstream ss(format);
			Internal::ValueConverter converter(amx_, ss);

			for (size_t i = 1; i < (params[0] / sizeof(cell)) + 1; i++) {
				jxValues_.push_back(converter.readFromStream(params[i]));
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