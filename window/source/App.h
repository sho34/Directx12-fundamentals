#pragma once
#ifndef __App__
#define __App__

#include "window/window.h"
#include "renderer.h"

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
	std::unique_ptr<renderer> p_3d_renderer;

private:
	bool initialised = false;

};
#endif // !__App__
