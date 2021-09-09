#pragma once
#include "TeslaWin.h"
#include "TeslaException.h"
#include "Tesla.h"
#include "DxgiInfoManager.h"
#include "Surface.h"
#include <d3d11.h>
#include <wrl.h>
#include <sstream>
#include <algorithm>

class Graphics
{
public:
	// Graphics exception handling
	class Exception : public TeslaException
	{
		using TeslaException::TeslaException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs = {}) noexcept;
		virtual const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator = (const Graphics&) = delete;
	~Graphics();
public:
	void BeginFrame(bool clear = true, Color clearColor = Color::Black);
	void EndFrame();
	void Clear(Color fillColor) noexcept;
	void EnableVSync() noexcept;
	void DisableVSync() noexcept;
	void SetVSyncInterval(const UINT verticalSyncInterval) noexcept;
	bool IsVSyncEnabled() const noexcept;
	void EnableImGui() noexcept;
	void DisableImGui() noexcept;
	bool IsImGuiEnabled() const noexcept;
	Color* GetFramebufferPtr() const noexcept;
	const Color* GetFramebufferPtrConst() const noexcept;
	void PutPixel(int x, int y, Color c);
	void PutPixel(const Tesla::Vec2& p, Color c);
	void PutPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void DrawLine(float x0, float y0, float x1, float y1, Color c, bool clip = true);
	void DrawLine(float x0, float y0, float x1, float y1, Color c0, Color c1, bool clip = true);
	void DrawLine(const Tesla::Vec2& p0, const Tesla::Vec2 p1, Color c, bool clip = true);
	void DrawLine(const Tesla::Vec2& p0, const Tesla::Vec2 p1, Color c0, Color c1, bool clip = true);
	void DrawRect(float left, float right, float top, float bottom, Color c);
	void DrawRect(const Tesla::Vec2& topLeft, float width, float height, Color c);
	void DrawRectDim(float topLeftX, float topLeftY, float width, float height, Color c);
	void FillRect(float left, float right, float top, float bottom, Color c);
	void FillRect(const Tesla::Vec2& topLeft, float width, float height, Color c);
	void FillRectDim(float topLeftX, float topLeftY, float width, float height, Color c);
	void DrawRegularPolygon(float x, float y, int nSides, float radius, Color c, float rotationRad = 0.0f);
	void DrawPolyline(const std::vector<Tesla::Vec2>& points, Color c, bool clip = true);
	void DrawPolyline(const std::vector<Tesla::Vec2>& points, Color cStart, Color cEnd, bool clip = true);
	void DrawClosedPolyline(const std::vector<Tesla::Vec2>& points, Color c, bool clip = true);
	void DrawCircle(float xc, float yc, float radius, Color c);
	void DrawCircle(const Tesla::Vec2& center, float radius, Color c);
	void FillCircle(float xc, float yc, float radius, Color c);
	void FillCircle(const Tesla::Vec2& center, float radius, Color c);
	void DrawEllipse(float xc, float yc, float ra, float rb, Color c);
	void FillEllipse(float xc, float yc, float ra, float rb, Color c);
	void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c, bool clip = true);
	void DrawTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c, bool clip = true);
	void FillTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c);
	void FillTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c, bool clip = true);
	void FillTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c0, Color c1, Color c2);
	void FillTriangle(const Tesla::Vec2& v0, Color c0, const Tesla::Vec2& v1, Color c1, const Tesla::Vec2& v2, Color c2);
	void FillTriangleTex(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, const Tesla::Vec2& uv0, const Tesla::Vec2& uv1, const Tesla::Vec2& uv2, const Surface& tex);
	void DrawBezierCurve(const Tesla::Vec2& p0, const Tesla::Vec2& p0ctrl, const Tesla::Vec2& p1ctrl, const Tesla::Vec2& p1, Color c);
	void DrawBezierCurve(const Tesla::Vec2& p0, const Tesla::Vec2& p0ctrl, const Tesla::Vec2& p1ctrl, const Tesla::Vec2& p1, Color c0, Color c1);
	std::string GetFrameStatistics() const noexcept;
private:
	void UpdateFrameStatistics() noexcept;
private:
	bool imGuiEnabled = true;
	UINT syncInterval = 1u;
	std::string statsInfo;
	std::string title = "Adrian Tesla DirectX Framework";
private:
	Microsoft::WRL::ComPtr<ID3D11Device>           pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>    pContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>         pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>        pTexture;
	D3D11_MAPPED_SUBRESOURCE msr;
private:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
private:
	Surface pBuffer;
public:
	static constexpr unsigned int PixelSize    = 1u;
	static constexpr unsigned int ScreenWidth  = 800u;
	static constexpr unsigned int ScreenHeight = 600u;
	static constexpr float ScreenWidthF  = static_cast<float>(ScreenWidth);
	static constexpr float ScreenHeightF = static_cast<float>(ScreenHeight);
	static constexpr unsigned int ScreenCenterX = ScreenWidth  / 2u;
	static constexpr unsigned int ScreenCenterY = ScreenHeight / 2u;
	static constexpr float ScreenCenterXf = static_cast<float>(ScreenWidthF  / 2.0f);
	static constexpr float ScreenCenterYf = static_cast<float>(ScreenHeightF / 2.0f);
};