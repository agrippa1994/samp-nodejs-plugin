#pragma once
#include <subhook/subhook.h>
#include <type_traits>
#include <iostream>

enum class convention_type{
	stdcall_t, cdecl_t, thiscall_t
};

template<convention_type tp, typename retn, typename convention, typename ...args> struct convention;

template<typename retn, typename ...args>
struct convention < convention_type::stdcall_t, retn, args... > {
	typedef retn(__stdcall *type)(args...);
};

template<typename retn, typename ...args>
struct convention < convention_type::cdecl_t, retn, args... > {
	typedef retn(__cdecl *type)(args...);
};

template<typename retn, typename ...args>
struct convention < convention_type::thiscall_t, retn, args... > {
	typedef retn(__fastcall *type)(void *, args...);
};


template<convention_type tp, typename retn, typename... args> class Hook
{
	typedef typename convention<tp, retn, args...>::type type;

	SubHook hook_;

public:
	Hook() { }

	template<typename T>
	Hook(T pFunc, type detour) {
		apply<T>(pFunc, detour);
	}

	template<typename T>
	void apply(T pFunc, type detour) {
		hook_.Install((void *)pFunc, (void *)detour);
	}

	bool remove() {
		return hook_.Remove();
	}

	retn operator()(args... p)
	{
		if (!hook_.IsInstalled())
			throw std::exception("Hook is not installed!");

		if (hook_.GetTrampoline() == 0)
			throw std::exception("Trampoline is null!");

		std::cout << "is null: " << (hook_.GetTrampoline() == 0) << std::endl;
		return ((type)(hook_.GetTrampoline()))(p...);
	}

	const bool isApplied() const {
		return hook_.IsInstalled();
	}
};