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
	Graphics::vec2 offset;
	float scale;
	int mipMode;
	float mipLod;
	float mipBias;
	Graphics::mat4x4 colorTransform;
	Graphics::vec4 colorOffset;

public:
	TextureViewRenderer(ptr<Engine> engine);
	~TextureViewRenderer();

	//*** ViewRenderer's methods.
	void Render();

	void SetTexture(ptr<Graphics::RawTextureData> rawTextureData);
	void SetScale(float scale);
	void SetOffset(const Graphics::vec2& offset);
	void SetFilter(int filterType, int filterValue);
	void SetTile(bool tile);
	void SetMipMode(int mipMode);
	void SetMipLod(float mipLod);
	void SetMipBias(float mipBias);
	void SetColorTransform(const Graphics::mat4x4& colorTransform);
	void SetColorOffset(const Graphics::vec4& colorOffset);
};

END_INANITY_OIL

#endif
