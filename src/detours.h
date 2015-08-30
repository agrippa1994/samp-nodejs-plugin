#pragma once
#pragma comment(lib, "detours.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <detours.h>

#include <type_traits>


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


template<convention_type tp, typename retn, typename ...args> class Hook
{
	typedef typename convention<tp, retn, args...>::type type;

	type _orig;
	type _detour;

	bool _isApplied;
	/*	
	void *DetourFunction(BYTE *source, const BYTE *destination, const int length)
	{
		Disasm(NULL);
		// source - original function.
		// destination - new function.
		// length - length of bytes to patch.

		BYTE *jmp = (BYTE *)malloc(length + 5);
		DWORD dwBack;

		VirtualProtect(source, length, PAGE_EXECUTE_READWRITE, &dwBack);

		memcpy(jmp, source, length);
		jmp += length;

		jmp[0] = 0xE9;
		*(DWORD *)(jmp + 1) = (DWORD)(source + length - jmp) - 5;

		source[0] = 0xE9;
		*(DWORD*)(source + 1) = (DWORD)(destination - source) - 5;

		for (int i = 5; i < length; i++)
		{
			source[i] = 0x90;
		}

		VirtualProtect(source, length, dwBack, &dwBack);

		return (jmp - length);
	}

	*/

public:
	Hook() : _isApplied(false), _orig(0), _detour(0) { }

	template<typename T>
	Hook(T pFunc, type detour) {
		apply<T>(pFunc, detour);
	}

	~Hook(){
		remove();
	}

	template<typename T>
	void apply(T pFunc, type detour)
	{
		_detour = detour;
		_orig = (type)DetourFunction((PBYTE)pFunc, (PBYTE)_detour);
		_isApplied = true;
	}

	bool remove()
	{
		if (!_isApplied)
			return false;

		_isApplied = false;
		return false;
		return DetourRemove((PBYTE)_orig, (PBYTE)_detour) > 0;
	}

	retn callOrig(args... p)
	{
		return _orig(p...);
	}

	const bool isApplied() const {
		return _isApplied;
	}
};