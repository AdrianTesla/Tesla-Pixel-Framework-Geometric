#pragma once
#include "TeslaWin.h"
#include <vector>
#include <wrl.h>
#include <string>
#include <dxgidebug.h>

class DxgiInfoManager
{
public:
	DxgiInfoManager();
	~DxgiInfoManager() = default;
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator = (const DxgiInfoManager&) = delete;
	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next = 0ui64;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
};
 