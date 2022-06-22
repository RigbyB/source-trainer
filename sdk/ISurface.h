#pragma once

class ISurface {
public:
	void DrawSetColor(int r, int g, int b, int a) {
		typedef void(__thiscall* original_function)(void*, int, int, int, int);
		reinterpret_cast<original_function>((*reinterpret_cast<void***>(this))[11])(this, r, g, b, a);
	}
	void DrawFilledRect(int x1, int y1, int x2, int y2) {
		typedef void(__thiscall* original_function)(void*, int, int, int, int);
		reinterpret_cast<original_function>((*reinterpret_cast<void***>(this))[12])(this, x1, y1, x2, y2);
	}
};