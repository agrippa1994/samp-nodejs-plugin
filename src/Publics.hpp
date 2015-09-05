#ifndef PUBLICS_HPP
#define PUBLICS_HPP
#include "detours.h"
#include "jx.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

namespace Publics {
	namespace Internal {
		namespace Stack {
			struct Value {
				std::string stringValue_;
				cell cellValue_ = 0;
				bool isString_ = false;
				bool isCell_ = false;

			public:
				Value(const std::string& stringValue)
					: stringValue_(stringValue), isString_(true) { }

				Value(cell cellValue)
					: cellValue_(cellValue), isCell_(true) { }

				bool isString() const {
					return isString_;
				}

				bool isCell() const {
					return isCell_;
				}

				const std::string& stringValue() const {
					return stringValue_;
				}

				cell cellValue() const {
					return cellValue_;
				}
			};

			class Stack : public std::vector<Value> {
				bool skipPush_ = false;
			public:
				Stack& reversed() {
					std::reverse(begin(), end());
					return *this;
				}

				template<typename T>
				void push(T value) {
					if (skipPush_)
						return;

					push_back(Value(value));
				}

				void clearStack() {
					clear();
					skipPush_ = false;
				}

				void shouldSkipPush(bool skipPush) {
					skipPush_ = skipPush;
				}
			};
		}
	
		// Convert AMX parameters, which are processed by the stack, to the associated JavaScript value
		namespace Converter {
			enum class DataType {
				type_int, type_float, type_string
			};

			template<DataType T>
			struct AMXValueToJXValue { };

			template<>
			struct AMXValueToJXValue< DataType::type_int > {
				static void convert(const Stack::Value& p, JX::ScopedValue& to) {
					if (!p.isCell())
						throw std::exception("Parameter is not a cell!");

					JX_SetInt32(&to, p.cellValue());
				}
			};

			template<>
			struct AMXValueToJXValue< DataType::type_float > {
				static void convert(const Stack::Value& p, JX::ScopedValue& to) {
					if (!p.isCell())
						throw std::exception("Parameter is not a cell!");

					cell val = p.cellValue();
					JX_SetDouble(&to, amx_ctof(val));
				}
			};

			template<>
			struct AMXValueToJXValue< DataType::type_string > {
				static void convert(const Stack::Value& p, JX::ScopedValue& to) {
					if(!p.isString())
						throw std::exception("Parameter is not a string!");

					JX_SetString(&to, p.stringValue().c_str());
				}
			};

			// This class converts an amx value to a js value
			class ValueBase {
			public:
				virtual JXValue value() = 0;

			protected:
				virtual ~ValueBase() { };
			};

			template<DataType dataType>
			class Value : public ValueBase { 
				JX::ScopedValue jxValue_;
			public:
				Value(AMX *amx, const Stack::Value& p) {
					AMXValueToJXValue<dataType>::convert(p, jxValue_);
				}

				JXValue value() {
					return jxValue_;
				}
			};

			class ValueConverter {
				std::istream& is_;
				AMX *amx_;
			public:
				ValueConverter(AMX *amx, std::istream& is)
					: amx_(amx), is_(is) { }

				std::shared_ptr<ValueBase> readFromStream(const Stack::Value &param) {
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
			
			// This class is used to convert values from PAWN to JavaScript (e.g. public calls)
			class AMX2JXCall {
				AMX *amx_;
				std::vector<std::shared_ptr<ValueBase>> jxValues_;

			public:
				AMX2JXCall(AMX *amx, const std::string& format, const Stack::Stack &params) : amx_(amx) {
					std::stringstream ss(format);
					ValueConverter converter(amx_, ss);

					for (auto& i:  params)
						jxValues_.push_back(converter.readFromStream(i));
				}

				bool operator()(JXValue *func, JXValue *output) {
					std::vector<JXValue> params;
					for (auto value: jxValues_)
						params.push_back(value->value());

					return JX_CallFunction(func, params.data(), params.size(), output);
				}
			};
		}

		namespace JavaScriptBinder {
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

			// Store the information for all JavaScript callbacks here
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
		}
		
