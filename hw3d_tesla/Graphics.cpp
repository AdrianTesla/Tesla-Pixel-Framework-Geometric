#include "Graphics.h"
#include "dxerr.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Graphics::InfoException(__LINE__, __FILE__, v);}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

Graphics::Graphics(HWND hWnd)
	:
	pBuffer(ScreenWidth, ScreenHeight),
	msr({})
{	
	// The graphics is initialized filling the pDevice, pContext and pSwapChain pointers.
	// First we configure the Swap Chain descriptor, passing also the handle to the window
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferDesc.Width                   = ScreenWidth  * PixelSize;
	swapDesc.BufferDesc.Height                  = ScreenHeight * PixelSize;
	swapDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator   = 0u;
	swapDesc.BufferDesc.RefreshRate.Denominator = 0u;
	swapDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.SampleDesc.Count                   = 1u;
	swapDesc.SampleDesc.Quality                 = 0u;
	swapDesc.Windowed                           = TRUE;
	swapDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;	
	swapDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.BufferCount                        = 1u;
	swapDesc.OutputWindow                       = hWnd;
	swapDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// We create the Device and the SwapChain on the DEBUG LAYER if we are in DEBUG for 
	// the crucial error information diagnostics
	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Error codes are stored in this type
	HRESULT hr;

	/******************** D3D11 DEVICE AND SWAPCHAIN *********************/
	// And we fill the pipi: this is the official birth of the D3D11 Device
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE, // (use D3D_DRIVER_TYPE_WARP for software implementation)
		nullptr,
		swapCreateFlags,
		nullptr,
		0u,
		D3D11_SDK_VERSION,
		&swapDesc,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pContext
	));

	// In order to clear the back buffer (a texture subresource), we need a view on that (pTargetView will be filled)
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTargetView));

	// Give ImGui the pDevice and the pContext pointers to allow it to draw on the screen
	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());

	/**************************************************************/
	/******************* SETTING THE PIPELINE *********************/

	// The screen consists of a textured quad on which the pBuffer texture is applied
	struct Vertex
	{
		float x;
		float y;
		float u;
		float v;
	};

	// Textured quad: two triangles that cover the screen, with proper texture coordinates
	static constexpr Vertex vertices[] =
	{
		{-1.0f, 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 0.0f },
		{-1.0f,-1.0f, 0.0f, 1.0f },

		{-1.0f,-1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 0.0f },
		{ 1.0f,-1.0f, 1.0f, 1.0f },
	};

	/*****************************************************/
	/***************** RESOURCE CREATION *****************/

	/*****************************************************/
	/************** Create the Vertex Buffer *************/
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd   = {};
	bd.Usage               = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags      = 0u;
	bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth           = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);
	bd.MiscFlags           = 0u;
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem                = vertices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));
	UINT strides = sizeof(Vertex);
	UINT offsets = 0u;
	
	/**********************************************************************/
	/************** Create the Vertex Shader and Pixel Shader *************/

	// We compile the shaders at runtime with D3DCompile so the final .exe doesn't need the shaders binary dependencies

	// Vertex Shader HLSL (High Level Shader Language) code
	static const char* vertexShader =
"\
struct VSOut\
{\
	float2 tc : TexCoord;\
	float4 pos : SV_Position;\
};\
\
VSOut main(float2 pos : Position, float2 tc : TexCoord)\
{\
	VSOut v;\
	\
	v.pos = float4(pos.x, pos.y, 0.0f, 1.0f);\
	v.tc = tc;\
	\
	return v;\
};\
";

	// Pixel Shader HLSL (High Level Shader Language) code
	static const char* pixelShader = 
