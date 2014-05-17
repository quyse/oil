#ifndef ___INANITY_OIL_ENGINE_HPP___
#define ___INANITY_OIL_ENGINE_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"
#include "../inanity/gui/gui.hpp"

BEGIN_INANITY

class File;
class FileSystem;

END_INANITY

BEGIN_INANITY_GRAPHICS

class System;
class Device;
class Context;
class ShaderCache;
class RawTextureData;
class ImageLoader;

END_INANITY_GRAPHICS

BEGIN_INANITY_GUI

class GrCanvas;
class Font;

END_INANITY_GUI

BEGIN_INANITY_OIL

class Painter;

class Engine : public Object
{
public:
	static Engine* instance;

private:
	ptr<Graphics::System> graphicsSystem;
	ptr<Graphics::Device> graphicsDevice;
	ptr<Graphics::Context> graphicsContext;
	ptr<Graphics::ShaderCache> shaderCache;
	ptr<Painter> painter;

	ptr<Gui::GrCanvas> canvas;
	ptr<Gui::Font> standardFont;

	ptr<Graphics::ImageLoader> imageLoader;

public:
	Engine(ptr<FileSystem> assetFileSystem, ptr<FileSystem> shaderCacheFileSystem);
	~Engine();

	ptr<Graphics::Device> GetGraphicsDevice() const;
	ptr<Graphics::Context> GetGraphicsContext() const;
	ptr<Painter> GetPainter() const;
	ptr<Gui::GrCanvas> GetCanvas() const;
	ptr<Gui::Font> GetStandardFont() const;

	ptr<Graphics::RawTextureData> LoadRawTexture(ptr<File> file);
};

END_INANITY_OIL

#endif
