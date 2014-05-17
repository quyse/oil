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

TextureViewRenderer::TextureViewRenderer(ptr<Engine> engine)
: engine(engine), scale(1), mipMode(0), mipLod(0), mipBias(0)
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

		textureQuad->uTextureScale.SetValue(vec2(
			viewportWidth / ((float)rawTextureData->GetMipWidth(0) * scale),
			viewportHeight / ((float)rawTextureData->GetMipHeight(0) * scale)
			));
		textureQuad->uBackgroundScale.SetValue(vec2(viewportWidth * 0.1f, viewportHeight * 0.1f));
		textureQuad->uOffset.SetValue(vec2(0, 0));
		textureQuad->uLod.SetValue(mipLod);
		textureQuad->uBias.SetValue(mipBias);

		Painter::TextureQuad::Let tql(context, textureQuad, texture, samplerState, (Painter::TextureQuad::Let::MipMode)mipMode);

		context->Draw();
	}
}

void TextureViewRenderer::SetTexture(ptr<RawTextureData> rawTextureData)
{
	this->rawTextureData = rawTextureData;
	this->texture = engine->GetGraphicsDevice()->CreateStaticTexture(rawTextureData, SamplerSettings());
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

END_INANITY_OIL
