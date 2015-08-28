#ifndef JX_HPP
#define JX_HPP

#include "../JXCore/jx.h"
#include "../JXCore/jx_result.h"

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
	};
}


#endif