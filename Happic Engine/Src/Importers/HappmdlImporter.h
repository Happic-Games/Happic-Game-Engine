#pragma once

#include "../Rendering/Mesh.h"
#include "../DataStructures/String.h"
#include <fstream>

namespace Happic {

	enum HappmdlVertexAttributeFlag
	{
		HAPPMDL_VERTEX_ATTRIBUTE_POS = 1,
		HAPPMDL_VERTEX_ATTRIBUTE_TEXCOORD = 2,
		HAPPMDL_VERTEX_ATTRIBUTE_COLOR = 4,
		HAPPMDL_VERTEX_ATTRIBUTE_NORMAL = 8,
		HAPPMDL_VERTEX_ATTRIBUTE_TANGENT = 16
	};

	enum HappmdlMaterialTextureFlag
	{
		HAPPMDL_MATERIAL_TEXTURE_ALBEDO = 1,
		HAPPMDL_MATERIAL_TEXTURE_NORMAL = 2,
		HAPPMDL_MATERIAL_TEXTURE_DISPLACEMENT = 4,
		HAPPMDL_MATERIAL_TEXTURE_METALLIC = 8,
		HAPPMDL_MATERIAL_TEXTURE_ROUGHNESS = 16
	};

	struct HappmdlModelMetaData
	{
		uint32 numMaterials;
		uint32 numMeshes;
		uint32 vertexAttributeFlags;
		uint32 materialTextureFlags;
	};

	struct HappmdlMaterial
	{
		String albedo;
	};

	struct HappmdlMesh
	{
		uint32 materialIndex;
		std::vector<Rendering::Vertex> vertices;
		std::vector<uint32> indices;
	};

	class HappmdlImporter
	{
	public:
		Rendering::Mesh LoadModel(const String& file, Rendering::Material* pDefaultMaterial = NULL);
	private:
		bool OpenFile(const String& file);
		bool VerifyHeader();
		void LoadMetaData(HappmdlModelMetaData* pMetaData);
		void LoadMaterial(HappmdlMaterial* pMaterial);
		void LoadMesh(HappmdlMesh* pMesh);
		String LoadString();
		void CloseFile();
	private:
		std::ifstream m_file;
		char* m_buffer;
		uint32 m_offset;
	};

}