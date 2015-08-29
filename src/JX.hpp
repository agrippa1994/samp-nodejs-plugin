#ifndef JX_HPP
#define JX_HPP

#include <jxcore/jx.h>
#include <jxcore/jx_result.h>
#include <string>

namespace JX {
	// This class can be used for parameters in JS function calls or evaluations.
	// At end of scope the JXValue is freed by the destructor (RAII)
	class ScopedValue : public JXValue {
	public:
		ScopedValue() {
			JX_New(this);
		}

		~ScopedValue() {
			JX_Free(this);
		}

		std::string stringValue() {
			if (!JX_IsString(this))
				return{};

			char *data = JX_GetString(this);
			std::string ret = data;
			delete[] data;

			return ret;
		}
	};

}


#endif