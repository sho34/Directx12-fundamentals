#include "App.h"
#include <sstream>


App::App()
	:wnd(700, 700, L"Blaze Engine")
{
	try
	{
		p_teapot_graphics = std::make_unique<teapot_render>(wnd.GetHwnd(), wnd.GetWidth(), wnd.GetHeight());
	}
	catch (const DxException& e)
	{
		
		MessageBox(
			wnd.GetHwnd(), e.what().c_str(), L"DIRECTX EXCEPTION", MB_OK | MB_ICONEXCLAMATION
		);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(
			wnd.GetHwnd(), e.what(), "STANDARD EXCEPTION", MB_OK | MB_ICONEXCLAMATION
		);
	}
	catch (...)
	{
		MessageBoxA(wnd.GetHwnd(), "no details available", "Unknown exception", MB_OK | MB_ICONEXCLAMATION);
	}
	
}

App::~App()
{
	// release all resources being used.
}

int App::Run()
{

	while (true)
	{
		// process all pending messages, but do not block
		if (const auto ecode = Window::ProcessMessages())
		{
			// if optional has a value 
			return *ecode;
		}

		handle_windows_messages();
		handle_kbd_messages();
		render_frames();
		
	}
}


void App::render_frames() const
{
	if (!wnd.appPaused)
	{
		// I need to fix the issue with imgui that prevents me from dragging the imgui window outside the render
		// window when using appPaused.
		p_teapot_graphics->get_mouse_pos({ wnd.mouse.GetPosX(), wnd.mouse.GetPosY() });
		p_teapot_graphics->activate_v_sync_parameters();
		p_teapot_graphics->update_fps();
		p_teapot_graphics->render();
		//p_teapot_graphics->imgui_test_render();
	}
	else
	{
		::Sleep(100);
	}

}

void App::handle_kbd_messages()
{
	while (const auto e = wnd.kbd.ReadKey())
	{
		if (!e->IsPress())
		{
			continue;
		}

		switch (e->GetCode())
		{
		case VK_F11:
			p_teapot_graphics->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;
		default:
			break;
		}

		// keyboard press keys
		if(wnd.kbd.KeyIsPressed('V'))
		{
			p_teapot_graphics->toggle_v_sync();
		}
	}
}

void App::handle_windows_messages()
{
	std::lock_guard<std::mutex> lock(wnd.queue_mutex);

	while (!wnd.message_queue.empty())
	{
		windows_messages m = wnd.message_queue.front();
		wnd.message_queue.pop();

		//p_teapot_graphics->handle_imgui_messages(wnd.GetHwnd(), m.msg, m.w_param, m.l_param);

		switch (m.msg)
		{
		case WM_CREATE:
			// resize the back buffers at window creation.
			p_teapot_graphics->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;

		case WM_SIZE:
			// when resizing the window also.
			p_teapot_graphics->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;
		default:
			break;
		}
	}
}