"\
Texture2D tex : register(t0);\
SamplerState splr;\
\
float4 main(float2 tc : TexCoord) : SV_Target\
{\
	return tex.Sample(splr, tc);\
}\
";

	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_THROW_INFO(D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr, nullptr, "main", "ps_4_0", 0u, 0u, &pBlob, nullptr));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr, nullptr, "main", "vs_4_0", 0u, 0u, &pBlob, nullptr));
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	/****************************************************/
	/************** Create the Input Layout *************/
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
		{ "TexCoord", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
	};
	GFX_THROW_INFO(pDevice->CreateInputLayout(ied, (UINT)std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));
	
	/*************************************************/
	/************** Create the View Port *************/
	D3D11_VIEWPORT vp = {};
	vp.Width          = ScreenWidth * PixelSize;
	vp.Height         = ScreenHeight * PixelSize;
	vp.TopLeftX       = 0.0f;
	vp.TopLeftY       = 0.0f;
	vp.MaxDepth       = 1.0f;
	vp.MinDepth       = 0.0f;

	/*********************************************************/
	/***************** FRAMEBUFFER TEXTURE *******************/
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.MipLevels            = 1u;
	texDesc.ArraySize            = 1u;
	texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Width                = ScreenWidth;
	texDesc.Height               = ScreenHeight;
	texDesc.Usage                = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags       = 0u;
	texDesc.SampleDesc.Count     = 1u;
	texDesc.SampleDesc.Quality   = 0u;
	texDesc.MiscFlags            = 0u;
	D3D11_SUBRESOURCE_DATA srd   = {};
	srd.pSysMem                  = pBuffer.GetBufferPtrConst();
	srd.SysMemPitch              = ScreenWidth * sizeof(Color);
	GFX_THROW_INFO(pDevice->CreateTexture2D(&texDesc, &srd, &pTexture));

	// Creation of the view on the texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                          = texDesc.Format;
	srvDesc.Texture2D.MipLevels             = 1u;
	srvDesc.Texture2D.MostDetailedMip       = 0u;
	srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
	GFX_THROW_INFO(pDevice->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTextureView));
	/*********************************************************/

	/*********************************************************/
	/**************** Create the Sampler State ***************/
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
	D3D11_SAMPLER_DESC samDesc = {};
	samDesc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW           = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
	samDesc.Filter             = D3D11_FILTER_MIN_MAG_MIP_POINT;
	GFX_THROW_INFO(pDevice->CreateSamplerState(&samDesc, &pSamplerState));
	/*********************************************************/

	/*******************************************************************************************/
	/******** Bind all the created resources and configurations to the Graphics Pipeline *******/
	GFX_THROW_INFO_ONLY(pContext->OMSetRenderTargets(1u, pTargetView.GetAddressOf(), nullptr));
	GFX_THROW_INFO_ONLY(pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &strides, &offsets));
	GFX_THROW_INFO_ONLY(pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u));
	GFX_THROW_INFO_ONLY(pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u));
	GFX_THROW_INFO_ONLY(pContext->IASetInputLayout(pInputLayout.Get()));
	GFX_THROW_INFO_ONLY(pContext->RSSetViewports(1u, &vp));
	GFX_THROW_INFO_ONLY(pContext->PSSetShaderResources(0u, 1u, pTextureView.GetAddressOf()));
	GFX_THROW_INFO_ONLY(pContext->PSSetSamplers(0u, 1u, pSamplerState.GetAddressOf()));
	GFX_THROW_INFO_ONLY(pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	/*******************************************************************************************/
}

void Graphics::UpdateFrameStatistics() noexcept
{
	const auto ImGuiIO    = ImGui::GetIO();
	const auto frame_rate = ImGuiIO.Framerate;
	std::stringstream ss;
	ss.precision(3);
	ss << std::fixed << 1000.0f / frame_rate;
	ss.precision(0);
	ss << std::fixed << " ms/frame (" << frame_rate << " FPS)] (" << ScreenWidth << "x" << ScreenHeight << ")";
	statsInfo = ss.str();
}

std::string Graphics::GetFrameStatistics() const noexcept
{
	return statsInfo;
}

Graphics::~Graphics()
{
	ImGui_ImplDX11_Shutdown();
}

