#include "ViewPluginInstance.hpp"
#include "MainPluginInstance.hpp"
#include "Engine.hpp"
#include "ViewScriptObject.hpp"
#include "ViewRenderer.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/FrameBuffer.hpp"
#include "../inanity/graphics/Presenter.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/gui/GrCanvas.hpp"
#include "../inanity/gui/Font.hpp"
#include "../inanity/Strings.hpp"

BEGIN_INANITY_OIL

ViewPluginInstance::ViewPluginInstance()
: NpapiPluginInstance(false)
{
	name = "Inanity Oil NPAPI View Plugin";
	description = name;
	windowless = true;
}

ptr<Graphics::Device> ViewPluginInstance::GetGraphicsDevice() const
{
	return Engine::instance->GetGraphicsDevice();
}

void ViewPluginInstance::Draw()
{
	BEGIN_TRY();

	Engine* engine = Engine::instance;
	Graphics::Context* context = engine->GetGraphicsContext();

	Graphics::Context::LetFrameBuffer letFrameBuffer(context, GetPresenter()->GetFrameBuffer());
	Graphics::Context::LetViewport letViewport(context, width, height);

	if(viewRenderer)
		viewRenderer->Render();

	// draw frame time
	{
		float frameTime = ticker.Tick();

		Gui::GrCanvas* canvas = engine->GetCanvas();
		canvas->SetContext(context);
		Gui::Font* font = engine->GetStandardFont();

		std::ostringstream ss;
		ss << "FPS: " << (1.0f / frameTime);

		font->DrawString(canvas, ss.str(), Graphics::vec2(10.0f, height - 10.0f), Graphics::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		canvas->Flush();
	}

	END_TRY("Can't draw view");
}

void ViewPluginInstance::PostInit()
{
	viewScriptObject = NEW(ViewScriptObject(this));
	scriptObject = MainPluginInstance::instance->GetScriptState()->WrapObject<ViewScriptObject>(viewScriptObject);
}

void ViewPluginInstance::SetViewRenderer(ptr<ViewRenderer> viewRenderer)
{
	this->viewRenderer = viewRenderer;
}

END_INANITY_OIL
