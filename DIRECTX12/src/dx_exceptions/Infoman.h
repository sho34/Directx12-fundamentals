#pragma once
// get the error description from the debug output
#ifndef __GFXExceptions__
#define __GFXExceptions__

#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
#include <dxgidebug.h>

class InfoManager
{
public:
	InfoManager();
	~InfoManager() = default;
	InfoManager(const InfoManager&) = delete;
	InfoManager& operator=(const InfoManager&) = delete;

	void Set() noexcept;
	std::vector<std::string> GetMessages() const;

private:
	unsigned long long next = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
};

#endif