void Graphics::BeginFrame(bool clear, Color clearColor)
{
	if (clear)
	{
		Clear(clearColor);
	}
	// We always do an ImGui NewFrame because of the useful framerate counter 
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Graphics::EndFrame() 
{
	HRESULT hr;

	UpdateFrameStatistics();

	// Update the framebuffer stored in the GPU memory with our color pBuffer 
	GFX_THROW_INFO_ONLY(pContext->UpdateSubresource(pTexture.Get(), 0u, nullptr, pBuffer.GetBufferPtrConst(), (UINT)pBuffer.GetRowPitch(), 0u));

	// Draw the CPU Frame Buffer
	GFX_THROW_INFO_ONLY(pContext->Draw(6u, 0u));

	// Render ImGui data on the screen only if it's enabled
	ImGui::Render();
	if (imGuiEnabled)
	{
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

#ifndef NDEBUG
	infoManager.Set();
#endif
	if (FAILED(hr = pSwapChain->Present(syncInterval, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(hr);
		}
		else
		{
			throw GFX_EXCEPT(hr);
		}
	}
}

void Graphics::Clear(Color c) noexcept
{
	pBuffer.Clear(c);
}

void Graphics::EnableVSync() noexcept
{
	syncInterval = 1u;
}

void Graphics::DisableVSync() noexcept
{
	syncInterval = 0u;
}

void Graphics::SetVSyncInterval(const UINT verticalSyncInterval) noexcept
{
	syncInterval = verticalSyncInterval;
}

bool Graphics::IsVSyncEnabled() const noexcept
{
	return syncInterval != 0u;
}

void Graphics::EnableImGui() noexcept
{
	imGuiEnabled = true;
}

void Graphics::DisableImGui() noexcept
{
	imGuiEnabled = false;
}

bool Graphics::IsImGuiEnabled() const noexcept
{
	return imGuiEnabled;
}

Color* Graphics::GetFramebufferPtr() const noexcept
{
	return pBuffer.GetBufferPtr();
}

const Color* Graphics::GetFramebufferPtrConst() const noexcept
{
	return pBuffer.GetBufferPtrConst();
}

void Graphics::PutPixel(int x, int y, Color c)
{
	pBuffer.PutPixel(x, y, c);
}

void Graphics::PutPixel(const Tesla::Vec2& p, Color c)
{
	PutPixel((int)p.x, (int)p.y, c);
}

void Graphics::PutPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	PutPixel(x, y, Color(r, g, b));
}

void Graphics::DrawLine(float x0, float y0, float x1, float y1, Color c, bool clip)
{
	auto Draw = [&](float x0, float y0, float x1, float y1)
	{
		const float side = std::max(std::abs(y1 - y0), std::abs(x1 - x0));
		const int nPixels = (int)side;
		const float sideInv = 1.0f / side;
		const float dx = (x1 - x0) * sideInv;
		const float dy = (y1 - y0) * sideInv;

		// Prestep
		float x = x0 + 0.5f;
		float y = y0 + 0.5f;
		for (int i = 0; i <= nPixels; i++)
		{
			PutPixel((int)x, (int)y, c);
			x += dx;
			y += dy;
		}
	};

	if (clip)
	{
		// Clip rectangle (off by -1, lol)
		static constexpr float xmin = -1.0f;
		static constexpr float ymin = -1.0f;
		static constexpr float xmax = (float)Graphics::ScreenWidth  - 1.0f;
		static constexpr float ymax = (float)Graphics::ScreenHeight - 1.0f;

		static constexpr int INSIDE = 0;  // 0000
		static constexpr int LEFT   = 1;  // 0001
		static constexpr int RIGHT  = 2;  // 0010
		static constexpr int BOTTOM = 4;  // 0100
		static constexpr int TOP    = 8;  // 1000

		typedef int OutCode;

		auto ComputeOutCode = [&](float x, float y)
		{
			OutCode code;

			code = INSIDE;          // initialised as being inside of [[clip window]]

			if (x < xmin)           // to the left of clip window
				code |= LEFT;
			else if (x > xmax)      // to the right of clip window
				code |= RIGHT;
			if (y < ymin)           // below the clip window
				code |= BOTTOM;
			else if (y > ymax)      // above the clip window
				code |= TOP;

			return code;
		};

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		OutCode outcode0 = ComputeOutCode(x0, y0);
		OutCode outcode1 = ComputeOutCode(x1, y1);
		bool accept = false;

		while (true)
		{
			if (!(outcode0 | outcode1))
			{
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			}
			else if (outcode0 & outcode1)
			{
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			}
			else
			{
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				float x = 0.0f, y = 0.0f;

				// At least one endpoint is outside the clip rectangle; pick it.
				OutCode outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & TOP)  // point is above the clip window
				{
					x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
					y = ymax;
				}
				else if (outcodeOut & BOTTOM)  // point is below the clip window
				{
					x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
					y = ymin;
				}
				else if (outcodeOut & RIGHT)  // point is to the right of clip window
				{
					y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
					x = xmax;
				}
				else if (outcodeOut & LEFT)  // point is to the left of clip window
				{
					y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
					x = xmin;
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0)
				{
					x0 = x;
					y0 = y;
					outcode0 = ComputeOutCode(x0, y0);
				}
				else
				{
					x1 = x;
					y1 = y;
					outcode1 = ComputeOutCode(x1, y1);
				}
			}
		}

		if (accept)
		{
			// Finally draw the clipped line with AdrianTesla's algorithm
			Draw(x0, y0, x1, y1);
		}
	}
	else
	{
		Draw(x0, y0, x1, y1);
	}
}

void Graphics::DrawLine(float x0, float y0, float x1, float y1, Color c0, Color c1, bool clip)
{
	using namespace Tesla;
	typedef unsigned char uc;

	auto Draw = [&](float x0, float y0, float x1, float y1, const Vec3& vc0, const Vec3 vc1)
	{
		const float side = std::max(std::abs(y1 - y0), std::abs(x1 - x0));
		const int nPixels = (int)side;
		const float sideInv = 1.0f / side;
		const float dx = (x1 - x0) * sideInv;
		const float dy = (y1 - y0) * sideInv;
		const Vec3 dvc = (vc1 - vc0) * sideInv;

		Vec3 vc = vc0;

		// Prestep
		float x = x0 + 0.5f;
		float y = y0 + 0.5f;
		for (int i = 0; i <= nPixels; i++)
		{
			PutPixel((int)x, (int)y, Color((uc)vc.x, (uc)vc.y, (uc)vc.z));
			x += dx;
			y += dy;
			vc += dvc;
		}
	};

	Vec3 vc0 = Vec3((uc)c0.GetR(), (uc)c0.GetG(), (uc)c0.GetB());
	Vec3 vc1 = Vec3((uc)c1.GetR(), (uc)c1.GetG(), (uc)c1.GetB());

	if (clip)
	{
		// Clip rectangle (off by -1, lol)
		static constexpr float xmin = -1.0f;
		static constexpr float ymin = -1.0f;
		static constexpr float xmax = (float)Graphics::ScreenWidth - 1.0f;
		static constexpr float ymax = (float)Graphics::ScreenHeight - 1.0f;

		static constexpr int INSIDE = 0;  // 0000
		static constexpr int LEFT   = 1;  // 0001
		static constexpr int RIGHT  = 2;  // 0010
		static constexpr int BOTTOM = 4;  // 0100
		static constexpr int TOP    = 8;  // 1000

		typedef int OutCode;

		auto ComputeOutCode = [&](float x, float y)
		{
			OutCode code;

			code = INSIDE;          // initialised as being inside of [[clip window]]

			if (x < xmin)           // to the left of clip window
				code |= LEFT;
			else if (x > xmax)      // to the right of clip window
				code |= RIGHT;
			if (y < ymin)           // below the clip window
				code |= BOTTOM;
			else if (y > ymax)      // above the clip window
				code |= TOP;

			return code;
		};

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		OutCode outcode0 = ComputeOutCode(x0, y0);
		OutCode outcode1 = ComputeOutCode(x1, y1);
		bool accept = false;

		while (true)
		{
			if (!(outcode0 | outcode1))
			{
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			}
			else if (outcode0 & outcode1)
			{
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			}
			else
			{
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				float x = 0.0f, y = 0.0f;
				Vec3 vc_clip = { 0.0f,0.0f,0.0f };

				// At least one endpoint is outside the clip rectangle; pick it.
				OutCode outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & TOP)  // point is above the clip window
				{
					const float t = (ymax - y0) / (y1 - y0);

					x = x0 + (x1 - x0) * t;
					vc_clip = vc0 + (vc1 - vc0) * t;
					y = ymax;
				}
				else if (outcodeOut & BOTTOM)  // point is below the clip window
				{
					const float t = (ymin - y0) / (y1 - y0);

					x = x0 + (x1 - x0) * t;
					vc_clip = vc0 + (vc1 - vc0) * t;
					y = ymin;
				}
				else if (outcodeOut & RIGHT)  // point is to the right of clip window
				{
					const float t = (xmax - x0) / (x1 - x0);
					y = y0 + (y1 - y0) * t;
					vc_clip = vc0 + (vc1 - vc0) * t;
					x = xmax;
				}
				else if (outcodeOut & LEFT)  // point is to the left of clip window
				{
					const float t = (xmin - x0) / (x1 - x0);
					y = y0 + (y1 - y0) * t;
					vc_clip = vc0 + (vc1 - vc0) * t;
					x = xmin;
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0)
				{
					x0 = x;
					y0 = y;
					vc0 = vc_clip;
					outcode0 = ComputeOutCode(x0, y0);
				}
				else
				{
					x1 = x;
					y1 = y;
					vc1 = vc_clip;
					outcode1 = ComputeOutCode(x1, y1);
				}
			}
		}

		if (accept)
		{
			// Finally draw the clipped line with AdrianTesla's algorithm
			Draw(x0, y0, x1, y1, vc0, vc1);
		}
	}
	else
	{
		Draw(x0, y0, x1, y1, vc0, vc1);
	}
}

