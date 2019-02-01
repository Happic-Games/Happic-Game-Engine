#include "HappmdlImporter.h"

namespace Happic {

	Rendering::Mesh HappmdlImporter::LoadModel(const String & file, Rendering::Material* pDefaultMaterial)
	{
		if (!OpenFile(file))
			return Rendering::Mesh();

		Rendering::Mesh renderMesh;

		if (!VerifyHeader())
			return Rendering::Mesh();

		HappmdlModelMetaData metaData;
		LoadMetaData(&metaData);
		for (uint32 i = 0; i < metaData.numMaterials; i++)
		{
			HappmdlMaterial material;
			LoadMaterial(&material);
			Rendering::Material renderMaterial(Rendering::ITextureSampler::CreateTextureSampler(Rendering::FILTER_LINEAR, Rendering::ADDRESS_MODE_REPEAT, Rendering::ANISOTROPY_16),
				Rendering::ITexture2D::CreateTexture(material.albedo));
			renderMesh.AddTextureGroup(renderMaterial);
		}

		if (metaData.numMaterials == 0)
		{
			renderMesh.AddTextureGroup(*pDefaultMaterial);
		}

		std::vector<Rendering::Vertex> vertices;
		std::vector<uint32> indices;
		for (uint32 i = 0; i < metaData.numMeshes; i++)
		{
			HappmdlMesh mesh;
			LoadMesh(&mesh);

			renderMesh.AddSubDraw(indices.size(), indices.size() + mesh.indices.size() - 1, mesh.materialIndex);

			for (uint32 j = 0; j < mesh.vertices.size(); j++)
				vertices.push_back(mesh.vertices[j]);
			for (uint32 j = 0; j < mesh.indices.size(); j++)
				indices.push_back(mesh.indices[j]);
		}


		renderMesh.pVertexBuffer = Rendering::IBuffer::CreateBuffer(Rendering::BUFFER_TYPE_VERTEX, Rendering::BUFFER_USAGE_STATIC,
			&vertices[0], vertices.size() * sizeof(Rendering::Vertex));
		renderMesh.pIndexBuffer = Rendering::IBuffer::CreateBuffer(Rendering::BUFFER_TYPE_INDEX, Rendering::BUFFER_USAGE_STATIC,
			&indices[0], indices.size() * sizeof(uint32));

		renderMesh.type = Rendering::DRAW_COMMAND_DRAW_INDEXED;

		CloseFile();
		return renderMesh;
	}

	bool HappmdlImporter::OpenFile(const String & file)
	{
		m_file.open(file.C_Str(), std::ios::binary | std::ios::in);

		if (!m_file.is_open())
			return false;

		std::streampos size = m_file.tellg();
		m_file.seekg(0, std::ios::end);
		size = m_file.tellg() - size;
		m_file.seekg(0, std::ios::beg);
		m_buffer = new char[size];
		m_offset = 0;
		m_file.read(m_buffer, size);

		return true;
	}

	bool HappmdlImporter::VerifyHeader()
	{
		char fileTypeValidation[7];
		memcpy(fileTypeValidation, m_buffer, 7);

		char* pFileType = "happmdl";
		for (uint32 i = 0; i < 7; i++)
			if (fileTypeValidation[i] != pFileType[i])
				return false;

		if (m_buffer[7] != 1)
			return false;

		m_offset = 8;
		return true;
	}

	void HappmdlImporter::LoadMetaData(HappmdlModelMetaData* pMetaData)
	{
		memcpy(pMetaData, m_buffer + m_offset, sizeof(HappmdlModelMetaData));
		m_offset += sizeof(HappmdlModelMetaData);
	}

	void HappmdlImporter::LoadMaterial(HappmdlMaterial * pMaterial)
	{
		pMaterial->albedo = LoadString();
	}

	void HappmdlImporter::LoadMesh(HappmdlMesh * pMesh)
	{
		uint32 numVertices;
		uint32 numIndices;
		memcpy(&numVertices,				m_buffer + m_offset, sizeof(uint32));		m_offset += sizeof(uint32);
		memcpy(&numIndices,					m_buffer + m_offset, sizeof(uint32));		m_offset += sizeof(uint32);
		memcpy(&pMesh->materialIndex,		m_buffer + m_offset, sizeof(uint32));		m_offset += sizeof(uint32);

		pMesh->vertices.resize(numVertices);
		pMesh->indices.resize(numIndices);

		memcpy(&pMesh->vertices[0], m_buffer + m_offset, sizeof(Rendering::Vertex) * numVertices);
		m_offset += sizeof(Rendering::Vertex) * numVertices;

		memcpy(&pMesh->indices[0], m_buffer + m_offset, sizeof(uint32) * numIndices);
		m_offset += sizeof(uint32) * numIndices;
	}

	String HappmdlImporter::LoadString()
	{
		uint32 length = 0;
		memcpy(&length, m_buffer + m_offset, sizeof(uint32));
		m_offset += sizeof(uint32);
		char* string = new char[length + 1];
		memcpy(string, m_buffer + m_offset, length);
		m_offset += length;
		string[length] = '\0';
		String str(string);
		delete[] string;
		return str;
	}

	void HappmdlImporter::CloseFile()
	{
		m_file.close();
		delete[] m_buffer;
		m_offset = 0;
	}

}