		// Hooks
		Hook<convention_type::cdecl_t, int, AMX *, cell *, int> execHook;
		Hook<convention_type::cdecl_t, int, AMX *, const char *, int *> findPublicHook;
		Hook<convention_type::cdecl_t, int, AMX *, cell *, cell **, const char *, int, int> pushStringHook;
		Hook<convention_type::cdecl_t, int, AMX *, cell> pushHook;
		std::string currentPublic;
		Stack::Stack stack;
	}

	void init(void **ppData) {
		Internal::findPublicHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_FindPublic], [](AMX *amx, const char *name, int *idx) -> int {
			// FindPublic is used in pawn to find a public function. Here we can fetch the function's name
			Internal::currentPublic = name;
			auto ret = Internal::findPublicHook(amx, name, idx);

			if (ret) {
				*idx = 1000;
				return 0;
			}

			return AMX_ERR_NONE;
		});

		Internal::pushHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_Push], [](AMX *amx, cell value) -> int {
			// Push integer / float value into the stack
			Internal::stack.push(value);

			// Call the original function
			return Internal::pushHook(amx, value);
		});

		Internal::pushStringHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_PushString], [](AMX *amx, cell *amxAddr, cell **physAddr, const char *str, int pack, int useWChar) -> int {
			// Push string value into the stack
			Internal::stack.push(std::string(str));
	
			// amx_PushString calls amx_Push internally and so we've to skip the next push
			Internal::stack.shouldSkipPush(true);
			auto ret = Internal::pushStringHook(amx, amxAddr, physAddr, str, pack, useWChar);
			Internal::stack.shouldSkipPush(false);
				
			// Return the result of our hook
			return ret;
		});
		
		Internal::execHook.apply(((void **)ppData[PLUGIN_DATA_AMX_EXPORTS])[PLUGIN_AMX_EXPORT_Exec], [](AMX *amx, cell *retVal, int idx) -> int {
			// Fetch params, reverse the stack order and clear the current stack
			auto params = Internal::stack.reversed();
			Internal::stack.clearStack();

			if (Internal::currentPublic.empty())
				return idx == 1000 ? 0 : Internal::execHook(amx, retVal, idx);

			std::string functionName = Internal::currentPublic;
			Internal::currentPublic.clear();

			try {
				// Iterate through all binded JavaScript functions and call them
				for (const auto entry : Internal::JavaScriptBinder::publicCallHookEntries) {
					if (std::string(functionName) != entry->name())
						continue;

					// Iterate through all callacks
					for (auto& callback : entry->callbacks()) {
						JX::ScopedValue jxFunctionReturnValue;
						Internal::Converter::AMX2JXCall(amx, entry->format(), params)(&callback, &jxFunctionReturnValue);

						// If the JavaScript function callback return value is an object, then process it
						if (!JX_IsObject(&jxFunctionReturnValue))
							break;
						
						JX::ScopedValue jxSkipPublic, jxSetReturnValueTo;
						JX_GetNamedProperty(&jxFunctionReturnValue, "skipPublic", &jxSkipPublic);
						JX_GetNamedProperty(&jxFunctionReturnValue, "setReturnValueTo", &jxSetReturnValueTo);

						int returnValue = 0;
						bool skipPublic = false;
						// Skip the execution of the public if skipPublic is
						if (JX_IsBoolean(&jxSkipPublic))
							skipPublic = JX_GetBoolean(&jxSkipPublic);

						if (!skipPublic)
							returnValue = idx == 1000 ? 0 : Internal::execHook(amx, retVal, idx);

						if (JX_IsInt32(&jxSetReturnValueTo) && retVal)
							*retVal = JX_GetInt32(&jxSetReturnValueTo);

						return returnValue;
					}
				}
			}
			catch (const std::exception& ) {
				
			}
			catch (...) {
			
			}

			return idx == 1000 ? 0 : Internal::execHook(amx, retVal, idx);
		});
		
	}

	void exit() {
		Internal::execHook.remove();
		Internal::findPublicHook.remove();
		Internal::pushStringHook.remove();
		Internal::pushHook.remove();
	}
}

#endif