void Graphics::DrawLine(const Tesla::Vec2& p0, const Tesla::Vec2 p1, Color c, bool clip)
{
	DrawLine(p0.x, p0.y, p1.x, p1.y, c, clip);
}

void Graphics::DrawLine(const Tesla::Vec2& p0, const Tesla::Vec2 p1, Color c0, Color c1, bool clip)
{
	DrawLine(p0.x, p0.y, p1.x, p1.y, c0, c1, clip);
}

void Graphics::DrawRect(float left, float right, float top, float bottom, Color c)
{
	assert(left <= right && "Left of rect should be less than Right of rect");
	assert(top <= bottom && "Top of rect should be less than Bottom of rect");
	DrawLine(left , top   , right, top   , c);
	DrawLine(right, top   , right, bottom, c);
	DrawLine(right, bottom, left , bottom, c);
	DrawLine(left , bottom, left , top   , c);
}

void Graphics::DrawRect(const Tesla::Vec2& topLeft, float width, float height, Color c)
{
	DrawRectDim(topLeft.x, topLeft.y, width, height, c);
}

void Graphics::DrawRectDim(float topLeftX, float topLeftY, float width, float height, Color c)
{
	DrawRect(topLeftX, topLeftX + width, topLeftY, topLeftY + height, c);
}

