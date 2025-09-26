#include "App.h"
#include <sstream>


App::App()
	:wnd(700, 700, L"Blaze Engine")
{
	try
	{
		p_3d_renderer = std::make_unique<renderer>(wnd.GetHwnd(), wnd.GetWidth(), wnd.GetHeight());
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
		p_3d_renderer->activate_v_sync_parameters();
		p_3d_renderer->draw_frame(1.0f, { wnd.mouse.GetPosX(), wnd.mouse.GetPosY() }, NULL);
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
			p_3d_renderer->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;
		default:
			break;
		}

		// keyboard press keys
		if(wnd.kbd.KeyIsPressed('V'))
		{
			p_3d_renderer->toggle_v_sync();
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
			p_3d_renderer->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;

		case WM_SIZE:
			p_3d_renderer->resize_buffers(
				static_cast<uint32_t>(wnd.GetWidth()), static_cast<uint32_t>(wnd.GetHeight())
			);
			break;

		case WM_KEYDOWN:
			p_3d_renderer->m_p_camera->on_key_down(static_cast<UINT8>(m.w_param));
			break;

		case WM_KEYUP:
			p_3d_renderer->m_p_camera->on_key_up(static_cast<UINT8>(m.w_param));

		default:
			break;
		}
	}
}


