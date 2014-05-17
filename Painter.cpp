#include "Painter.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/ShaderCache.hpp"
#include "../inanity/graphics/VertexLayout.hpp"
#include "../inanity/graphics/VertexLayoutElement.hpp"
#include "../inanity/graphics/AttributeLayout.hpp"
#include "../inanity/graphics/AttributeLayoutSlot.hpp"
#include "../inanity/graphics/AttributeBinding.hpp"
#include "../inanity/graphics/Texture.hpp"
#include "../inanity/graphics/SamplerState.hpp"
#include "../inanity/graphics/UniformBuffer.hpp"
#include "../inanity/graphics/VertexBuffer.hpp"
#include "../inanity/graphics/IndexBuffer.hpp"
#include "../inanity/graphics/VertexShader.hpp"
#include "../inanity/graphics/PixelShader.hpp"
#include "../inanity/graphics/shaders/UniformGroup.hpp"
#include "../inanity/graphics/shaders/Uniform.ipp"
#include "../inanity/graphics/shaders/Sampler.ipp"
#include "../inanity/graphics/shaders/Temp.ipp"
#include "../inanity/graphics/shaders/LValue.ipp"
#include "../inanity/graphics/shaders/functions.hpp"
#include "../inanity/MemoryFile.hpp"

BEGIN_INANITY_OIL

using namespace Inanity::Graphics;
using namespace Inanity::Graphics::Shaders;

//*** class Painter

Painter::Painter(ptr<Device> device, ptr<Context> context, ptr<ShaderCache> shaderCache)
: device(device), context(context), shaderCache(shaderCache)
{
	quad = NEW(Quad(device, shaderCache));
	textureQuad = NEW(TextureQuad(device, shaderCache, quad));
}

Painter::~Painter() {}

//*** class Painter::Quad

Painter::Quad::Vertex Painter::Quad::vertices[4] =
{
	{ vec4(-1, -1, 0, 1), vec2(0, 1) },
	{ vec4(1, -1, 0, 1), vec2(1, 1) },
	{ vec4(1, 1, 0, 1), vec2(1, 0) },
	{ vec4(-1, 1, 0, 1), vec2(0, 0) }
};
unsigned short Painter::Quad::indices[6] = { 0, 2, 1, 0, 3, 2 };

Painter::Quad::Quad(ptr<Device> device, ptr<ShaderCache> shaderCache) :
	// vertex & attribute layouts
	vl(NEW(VertexLayout(sizeof(Vertex)))),
	al(NEW(AttributeLayout())),
	als(al->AddSlot()),
	aPosition(al->AddElement(als, vl->AddElement(&Vertex::position))),
	aTexcoord(al->AddElement(als, vl->AddElement(&Vertex::texcoord))),
	// interpolants
	iTexcoord(0),
	// geometry
	vb(device->CreateStaticVertexBuffer(MemoryFile::CreateViaCopy(vertices, sizeof(vertices)), vl)),
	ib(device->CreateStaticIndexBuffer(MemoryFile::CreateViaCopy(indices, sizeof(indices)), sizeof(indices[0]))),
	// attribute binding
	ab(device->CreateAttributeBinding(al))
{
	// vertex shader
	vs = shaderCache->GetVertexShader((
		setPosition(aPosition),
		iTexcoord = screenToTexture(aPosition["xy"])
	));
}

Painter::Quad::Let::Let(Context* context, const Quad* quad) :
	lab(context, quad->ab),
	lvb(context, 0, quad->vb),
	lib(context, quad->ib),
	lvs(context, quad->vs)
{}

//*** class Painter::TextureQuad

Painter::TextureQuad::TextureQuad(ptr<Device> device, ptr<ShaderCache> shaderCache, ptr<Quad> quad) :
	quad(quad),
	ug(NEW(UniformGroup(0))),
	uTextureScale(ug->AddUniform<vec2>()),
	uBackgroundScale(ug->AddUniform<vec2>()),
	uOffset(ug->AddUniform<vec2>()),
	uLod(ug->AddUniform<float>()),
	uBias(ug->AddUniform<float>()),
	s(0)
{
	ug->Finalize(device);

	// pixel shader
	Temp<vec2> texcoord;
	Temp<vec4> sample;
	Temp<vec2> backgroundTexcoord;
	Temp<float> background;

	Expression head = (
		texcoord = quad->iTexcoord * uTextureScale + uOffset
	);

	Expression tail = (
		backgroundTexcoord = quad->iTexcoord * uBackgroundScale,
		background = mod(floor(backgroundTexcoord["x"]) + floor(backgroundTexcoord["y"]), 2.0f) * Value<float>(0.5f) + Value<float>(0.5f),
		fragment(0, newvec4(lerp(newvec3(background, background, background), sample["xyz"], sample["w"]), 1.0f))
	);

	psMipAuto = shaderCache->GetPixelShader((
		head,
		sample = s.Sample(texcoord),
		tail
	));
	psMipLod = shaderCache->GetPixelShader((
		head,
		sample = s.SampleLod(texcoord, uLod),
		tail
	));
	psMipBias = shaderCache->GetPixelShader((
		head,
		sample = s.SampleBias(texcoord, uBias),
		tail
	));
}

Painter::TextureQuad::~TextureQuad() {}

Painter::TextureQuad::Let::Let(Context* context, const TextureQuad* textureQuad, ptr<Texture> t, ptr<SamplerState> ss, MipMode mipMode) :
	lq(context, textureQuad->quad),
	lub(context, textureQuad->ug),
	ls(context, textureQuad->s, t, ss),
	lps(context, GetPixelShader(textureQuad, mipMode))
{
	textureQuad->ug->Upload(context);
}

Painter::TextureQuad::Let::~Let() {}

ptr<PixelShader> Painter::TextureQuad::Let::GetPixelShader(const TextureQuad* textureQuad, MipMode mipMode)
{
	switch(mipMode)
	{
	case mipModeAuto:
		return textureQuad->psMipAuto;
	case mipModeLod:
		return textureQuad->psMipLod;
	case mipModeBias:
		return textureQuad->psMipBias;
	default:
		THROW("Wrong lod mode");
	}
}

END_INANITY_OIL
