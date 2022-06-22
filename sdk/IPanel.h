#pragma once

class IPanel {
public:
	const char* GetName(unsigned int vguiPanel) {
		typedef const char* (__thiscall* original_function)(void*, unsigned int);
		return reinterpret_cast<original_function>((*reinterpret_cast<void***>(this))[36])(this, vguiPanel);
	}
};