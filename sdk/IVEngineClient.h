#pragma once

class IVEngineClient {
public:
	int GetLocalPlayer(void) {
		typedef int (__thiscall* original_function)(void*);
		return reinterpret_cast<original_function>((*reinterpret_cast<void***>(this))[12])(this);
	}
};