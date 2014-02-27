#include "Engine.hpp"
#include "Painter.hpp"
#include "../inanity/graphics/System.hpp"
#include "../inanity/graphics/Adapter.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/ShaderCache.hpp"
#include "../inanity/graphics/ShaderCompiler.hpp"
#include "../inanity/graphics/shaders/ShaderGenerator.hpp"
#include "../inanity/platform/Game.hpp"
#include "../inanity/gui/GrCanvas.hpp"
#include "../inanity/gui/FtEngine.hpp"
#include "../inanity/gui/FontFace.hpp"
#include "../inanity/gui/FontShape.hpp"
#include "../inanity/gui/FontGlyphs.hpp"
#include "../inanity/gui/Font.hpp"
#include "../inanity/crypto/WhirlpoolStream.hpp"
#include "../inanity/FileSystem.hpp"
#include "../inanity/File.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

Engine* Engine::instance = nullptr;

Engine::Engine(ptr<FileSystem> assetFileSystem, ptr<FileSystem> shaderCacheFileSystem)
{
	instance = this;

	// initialize graphics system
	graphicsSystem = Platform::Game::CreateDefaultGraphicsSystem();
	ptr<Graphics::Adapter> adapter = graphicsSystem->GetAdapters()[0];
	graphicsDevice = graphicsSystem->CreateDevice(adapter);
	graphicsContext = graphicsSystem->CreateContext(graphicsDevice);
	// create shader cache
	shaderCache = NEW(Graphics::ShaderCache(
		shaderCacheFileSystem,
		graphicsDevice,
		graphicsDevice->CreateShaderCompiler(),
		graphicsDevice->CreateShaderGenerator(),
		NEW(Crypto::WhirlpoolStream())));
	painter = NEW(Painter(graphicsDevice, graphicsContext, shaderCache));

	// initialize gui system
	canvas = Gui::GrCanvas::Create(graphicsDevice, shaderCache);
	{
		ptr<Gui::FontEngine> fontEngine = NEW(Gui::FtEngine());
		ptr<Gui::FontFace> fontFace = fontEngine->LoadFontFace(assetFileSystem->LoadFile("/DejaVuSans.ttf"));
		const int fontSize = 13;
		ptr<Gui::FontShape> fontShape = fontFace->CreateShape(fontSize);
		ptr<Gui::FontGlyphs> fontGlyphs = fontFace->CreateGlyphs(canvas, fontSize);
		standardFont = NEW(Gui::Font(fontShape, fontGlyphs));
	}
}

Engine::~Engine()
{
	instance = nullptr;
}

ptr<Graphics::Device> Engine::GetGraphicsDevice() const
{
	return graphicsDevice;
}

ptr<Graphics::Context> Engine::GetGraphicsContext() const
{
	return graphicsContext;
}

ptr<Painter> Engine::GetPainter() const
{
	return painter;
}

ptr<Gui::GrCanvas> Engine::GetCanvas() const
{
	return canvas;
}

ptr<Gui::Font> Engine::GetStandardFont() const
{
	return standardFont;
}

END_INANITY_OIL