void Graphics::FillRect(float left, float right, float top, float bottom, Color c)
{
	const int xStart = std::max((int)left, 0);
	const int xEnd   = std::min((int)right, (int)ScreenWidth - 1);
	const int yStart = std::max((int)top, 0);
	const int yEnd   = std::min((int)bottom, (int)ScreenHeight - 1);

	for (int y = yStart; y <= yEnd; y++)
	{
		for (int x = xStart; x <= xEnd; x++)
		{
			PutPixel(x, y, c);
		}
	}
}

void Graphics::FillRect(const Tesla::Vec2& topLeft, float width, float height, Color c)
{
	FillRectDim(topLeft.x, topLeft.y, width, height, c);
}

void Graphics::FillRectDim(float topLeftX, float topLeftY, float width, float height, Color c)
{
	FillRect(topLeftX, topLeftX + width, topLeftY, topLeftY + height, c);
}

void Graphics::DrawRegularPolygon(float x, float y, int nSides, float radius, Color c, float rotationRad)
{
	assert(nSides > 1 && "What is a regular polygon with less than 2 sides?");
	using namespace Tesla;
	const float phiStep = twoPI / float(nSides);
	float phi = phiStep - rotationRad;

	auto CalculatePosition = [&](float phi)
	{
		return Vec2(x + radius * std::cosf(phi), y + radius * std::sinf(phi));
	};

	Vec2 cur = CalculatePosition(-rotationRad);
	for (int i = 0; i < nSides; i++)
	{
		const Vec2 next = CalculatePosition(phi);
		DrawLine(cur, next, c, true);
		cur = next;
		phi += phiStep;
	}
}

void Graphics::DrawPolyline(const std::vector<Tesla::Vec2>& points, Color c, bool clip)
{
	if (points.size() > 1)
	{
		for (auto i = points.cbegin(), end = std::prev(points.end()); i < end; i++)
		{
			DrawLine(*i, *std::next(i), c, clip);
		}
	}
}

void Graphics::DrawPolyline(const std::vector<Tesla::Vec2>& points, Color c0, Color c1, bool clip)
{
	if (points.size() > 1)
	{
		using namespace Tesla;

		if (points.size() == 2)
		{
			DrawLine(points[0], points[1], c0, c1, clip);
		}
		else
		{
			const Vec3 vc0 = Vec3(float(c0.GetR()), float(c0.GetG()), float(c0.GetB()));
			const Vec3 vc1 = Vec3(float(c1.GetR()), float(c1.GetG()), float(c1.GetB()));

			auto col = [](const Vec3& vc)
			{
				typedef unsigned char uc;
				return Color((uc)vc.x, (uc)vc.y, (uc)vc.z);
			};

			const Vec3 dvc = (vc1 - vc0) / float(points.size() - 1);

			Vec3 vc_cur = vc0;
			Vec3 vc_next = vc0 + dvc;
			for (auto i = points.cbegin(), end = std::prev(points.end()); i < end; i++)
			{
				DrawLine(*i, *std::next(i), col(vc_cur), col(vc_next), clip);

				vc_cur = vc_next;
				vc_next += dvc;
			}
		}
	}
}

void Graphics::DrawClosedPolyline(const std::vector<Tesla::Vec2>& points, Color c, bool clip)
{
	if (points.size() > 1)
	{
		DrawPolyline(points, c, clip);
		DrawLine(*std::prev(points.end()), *points.begin(), c, clip);
	}
}

void Graphics::DrawCircle(float xc, float yc, float radius, Color c)
{
	DrawEllipse(xc, yc, radius, radius, c);
}

void Graphics::DrawCircle(const Tesla::Vec2& center, float radius, Color c)
{
	DrawCircle(center.x, center.y, radius, c);
}

