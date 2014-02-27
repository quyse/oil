#include "ViewPluginInstance.hpp"
#include "MainPluginInstance.hpp"
#include "ViewScriptObject.hpp"
#include "Engine.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/FrameBuffer.hpp"
#include "../inanity/graphics/MonitorMode.hpp"
#include "../inanity/graphics/RenderBuffer.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/gui/GrCanvas.hpp"
#include "../inanity/gui/Font.hpp"
#include "../inanity/Strings.hpp"
#ifdef ___INANITY_PLATFORM_WINDOWS
#include "../inanity/graphics/Dx11Device.hpp"
#include "../inanity/graphics/Dx11RenderBuffer.hpp"
#include "../inanity/graphics/Dx11Texture.hpp"
#include "../inanity/platform/Win32Window.hpp"
#include "../inanity/input/Win32WmManager.hpp"
#endif

BEGIN_INANITY_OIL

ViewPluginInstance::ViewPluginInstance()
: NpapiPluginInstance(false), renderBufferWidth(0), renderBufferHeight(0)
{
	name = "Inanity Oil NPAPI View Plugin";
	description = name;
	windowless = true;
}

#ifdef ___INANITY_PLATFORM_WINDOWS

void ViewPluginInstance::Paint(HDC hdc)
{
	try
	{
		ptr<Graphics::Device> device = Engine::instance->GetGraphicsDevice();

		// create framebuffer if needed
		if(!frameBuffer)
			frameBuffer = device->CreateFrameBuffer();
		// check if we need to (re)create render buffer
		if(!renderBuffer || renderBufferWidth != width || renderBufferHeight != height)
		{
			// recreate render buffer
			// on Windows use DX11 specific method
			ptr<Graphics::Dx11Device> dx11Device = device.DynamicCast<Graphics::Dx11Device>();
			if(!dx11Device)
				THROW("Only DirectX 11 is supported on Windows now");
			renderBuffer = dx11Device->CreateRenderBufferGdiCompatible(width, height, Graphics::SamplerSettings());

			frameBuffer->SetColorBuffer(0, renderBuffer);

			renderBufferWidth = width;
			renderBufferHeight = height;
		}

		// perform actual drawing
		Draw();

		// get DC of render buffer
		ID3D11ShaderResourceView* resourceView = renderBuffer->GetTexture()
			.FastCast<Graphics::Dx11Texture>()->GetShaderResourceViewInterface();

		ComPointer<ID3D11Resource> resource;
		resourceView->GetResource(&resource);

		ComPointer<IDXGISurface1> surface;
		if(FAILED(resource->QueryInterface(IID_IDXGISurface1, (void**)&surface)))
			THROW("Can't get IDXGISurface1 interface");

		// perform blit
		HDC dxHdc;
		if(FAILED(surface->GetDC(FALSE, &dxHdc)))
			THROW("Can't get render buffer's DC");

		BitBlt(hdc, 0, 0, width, height, dxHdc, 0, 0, SRCCOPY);

		RECT nullRect = { 0, 0, 0, 0 };
		if(FAILED(surface->ReleaseDC(&nullRect)))
			THROW("Can't release DirectX DC");
	}
	catch(Exception* exception)
	{
		std::ostringstream ss;
		MakePointer(exception)->PrintStack(ss);
		std::wstring ws = Strings::UTF82Unicode(ss.str());
		TextOut(hdc, 0, 0, ws.c_str(), ws.length());
	}
}

#endif

void ViewPluginInstance::Draw()
{
	BEGIN_TRY();

	Engine* engine = Engine::instance;
	Graphics::Context* context = engine->GetGraphicsContext();

	Graphics::Context::LetFrameBuffer letFrameBuffer(context, frameBuffer);
	Graphics::Context::LetViewport letViewport(context, width, height);

	END_TRY("Can't draw view");
}

void ViewPluginInstance::PostInit()
{
	scriptObject = MainPluginInstance::instance->GetScriptState()->WrapObject<ViewScriptObject>(NEW(ViewScriptObject(this)));
}

END_INANITY_OIL
