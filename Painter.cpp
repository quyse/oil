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
#include "../inanity/graphics/shaders/UniformArray.ipp"
#include "../inanity/graphics/shaders/Sampler.ipp"
#include "../inanity/graphics/shaders/Interpolant.ipp"
#include "../inanity/graphics/shaders/Instancer.hpp"
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
	modelRender = NEW(ModelRender(this));
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
		iTexcoord.Set(aPosition["xy"] * newvec2(0.5f, -0.5f) + newvec2(0.5f, 0.5f))
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
	uColorTransform(ug->AddUniform<mat4x4>()),
	uColorOffset(ug->AddUniform<vec4>()),
	s(0)
{
	ug->Finalize(device);

	// pixel shader
	Value<vec2> texcoord = quad->iTexcoord * uTextureScale + uOffset;
	Value<vec2> backgroundTexcoord = quad->iTexcoord * uBackgroundScale;
	Value<float> background = mod(floor(backgroundTexcoord["x"]) + floor(backgroundTexcoord["y"]), 2.0f) * val(0.5f) + val(0.5f);

	auto tail = [this, &background] (Value<vec4> sample) -> Expression
	{
		sample = mul(uColorTransform, sample) + uColorOffset;
		return fragment(0, newvec4(lerp(newvec3(background, background, background), sample["xyz"], sample["w"]), 1.0f));
	};

	psMipAuto = shaderCache->GetPixelShader(tail(s.Sample(texcoord)));
	psMipLod = shaderCache->GetPixelShader(tail(s.SampleLod(texcoord, uLod)));
	psMipBias = shaderCache->GetPixelShader(tail(s.SampleBias(texcoord, uBias)));
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

//*** class Painter::ModelRender

Painter::ModelRender::ModelRender(Painter* painter) :
	device(painter->device),
	shaderCache(painter->shaderCache),

	iPosition(0),
	iTangent(1),
	iBinormal(2),
	iNormal(3),
	iTexcoord(4),

	ug(NEW(UniformGroup(0))),
	uViewProj(ug->AddUniform<mat4x4>()),
	uEye(ug->AddUniform<vec3>())
{}

Painter::ModelRender::~ModelRender() {}

//*** class Painter::ModelRender::VertexVariantKey

Painter::ModelRender::VertexVariantKey::VertexVariantKey(
	bool bump,
	bool skinned
) :
	bump(bump),
	skinned(skinned)
{}

size_t Painter::ModelRender::VertexVariantKey::GetHash() const
{
	return (bump ? 1 : 0) | (skinned ? 2 : 0);
}

//*** class Painter::ModelRender::PixelVariantKey

Painter::ModelRender::PixelVariantKey::PixelVariantKey(
	bool hasDiffuseTexture,
	bool hasSpecularTexture,
	bool hasNormalTexture
) :
	hasDiffuseTexture(hasDiffuseTexture),
	hasSpecularTexture(hasSpecularTexture),
	hasNormalTexture(hasNormalTexture)
{}

size_t Painter::ModelRender::PixelVariantKey::GetHash() const
{
	return (hasDiffuseTexture ? 1 : 0) | (hasSpecularTexture ? 2 : 0) | (hasNormalTexture ? 4 : 0);
}

//*** class Painter::ModelRender::VertexVariant

Painter::ModelRender::VertexVariant::VertexVariant(ModelRender* modelRender, const VertexVariantKey& key) : key(key)
{
	BEGIN_TRY();

	// calculate size of vertex
	size_t vertexSize =
		sizeof(vec3) // position
		+ (key.bump ? sizeof(vec3) * 2 : 0) // tangent + binormal
		+ sizeof(vec3) // normal
		+ sizeof(vec2) // texcoords
		+ (key.skinned ? sizeof(vec4) + sizeof(uvec4) : 0); // bone weights + bone numbers

	// create vertex & attribute layout
	vl = NEW(VertexLayout(vertexSize));
	al = NEW(AttributeLayout());
	als = al->AddSlot();

	int offset = 0;
	aPosition = al->AddElement(als, vl->AddElement(DataTypes::_vec3, offset));
	offset += sizeof(vec3);
	if(key.bump)
	{
		aTangent = al->AddElement(als, vl->AddElement(DataTypes::_vec3, offset));
		offset += sizeof(vec3);
		aBinormal = al->AddElement(als, vl->AddElement(DataTypes::_vec3, offset));
		offset += sizeof(vec3);
	}
	aNormal = al->AddElement(als, vl->AddElement(DataTypes::_vec3, offset));
	offset += sizeof(vec3);
	aTexcoord = al->AddElement(als, vl->AddElement(DataTypes::_vec2, offset));
	offset += sizeof(vec2);
	if(key.skinned)
	{
		aBoneWeights = al->AddElement(als, vl->AddElement(DataTypes::_vec4, offset));
		offset += sizeof(vec4);
		aBoneNumbers = al->AddElement(als, vl->AddElement(DataTypes::_uvec4, offset));
		offset += sizeof(uvec4);
	}

	ab = modelRender->device->CreateAttributeBinding(al);

	if(!key.skinned)
		instancer = NEW(Instancer(modelRender->device, maxInstancesCount, al));

	// uniforms
	ug = NEW(UniformGroup(1));
	if(key.skinned)
	{
		uBoneOrientations = ug->AddUniformArray<vec4>(maxInstancesCount);
		uBoneOffsets = ug->AddUniformArray<vec4>(maxInstancesCount);
	}
	else
	{
		uWorlds = ug->AddUniformArray<mat4x4>(maxInstancesCount);
	}

	// vertex shader
	{
		BEGIN_TRY();

		// calculate world-space position, tangent/binormal, normal
		Value<vec4> tmpPosition;
		Value<vec3> tmpTangent, tmpBinormal, tmpNormal;
		if(key.skinned)
		{
			static auto applyQuaternion = [](const Value<vec4>& q, const Value<vec3>& v)
			{
				return v + cross(q["xyz"], cross(q["xyz"], v) + v * q["w"]) * val(2.0f);
			};

			static const char xyzw[4][2] = { "x", "y", "z", "w" };

			auto applyBoneOrientation = [this](const Value<vec3>& p) -> Value<vec3>
			{
				Value<vec3> r = newvec3(0.0f, 0.0f, 0.0f);
				for(int i = 0; i < 4; ++i)
				{
					Value<uint> boneNumber = aBoneNumbers[xyzw[i]];
					r += applyQuaternion(uBoneOrientations[boneNumber], p) * aBoneWeights[xyzw[i]];
				}
				return r;
			};
			auto applyBoneTransform = [this](const Value<vec3>& p) -> Value<vec3>
			{
				Value<vec3> r = newvec3(0.0f, 0.0f, 0.0f);
				for(int i = 0; i < 4; ++i)
				{
					Value<uint> boneNumber = aBoneNumbers[xyzw[i]];
					r += (applyQuaternion(uBoneOrientations[boneNumber], p) + uBoneOffsets[boneNumber]["xyz"]) * aBoneWeights[xyzw[i]];
				}
				return r;
			};

			tmpPosition = newvec4(applyBoneTransform(aPosition), 1.0f);
			if(key.bump)
			{
				tmpTangent = applyBoneOrientation(aTangent);
				tmpBinormal = applyBoneOrientation(aBinormal);
			}
			tmpNormal = applyBoneOrientation(aNormal);
		}
		else
		{
			Value<mat4x4> tmpWorld = uWorlds[instancer->GetInstanceID()];

			tmpPosition = mul(tmpWorld, newvec4(aPosition, 1.0f));
			Value<mat3x3> tmpWorldOrientation = tmpWorld.Cast<mat3x3>();
			if(key.bump)
			{
				tmpTangent = mul(tmpWorldOrientation, aTangent);
				tmpBinormal = mul(tmpWorldOrientation, aBinormal);
			}
			tmpNormal = mul(tmpWorldOrientation, aNormal);
		}

		Expression e = (
			setPosition(mul(modelRender->uViewProj, tmpPosition)),
			modelRender->iPosition.Set(tmpPosition["xyz"]),
			modelRender->iNormal.Set(tmpNormal),
			modelRender->iTexcoord.Set(aTexcoord)
		);
		if(key.bump)
		{
			e = (
				e,
				modelRender->iTangent.Set(tmpTangent),
				modelRender->iBinormal.Set(tmpBinormal)
			);
		}
		vs = modelRender->shaderCache->GetVertexShader(e);

		END_TRY("Can't create vertex shader");
	}

	END_TRY("Can't create vertex variant");
}

END_INANITY_OIL