void Graphics::FillCircle(float xc, float yc, float radius, Color c)
{
	FillEllipse(xc, yc, radius, radius, c);
}

void Graphics::FillCircle(const Tesla::Vec2& center, float radius, Color c)
{
	FillCircle(center.x, center.y, radius, c);
}

void Graphics::DrawEllipse(float xc, float yc, float ra, float rb, Color c)
{
	using namespace Tesla;

	static constexpr int nSides = 100;
	const float phiStep = twoPI / float(nSides);
	float phi = phiStep;

	auto CalculatePosition = [&](float phi)
	{
		return Vec2(xc + ra * std::cosf(phi), yc + rb * std::sinf(phi));
	};

	Vec2 cur = CalculatePosition(0.0f);
	for (int i = 0; i < nSides; i++)
	{
		const Vec2 next = CalculatePosition(phi);
		DrawLine(cur, next, c, true);
		cur = next;
		phi += phiStep;
	}
}

void Graphics::FillEllipse(float xc, float yc, float ra, float rb, Color c)
{
	using namespace Tesla;
	const int yStart = std::max((int)(yc - rb + 0.5f), 0);
	const int yEnd   = std::min((int)(yc + rb + 0.5f), int(ScreenHeight - 1));
	const float raSq = sq(ra);
	const float rbSqInv = 1.0f / sq(rb);

	for (int y = yStart; y <= yEnd; y++)
	{
		const float arg = 1.0f - rbSqInv * sq(static_cast<float>(y) - yc + 0.5f);
		if (arg >= 0)
		{
			const float x_displacement = ra * std::sqrtf(arg);

			const int xStart = std::max(int(xc - x_displacement + 0.5f), 0);
			const int xEnd   = std::min(int(xc + x_displacement + 0.5f), int(ScreenWidth - 1));

			for (int x = xStart; x <= xEnd; x++)
			{
				PutPixel(x, y, c);
			}
		}
	}
}

void Graphics::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c, bool clip)
{
	DrawTriangle({ x0,y0 }, { x1,y1 }, { x1,x2 }, c, clip);
}

void Graphics::DrawTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c, bool clip)
{
	DrawLine(v0, v1, c, clip);
	DrawLine(v1, v2, c, clip);
	DrawLine(v2, v0, c, clip);
}

void Graphics::FillTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c)
{
	// Power
	using namespace Tesla; 

	// AABB - Aligned Axis Bounding Box, clipped
	const int xStart = std::max(static_cast<int>(std::min({ v0.x,v1.x,v2.x })), 0);
	const int yStart = std::max(static_cast<int>(std::min({ v0.y,v1.y,v2.y })), 0);
	const int xEnd   = std::min(static_cast<int>(std::max({ v0.x,v1.x,v2.x })), static_cast<int>(ScreenWidth) - 1);
	const int yEnd   = std::min(static_cast<int>(std::max({ v0.y,v1.y,v2.y })), static_cast<int>(ScreenHeight) - 1);

	// Normalized side vectors (also accounts for CW/CCW triangle vertices)
	const float areaInv = 1.0f / Vec2::Cross(v0 - v1, v2 - v1);
	const Vec2 s01 = (v1 - v0) * areaInv;
	const Vec2 s12 = (v2 - v1) * areaInv;
	const Vec2 s20 = (v0 - v2) * areaInv;
	
	// Starting point
	const Vec2 p = { float(xStart)+0.5f,float(yStart)+0.5f };

	// First row barycentric coordinates
	float w0_row = Vec2::Cross(p - v1, s12);
	float w1_row = Vec2::Cross(p - v2, s20);
	float w2_row = Vec2::Cross(p - v0, s01);

	// y-loop
	for (int y = yStart; y <= yEnd; y++)
	{
		// Barycentric coordinates at the start of the row
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;

		// x-loop
		for (int x = xStart; x <= xEnd; x++)
		{
			// Only draw pixels with positive Barycentric coordinates (inside triangle)
			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				// Here, (w0, w1, w2) are the normalized barycentric coordinates 
				// of the triangle, limited to the [0, 1] interval
				PutPixel(x, y, c);
			}
			// Update barycentric coordinates one pixel to the right
			w0 += s12.y;
			w1 += s20.y;
			w2 += s01.y;
		}
		// Update barycentric coordinates one pixel down
		w0_row -= s12.x;
		w1_row -= s20.x;
		w2_row -= s01.x;
	}
}

void Graphics::FillTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c, bool clip)
{
	FillTriangle({ x0,y0 }, { x1,y1 }, { x2,y2 }, c);
}

