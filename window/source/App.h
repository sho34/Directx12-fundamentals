#pragma once
#ifndef __App__
#define __App__

#include "window/window.h"
#include "teapot_render.h"

class App
{
public:
	App();
	~App();
	int Run();

private:
	// 
	void render_frames() const;
	void handle_kbd_messages();
	void handle_windows_messages();

private:
	Window wnd;
	std::unique_ptr<teapot_render> p_teapot_graphics;

private:
	bool initialised = false;

};
#endif // !__App__
