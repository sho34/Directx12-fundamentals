#pragma once
#ifndef __windows_messages__
#define __windows_messages__

// get minimal windows to reduce compile time.
#define WIN32_LEAN_AND_MEAN
// to store the windows messages.
#include <queue>
// for thread safety 
#include <mutex>
#include <windows.h>

struct windows_messages
{
	UINT msg;
	WPARAM w_param;
	LPARAM l_param;
};

#endif