void Graphics::FillTriangle(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, Color c0, Color c1, Color c2)
{
	using namespace Tesla;

	// AABB - Aligned Axis Bounding Box, clipped
	const int xStart = std::max(static_cast<int>(std::min({ v0.x,v1.x,v2.x })), 0);
	const int yStart = std::max(static_cast<int>(std::min({ v0.y,v1.y,v2.y })), 0);
	const int xEnd   = std::min(static_cast<int>(std::max({ v0.x,v1.x,v2.x })), static_cast<int>(ScreenWidth) - 1);
	const int yEnd   = std::min(static_cast<int>(std::max({ v0.y,v1.y,v2.y })), static_cast<int>(ScreenHeight) - 1);

	// Normalized side vectors (also accounts for CW/CCW triangle vertices)
	const float areaInv = 1.0f / Vec2::Cross(v0 - v1, v2 - v1);
	const Vec2 s01 = (v1 - v0) * areaInv;
	const Vec2 s12 = (v2 - v1) * areaInv;
	const Vec2 s20 = (v0 - v2) * areaInv;

	// Starting point
	const Vec2 p = { float(xStart) + 0.5f,float(yStart) + 0.5f };

	// First row barycentric coordinates
	float w0_row = Vec2::Cross(p - v1, s12);
	float w1_row = Vec2::Cross(p - v2, s20);
	float w2_row = Vec2::Cross(p - v0, s01);

	// Color to Vec3 for easy interpolation
	const Vec3 vc0 = { float(c0.GetR()),float(c0.GetG()),float(c0.GetB()) };
	const Vec3 vc1 = { float(c1.GetR()),float(c1.GetG()),float(c1.GetB()) };
	const Vec3 vc2 = { float(c2.GetR()),float(c2.GetG()),float(c2.GetB()) };

	// The color at the start, 
	// and the linear change amount per pixel step (horizontal and vertical)
	Vec3 c_row = vc0 * w0_row + vc1 * w1_row + vc2 * w2_row;
	Vec3 dcx   = vc0 * s12.x  + vc1 * s20.x  + vc2 * s01.x;
	Vec3 dcy   = vc0 * s12.y  + vc1 * s20.y  + vc2 * s01.y;

	// y-loop
	for (int y = yStart; y <= yEnd; y++)
	{
		// Barycentric coordiantes at the start of the row
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;

		// Color at the start of the row
		Vec3   c = c_row;

		// x-loop
		for (int x = xStart; x <= xEnd; x++)
		{
			// Only draw pixels with positive Barycentric coordinates (inside triangle)
			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				PutPixel(x, y, Color((unsigned char)c.x, (unsigned char)c.y, (unsigned char)c.z));
			}
			// Update barycentric coordinates and color for one step to the right
			w0 += s12.y;
			w1 += s20.y;
			w2 += s01.y;
			c  += dcy;
		}
		// Update barycentric coordinates at the start of the row and color for one step down
		w0_row -= s12.x;
		w1_row -= s20.x;
		w2_row -= s01.x;
		c_row  -= dcx;
	}
}

void Graphics::FillTriangle(const Tesla::Vec2& v0, Color c0, const Tesla::Vec2& v1, Color c1, const Tesla::Vec2& v2, Color c2)
{
	FillTriangle(v0, v1, v2, c0, c1, c2);
}

