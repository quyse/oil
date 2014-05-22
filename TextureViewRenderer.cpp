#include "TextureViewRenderer.hpp"
#include "Engine.hpp"
#include "Painter.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/FrameBuffer.hpp"
#include "../inanity/graphics/Texture.hpp"
#include "../inanity/graphics/SamplerState.hpp"
#include "../inanity/graphics/RawTextureData.hpp"
#include "../inanity/graphics/shaders/Uniform.ipp"

BEGIN_INANITY_OIL

using namespace Inanity::Graphics;

TextureViewRenderer::TextureViewRenderer(ptr<Engine> engine) :
	engine(engine), offset(0, 0), scale(1), mipMode(0), mipLod(0), mipBias(0),
	colorTransform(Graphics::identity_mat<float, 4>()), colorOffset(0, 0, 0, 0)
{
}

TextureViewRenderer::~TextureViewRenderer() {}

void TextureViewRenderer::Render()
{
	ptr<Context> context = engine->GetGraphicsContext();

	context->ClearColor(0, vec4(0, 1, 1, 1));

	if(rawTextureData)
	{
		if(!samplerState)
			samplerState = engine->GetGraphicsDevice()->CreateSamplerState(samplerSettings);

		ptr<Painter::TextureQuad> textureQuad = engine->GetPainter()->textureQuad;

		float viewportWidth = (float)context->GetViewportWidth();
		float viewportHeight = (float)context->GetViewportHeight();
		float imageWidth = (float)rawTextureData->GetImageWidth();
		float imageHeight = (float)rawTextureData->GetImageHeight();
		float scaleXCoef = 1.0f / (imageWidth * scale);
		float scaleYCoef = 1.0f / (imageHeight * scale);

		textureQuad->uTextureScale.SetValue(vec2(viewportWidth * scaleXCoef, viewportHeight * scaleYCoef));
		textureQuad->uBackgroundScale.SetValue(vec2(viewportWidth * 0.1f, viewportHeight * 0.1f));
		textureQuad->uOffset.SetValue(vec2(-offset.x * scaleXCoef, -offset.y * scaleYCoef));
		textureQuad->uLod.SetValue(mipLod);
		textureQuad->uBias.SetValue(mipBias);
		textureQuad->uColorTransform.SetValue(colorTransform);
		textureQuad->uColorOffset.SetValue(colorOffset);

		Painter::TextureQuad::Let tql(context, textureQuad, texture, samplerState, (Painter::TextureQuad::Let::MipMode)mipMode);

		context->Draw();
	}
}

void TextureViewRenderer::SetTexture(ptr<RawTextureData> rawTextureData)
{
	this->rawTextureData = rawTextureData;
	this->texture = engine->GetGraphicsDevice()->CreateStaticTexture(rawTextureData, SamplerSettings());
}

void TextureViewRenderer::SetOffset(const vec2& offset)
{
	this->offset = offset;
}

void TextureViewRenderer::SetScale(float scale)
{
	this->scale = scale;
}

void TextureViewRenderer::SetFilter(int filterType, int filterValue)
{
	SamplerSettings::Filter filter = (SamplerSettings::Filter)filterValue;
	switch(filterType)
	{
	case 0:
		samplerSettings.minFilter = filter;
		break;
	case 1:
		samplerSettings.mipFilter = filter;
		break;
	case 2:
		samplerSettings.magFilter = filter;
		break;
	}
	samplerState = nullptr;
}

void TextureViewRenderer::SetTile(bool tile)
{
	samplerSettings.SetWrap(tile ? SamplerSettings::wrapRepeat : SamplerSettings::wrapBorder);
	samplerState = nullptr;
}

void TextureViewRenderer::SetMipMode(int mipMode)
{
	this->mipMode = mipMode;
}

void TextureViewRenderer::SetMipLod(float mipLod)
{
	this->mipLod = mipLod;
}

void TextureViewRenderer::SetMipBias(float mipBias)
{
	this->mipBias = mipBias;
}

void TextureViewRenderer::SetColorTransform(const Graphics::mat4x4& colorTransform)
{
	this->colorTransform = colorTransform;
}

void TextureViewRenderer::SetColorOffset(const Graphics::vec4& colorOffset)
{
	this->colorOffset = colorOffset;
}

END_INANITY_OIL
