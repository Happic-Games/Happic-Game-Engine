#pragma once

#include "ITexture2D.h"
#include "ITextureSampler.h"
#include <map>

#define MATERIAL_TEXTURE_ALBEDO "Albedo"

namespace Happic { namespace Rendering {

	typedef uint32 TextureGroupID;

	struct TextureSamplerPair
	{
		ITexture2D*			pTexture;
		ITextureSampler*	pSampler;
	};

	class TextureGroup
	{
	private:
		struct TextureSamplerInfo
		{
			TextureSamplerInfo(ITexture2D* pTexture, String samplerName) :
				pTexture(pTexture),
				samplerName(samplerName)
			{}

			TextureSamplerInfo() : pTexture(NULL) {}

			ITexture2D*			pTexture;
			String				samplerName;
		};

	public:
		inline TextureGroup() { m_id = s_lastID++; }

		inline void AddSampler(const String& name, ITextureSampler* pSampler) { m_samplers[name] = pSampler; }
		inline void AddTexture(const String& textureName, ITexture2D* pTexture, const String& samplerName) 
		{ m_textures[textureName] = TextureSamplerInfo(pTexture, samplerName); }

		inline ITexture2D* GetTexture(const String& textureName) const		{ return m_textures.at(textureName).pTexture; }
		inline ITextureSampler* GetSampler(const String& samplerName) const { return m_samplers.at(samplerName); }
		inline TextureSamplerPair GetTextureAndSampler(const String& textureName) const
		{
			TextureSamplerInfo info = m_textures.at(textureName);
			TextureSamplerPair pair;
			pair.pTexture = info.pTexture;
			pair.pSampler = m_samplers.at(info.samplerName);
			return pair;
		}

		inline uint32 GetID() const { return m_id; }

		inline bool operator==(const TextureGroup& other) const { return m_id == other.m_id; }
	private:
		std::map<String, ITextureSampler*>		m_samplers;
		std::map<String, TextureSamplerInfo>		m_textures;

		TextureGroupID m_id;

		static uint32 s_lastID;
	};

	class Material : public TextureGroup
	{
	public:
		inline Material(ITextureSampler* pSampler, ITexture2D* pAlbedo)
		{
			AddSampler("MSamp", pSampler);
			AddTexture(MATERIAL_TEXTURE_ALBEDO, pAlbedo, "MSamp");
		}

		Material() {}
	};

} }
