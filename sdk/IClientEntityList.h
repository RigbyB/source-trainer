#pragma once

class IClientEntity;

class IClientEntityList {
public:
	IClientEntity* GetClientEntity(int entnum) {
		typedef IClientEntity* (__thiscall* original_function)(void*, int);
		return reinterpret_cast<original_function>((*reinterpret_cast<void***>(this))[3])(this, entnum);
	}
};