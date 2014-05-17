#ifndef ___INANITY_OIL_PAINTER_HPP___
#define ___INANITY_OIL_PAINTER_HPP___

#include "oil.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/shaders/Value.hpp"
#include "../inanity/graphics/shaders/Interpolant.hpp"
#include "../inanity/graphics/shaders/Uniform.hpp"
#include "../inanity/graphics/shaders/Sampler.hpp"

#define G Inanity::Graphics
#define GS Inanity::Graphics::Shaders

BEGIN_INANITY_GRAPHICS

class Device;
class ShaderCache;
class VertexLayout;
class AttributeLayout;
class AttributeLayoutSlot;
class VertexBuffer;
class IndexBuffer;
class AttributeBinding;
class Texture;
class SamplerState;

END_INANITY_GRAPHICS

BEGIN_INANITY_SHADERS

class UniformGroup;

END_INANITY_SHADERS

BEGIN_INANITY_OIL

class Painter : public Object
{
private:
	ptr<G::Device> device;
	ptr<G::Context> context;
	ptr<G::ShaderCache> shaderCache;

public:
	class Quad : public Object
	{
	private:
		struct Vertex
		{
			G::vec4 position;
			G::vec2 texcoord;
		};
		static Vertex vertices[4];
		static unsigned short indices[6];

		ptr<G::VertexLayout> vl;
		ptr<G::AttributeLayout> al;
		ptr<G::AttributeLayoutSlot> als;
	public:
		GS::Value<G::vec4> aPosition;
		GS::Value<G::vec2> aTexcoord;

		GS::Interpolant<G::vec2> iTexcoord;

	private:
		ptr<G::VertexBuffer> vb;
		ptr<G::IndexBuffer> ib;
		ptr<G::AttributeBinding> ab;
		ptr<G::VertexShader> vs;

	public:
		Quad(ptr<G::Device> device, ptr<G::ShaderCache> shaderCache);

		class Let
		{
		private:
			G::Context::LetAttributeBinding lab;
			G::Context::LetVertexBuffer lvb;
			G::Context::LetIndexBuffer lib;
			G::Context::LetVertexShader lvs;

		public:
			Let(G::Context* context, const Quad* quad);
		};
	};

	/// Painter for texture view renderer.
	class TextureQuad : public Object
	{
	private:
		ptr<Quad> quad;
		ptr<GS::UniformGroup> ug;

	public:
		GS::Uniform<G::vec2> uTextureScale;
		GS::Uniform<G::vec2> uBackgroundScale;
		GS::Uniform<G::vec2> uOffset;
		GS::Uniform<float> uLod;
		GS::Uniform<float> uBias;
		GS::Sampler<G::vec4, 2> s;

	private:
		ptr<G::PixelShader> psMipAuto;
		ptr<G::PixelShader> psMipLod;
		ptr<G::PixelShader> psMipBias;

	public:
		TextureQuad(ptr<G::Device> device, ptr<G::ShaderCache> shaderCache, ptr<Quad> quad);
		~TextureQuad();

		class Let
		{
		private:
			Quad::Let lq;
			G::Context::LetUniformBuffer lub;
			G::Context::LetSampler ls;
			G::Context::LetPixelShader lps;

		public:
			enum MipMode
			{
				mipModeAuto,
				mipModeLod,
				mipModeBias
			};

			static ptr<G::PixelShader> GetPixelShader(const TextureQuad* textureQuad, MipMode mipMode);

		public:
			Let(G::Context* context, const TextureQuad* textureQuad, ptr<G::Texture> t, ptr<G::SamplerState> ss, MipMode mipMode);
			~Let();
		};
	};

public: // for convenience
	ptr<Quad> quad;
	ptr<TextureQuad> textureQuad;

public:
	Painter(ptr<G::Device> device, ptr<G::Context> context, ptr<G::ShaderCache> shaderCache);
	~Painter();
};

END_INANITY_OIL

#undef G
#undef GS

#endif