void Graphics::FillTriangleTex(const Tesla::Vec2& v0, const Tesla::Vec2& v1, const Tesla::Vec2& v2, const Tesla::Vec2& uv0, const Tesla::Vec2& uv1, const Tesla::Vec2& uv2, const Surface& tex)
{
	using namespace Tesla;

	// AABB - Aligned Axis Bounding Box, clipped
	const int xStart = std::max(static_cast<int>(std::min({ v0.x,v1.x,v2.x })), 0);
	const int yStart = std::max(static_cast<int>(std::min({ v0.y,v1.y,v2.y })), 0);
	const int xEnd   = std::min(static_cast<int>(std::max({ v0.x,v1.x,v2.x })), static_cast<int>(ScreenWidth) - 1);
	const int yEnd   = std::min(static_cast<int>(std::max({ v0.y,v1.y,v2.y })), static_cast<int>(ScreenHeight) - 1);

	// Normalized side vectors (also accounts for CW/CCW triangle vertices)
	const float areaInv = 1.0f / Vec2::Cross(v0 - v1, v2 - v1);
	const Vec2 s01 = (v1 - v0) * areaInv;
	const Vec2 s12 = (v2 - v1) * areaInv;
	const Vec2 s20 = (v0 - v2) * areaInv;

	// Starting point
	const Vec2 p = { float(xStart) + 0.5f,float(yStart) + 0.5f };

	// First row barycentric coordinates
	float w0_row = Vec2::Cross(p - v1, s12);
	float w1_row = Vec2::Cross(p - v2, s20);
	float w2_row = Vec2::Cross(p - v0, s01);

	// The uv coords at the start, 
	// and the linear change amount per pixel step (horizontal and vertical)
	Vec2 uv_row = uv0 * w0_row + uv1 * w1_row + uv2 * w2_row;
	Vec2 duvx   = uv0 * s12.x  + uv1 * s20.x  + uv2 * s01.x;
	Vec2 duvy   = uv0 * s12.y  + uv1 * s20.y  + uv2 * s01.y;

	// y-loop
	for (int y = yStart; y <= yEnd; y++)
	{
		// Barycentric coordiantes at the start of the row
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;

		// uv-coords at the start of the row
		Vec2  uv = uv_row;

		// x-loop
		for (int x = xStart; x <= xEnd; x++)
		{
			// Only draw pixels with positive Barycentric coordinates (inside triangle)
			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				PutPixel(x, y, tex.Sample(uv.x, uv.y));
			}
			// Update barycentric coordinates and uv for one step to the right
			w0 += s12.y;
			w1 += s20.y;
			w2 += s01.y;
			uv += duvy;
		}
		// Update barycentric coordinates at the start of the row and uv for one step to the right
		w0_row -= s12.x;
		w1_row -= s20.x;
		w2_row -= s01.x;
		uv_row -= duvx;
	}
}

void Graphics::DrawBezierCurve(const Tesla::Vec2& p0, const Tesla::Vec2& p0ctrl, const Tesla::Vec2& p1ctrl, const Tesla::Vec2& p1, Color c)
{
	// Barozzi rules. Period.
	// 1 = ((1 - t) + t)^3 =
	// 1 = (1 - t)^3 + 3(1 - t)^2 t + 3(1 - t) t^2 + t^3
	// 1 = b0(t) + b1(t) + b2(t) + b3(t)
	// b0(t), b1(t), b2(t), b3(t) sono i polinomi di Bernstein
	// Una parametrizzazione della curva di Bezier è data da
	// p(t) = b0(t) * p0 + b1(t) * p0ctrl + b2(t) * p1ctrl + b3(t) * p1;

	using namespace Tesla;

	// How many?
	const int nPoints = 100;
	float t = 0.0f;
	const float dt = 1.0f / float(nPoints);

	// Not fully optimized, but sexy as hell
	Vec2 cur = p0;
	for (int i = 0; i <= nPoints; i++)
	{
		// Bernstein coefficients
		const float b0 = cube(1.0f - t);
		const float b1 = 3.0f * sq(1.0f - t) * t;
		const float b2 = 3.0f * (1.0f - t) * sq(t);
		const float b3 = cube(t);

		// Interpolate the points with the Bernstein coefficients
		const Vec2 next = p0 * b0 + p0ctrl * b1 + p1ctrl * b2 + p1 * b3;

		DrawLine(cur, next, c);
		cur = next;
		t += dt;
	}
}

void Graphics::DrawBezierCurve(const Tesla::Vec2& p0, const Tesla::Vec2& p0ctrl, const Tesla::Vec2& p1ctrl, const Tesla::Vec2& p1, Color c0, Color c1)
{
	using namespace Tesla;

	// How many?
	const int nPoints = 100;

	// Vec3 colors for easy interpolation
	const Vec3 vc0 = { (float)c0.GetR(),(float)c0.GetG(),(float)c0.GetB() };
	const Vec3 vc1 = { (float)c1.GetR(),(float)c1.GetG(),(float)c1.GetB() };
	
	const float dt = 1.0f / float(nPoints);
	const Vec3 dvc = (vc1 - vc0) * dt;

	float t = 0.0f;
	Vec3 vc = vc0;

	// Not fully optimized, but sexy as hell
	Vec2 cur = p0;
	for (int i = 0; i <= nPoints; i++)
	{
		// Bernstain coefficients
		const float b0 = cube(1.0f - t);
		const float b1 = 3.0f * sq(1.0f - t) * t;
		const float b2 = 3.0f * (1.0f - t) * sq(t);
		const float b3 = cube(t);

		// Interpolate the points with the Bernstain coefficients
		const Vec2 next = p0 * b0 + p0ctrl * b1 + p1ctrl * b2 + p1 * b3;

		DrawLine(cur, next, Color((unsigned char)vc.x, (unsigned char)vc.y, (unsigned char)vc.z));
		cur = next;
		t += dt;
		vc += dvc;
	}
}

Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	// Join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// Remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Tesla Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Tesla Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.append("\n\n");
	}
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Tesla Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}