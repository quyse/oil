#ifndef ___INANITY_OIL_TEXTURE_VIEW_RENDERER_HPP___
#define ___INANITY_OIL_TEXTURE_VIEW_RENDERER_HPP___

#include "ViewRenderer.hpp"
#include "../inanity/graphics/SamplerSettings.hpp"

BEGIN_INANITY_GRAPHICS

class Texture;
class SamplerState;
class RawTextureData;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class Engine;

/// View renderer supporting rendering one texture.
class TextureViewRenderer : public ViewRenderer
{
private:
	ptr<Engine> engine;
	ptr<Graphics::RawTextureData> rawTextureData;
	ptr<Graphics::Texture> texture;
	ptr<Graphics::SamplerState> samplerState;
	Graphics::SamplerSettings samplerSettings;
	float scale;
	int mipMode;
	float mipLod;
	float mipBias;

public:
	TextureViewRenderer(ptr<Engine> engine);
	~TextureViewRenderer();

	//*** ViewRenderer's methods.
	void Render();

	void SetTexture(ptr<Graphics::RawTextureData> rawTextureData);
	void SetScale(float scale);
	void SetFilter(int filterType, int filterValue);
	void SetMipMode(int mipMode);
	void SetMipLod(float mipLod);
	void SetMipBias(float mipBias);
};

END_INANITY_OIL

#endif
