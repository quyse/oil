#ifndef ___INANITY_OIL_PAINTER_HPP___
#define ___INANITY_OIL_PAINTER_HPP___

#include "oil.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/shaders/Value.hpp"
#include "../inanity/graphics/shaders/Interpolant.hpp"
#include "../inanity/graphics/shaders/Uniform.hpp"
#include "../inanity/graphics/shaders/UniformArray.hpp"
#include "../inanity/graphics/shaders/Sampler.hpp"
#include <unordered_map>

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
class AttributeNode;
class Instancer;

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
		GS::Uniform<G::mat4x4> uColorTransform;
		GS::Uniform<G::vec4> uColorOffset;
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
			Let(
				G::Context* context,
				const TextureQuad* textureQuad,
				ptr<G::Texture> t,
				ptr<G::SamplerState> ss, MipMode mipMode
			);
			~Let();
		};
	};

	class ModelRender : public Object
	{
	private:
		ptr<G::Device> device;
		ptr<G::ShaderCache> shaderCache;

		static const int maxInstancesCount = 64;

		/// Key for variant of vertex shader.
		struct VertexVariantKey
		{
			/// Does mesh have tangent and binormal?
			bool bump;
			/// Does mesh use skinning?
			bool skinned;

			VertexVariantKey(bool bump, bool skinned);

			size_t GetHash() const;
		};

		/// Key for variant of pixel shader.
		struct PixelVariantKey
		{
			/// Does material have a diffuse texture?
			bool hasDiffuseTexture;
			/// Does material have a specular texture?
			bool hasSpecularTexture;
			/// Does material have a normal texture?
			bool hasNormalTexture;

			PixelVariantKey(bool hasDiffuseTexture, bool hasSpecularTexture, bool hasNormalTexture);

			size_t GetHash() const;
		};

		/// Struct containing everything needed for vertex shader.
		struct VertexVariant
		{
			VertexVariantKey key;

			ptr<G::VertexLayout> vl;
			ptr<G::AttributeLayout> al;
			ptr<G::AttributeLayoutSlot> als;
			//*** Attributes.
			GS::Value<G::vec3> aPosition;
			GS::Value<G::vec3> aTangent; // if key.bump
			GS::Value<G::vec3> aBinormal; // if key.bump
			GS::Value<G::vec3> aNormal;
			GS::Value<G::vec2> aTexcoord;
			GS::Value<G::vec4> aBoneWeights; // if key.skinned
			GS::Value<G::uvec4> aBoneNumbers; // if key.skinned

			ptr<G::AttributeBinding> ab;

			ptr<GS::Instancer> instancer; // if !key.skinned

			ptr<GS::UniformGroup> ug;
			GS::UniformArray<G::mat4x4> uWorlds; // if !key.skinned
			GS::UniformArray<G::vec4> uBoneOrientations; // if key.skinned
			GS::UniformArray<G::vec4> uBoneOffsets; // if key.skinned

			ptr<G::VertexShader> vs;

			VertexVariant(ModelRender* modelRender, const VertexVariantKey& key);
		};

		/// Struct containing everything needed for pixel shader.
		struct PixelVariant
		{
			PixelVariantKey key;

			//ptr<GS::UniformGroup> ug;
			GS::Sampler<G::vec4, 2> sDiffuse;
			GS::Sampler<G::vec4, 2> sSpecular;

			PixelVariant(ModelRender* modelRender, const PixelVariantKey& key);
		};

		/// Vertex variants.
		typedef std::unordered_map<size_t, VertexVariant> VertexVariants;
		VertexVariants vertexVariants;

		/// Pixel variants.
		typedef std::unordered_map<size_t, PixelVariant> PixelVariants;
		PixelVariants pixelVariants;

		//*** Interpolants.
		GS::Interpolant<G::vec3> iPosition; // in world-space
		GS::Interpolant<G::vec3> iTangent; // in tangent-space
		GS::Interpolant<G::vec3> iBinormal; // in tangent-space
		GS::Interpolant<G::vec3> iNormal; // in tangent-space
		GS::Interpolant<G::vec2> iTexcoord;

		ptr<GS::UniformGroup> ug;
		GS::Uniform<G::mat4x4> uViewProj;
		GS::Uniform<G::vec3> uEye;

	public:
		ModelRender(Painter* painter);
		~ModelRender();
	};

public: // for convenience
	ptr<Quad> quad;
	ptr<TextureQuad> textureQuad;
	ptr<ModelRender> modelRender;

public:
	Painter(ptr<G::Device> device, ptr<G::Context> context, ptr<G::ShaderCache> shaderCache);
	~Painter();
};

END_INANITY_OIL

#undef G
#undef GS

#endif
