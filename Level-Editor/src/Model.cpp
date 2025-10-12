#include "Model.hpp"
#include "Filesystem/File.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace SGF {
	/*
	void TraverseNode(GenericModel* model, aiNode* node, const aiMatrix4x4& parentTransform = aiMatrix4x4());
	Texture TextureFromBaseColor(aiColor4D& baseColor) {
		uint8_t texel[4];
		texel[0] = static_cast<uint8_t>(glm::clamp(baseColor.r, 0.0f, 1.0f) * 255.0f);
		texel[1] = static_cast<uint8_t>(glm::clamp(baseColor.g, 0.0f, 1.0f) * 255.0f);
		texel[2] = static_cast<uint8_t>(glm::clamp(baseColor.b, 0.0f, 1.0f) * 255.0f);
		texel[3] = static_cast<uint8_t>(glm::clamp(baseColor.a, 0.0f, 1.0f) * 255.0f);
		return Texture(1, 1, texel);
	}
	
	Texture LoadTextureFromAssimp(GenericModel* model, const aiScene* scene, std::string& path, const char* filename) {
		if (path.empty()) {
			aiColor4D baseColor(0.5, 0.5, 0.5, 1.f);
			return TextureFromBaseColor(baseColor);
		}
		else if (path[0] == '*') {
			// Is embedded
			unsigned int texIndex = std::stoi(path.substr(1));
			const aiTexture* embeddedTex = scene->mTextures[texIndex];
			if (embeddedTex->mHeight == 0) {
				// is compressed
				return Texture((const uint8_t*)embeddedTex->pcData, embeddedTex->mWidth);
			} else {
				// is uncompressed
				return Texture((uint32_t)embeddedTex->mWidth, (uint32_t)embeddedTex->mHeight, (uint8_t*)embeddedTex->pcData);
			}
		} else {
			std::string texFilepath = GetDirectoryFromFilePath(filename) +'/' + path;
			return Texture(texFilepath.c_str());
		}
		//assert(textures[i].GetWidth() != 0 && textures[i].GetHeight() != 0);
	}
	void GetVertexPositionsAndNormals(GenericModel* pModel, const aiMesh* pMesh, GenericModel::MeshInfo& meshInfo) {
		for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
			auto& vert = pMesh->mVertices[j];
			//vertexPositions[m.j]

			pModel->vertices[meshInfo.vertexOffset + j].position.x = pMesh->mVertices[j].x;
			pModel->vertices[meshInfo.vertexOffset + j].position.y = pMesh->mVertices[j].y;
			pModel->vertices[meshInfo.vertexOffset + j].position.z = pMesh->mVertices[j].z;
			pModel->vertices[meshInfo.vertexOffset + j].normal.x = pMesh->mNormals[j].x;
			pModel->vertices[meshInfo.vertexOffset + j].normal.y = pMesh->mNormals[j].y;
			pModel->vertices[meshInfo.vertexOffset + j].normal.z = pMesh->mNormals[j].z;
			//vertices[meshInfo.vertexOffset + j].w = 1.0f;
		}
	}
	void GetIndices(GenericModel* pModel, const aiMesh* pMesh, GenericModel::MeshInfo& meshInfo) {
		for (uint32_t j = 0; j < pMesh->mNumFaces; ++j) {
			auto& smf = pMesh->mFaces[j];
			assert(smf.mNumIndices == 3);
			for (uint32_t k = 0; k < smf.mNumIndices; ++k) {
				pModel->indices[meshInfo.indexOffset + j * smf.mNumIndices + k] = smf.mIndices[k];
			}
		}
	}

	void LoadMesh(GenericModel* pModel, const aiScene* pScene, const aiMesh* pMesh, GenericModel::MeshInfo& meshInfo, std::vector<std::string>& diffuseTextures, const char* filename) {
		// Get vertex positions and indices
		GetIndices(pModel, pMesh, meshInfo);
		
		GetVertexPositionsAndNormals(pModel, pMesh, meshInfo);

		{
			aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];

			uint32_t uvChannelIndex = 0;

			bool hasTexture = false;
			bool hasVertexColors = false;
			bool hasBaseColor = false;
			aiString texPath;
			aiTextureType types[] = { aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR };
			for (auto type : types) {
				if (material->GetTextureCount(type) > 0) {
					// Get the first diffuse texture, along with the UV index it uses
					if (material->GetTexture(type, 0, &texPath, nullptr, &uvChannelIndex) == AI_SUCCESS) {
						bool isAlreadyInside = false;
						for (size_t i = 0; i < diffuseTextures.size(); ++i) {
							if ((diffuseTextures[i].length() == texPath.length) && strncmp(diffuseTextures[i].c_str(), texPath.C_Str(), texPath.length) == 0) {
								isAlreadyInside = true;
								meshInfo.textureIndex = (uint32_t)i;
								break;
							}
						}
						if (!isAlreadyInside) {
							meshInfo.textureIndex = (uint32_t)pModel->textures.size();
							diffuseTextures.emplace_back(texPath.C_Str());
							pModel->textures.push_back(LoadTextureFromAssimp(pModel, pScene, diffuseTextures.back(), filename));
						}

						hasTexture = true;
						break;
					}
				} else {
					meshInfo.textureIndex = UINT32_MAX;
				}
			}
			if (pMesh->HasVertexColors(0)) {
				for (size_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[j + meshInfo.vertexOffset].color.r = pMesh->mColors[0][j].r;
					pModel->vertices[j + meshInfo.vertexOffset].color.g = pMesh->mColors[0][j].g;
					pModel->vertices[j + meshInfo.vertexOffset].color.b = pMesh->mColors[0][j].b;
					pModel->vertices[j + meshInfo.vertexOffset].color.a = pMesh->mColors[0][j].a;
				}
				hasVertexColors = true;
			} else {
				aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f); // default white
				if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &baseColor) ||
					AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &baseColor)) {
					hasBaseColor = true;
					info("Base color is: ", baseColor.r, ", ", baseColor.g, ", ", baseColor.b, ", ", baseColor.a);
				}
				for (size_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[j + meshInfo.vertexOffset].color.r = baseColor.r;
					pModel->vertices[j + meshInfo.vertexOffset].color.g = baseColor.g;
					pModel->vertices[j + meshInfo.vertexOffset].color.b = baseColor.b;
					pModel->vertices[j + meshInfo.vertexOffset].color.a = baseColor.a;
				}
			}

			// Now get UVs from the correct channel
			if (hasTexture && pMesh->HasTextureCoords(uvChannelIndex)) {
				for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
					aiVector3D uv = pMesh->mTextureCoords[uvChannelIndex][j];
					pModel->vertices[meshInfo.vertexOffset + j].uv.x = uv.x;
					pModel->vertices[meshInfo.vertexOffset + j].uv.y = uv.y;
				}
			} else {
				if (hasTexture) warn("mesh has a texture but doesnt have uv coordinates!");
				for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[meshInfo.vertexOffset + j].uv.x = 2.0f;
					pModel->vertices[meshInfo.vertexOffset + j].uv.y = 2.0f;
				}
			}
			if (hasBaseColor) info("has base color");
			if (hasVertexColors) info("has vertex colors");
			if (hasTexture) info("Has texture");
			if (!hasBaseColor && !hasTexture && !hasVertexColors) warn("Mesh doesnt have any color information!");
		}
	}

	void GenericModel::LoadFromFile(const char* filename) {
		Clear();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_GenBoundingBoxes | aiProcess_FlipUVs);
		debugName = scene->mName.C_Str();

		if (scene == nullptr) {
			SGF::warn("No Scene available!");
			return;
		}

		// Set Bounding-Boxes for each mesh and reserve the Containers
		{
			meshInfos.resize(scene->mNumMeshes);
			uint32_t totalIndices = 0;
			uint32_t totalVertices = 0;
			for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
				auto& m = meshInfos[i];
				auto& sm = scene->mMeshes[i];
				m.boundingBox.max.x = sm->mAABB.mMax.x;
				m.boundingBox.max.y = sm->mAABB.mMax.y;
				m.boundingBox.max.z = sm->mAABB.mMax.z;

				m.boundingBox.min.x = sm->mAABB.mMin.x;
				m.boundingBox.min.y = sm->mAABB.mMin.y;
				m.boundingBox.min.z = sm->mAABB.mMin.z;
				
				m.vertexCount = sm->mNumVertices;
				m.vertexOffset = totalVertices;
				m.indexCount = sm->mNumFaces * 3;
				m.indexOffset = totalIndices;

				totalVertices += sm->mNumVertices;
				totalIndices += sm->mNumFaces * 3;
			}

			vertices.resize(totalVertices);
			indices.resize(totalIndices);
		}
		// Get texture uv-coordinates and texture indices
		{
			std::vector<std::string> diffuseTextures;
			diffuseTextures.reserve(scene->mNumMeshes);
			std::vector<aiColor4D> baseColors;
			textures.reserve(scene->mNumMeshes);

			for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
				aiMesh* mesh = scene->mMeshes[meshIndex];
				auto& meshInfo = meshInfos[meshIndex];
				LoadMesh(this, scene, mesh, meshInfo, diffuseTextures, filename);
			}
			// load all textures:
			{
				for (size_t i = 0; i < diffuseTextures.size(); ++i) {
					assert(textures[i].GetWidth() != 0 && textures[i].GetHeight() != 0);
				}
				textures.shrink_to_fit();
			}
		}
		TraverseNode(this, scene->mRootNode);
		modelTransforms.emplace_back(1.f);
	}

	void TraverseNode(GenericModel* model, aiNode* node, const aiMatrix4x4& parentTransform) {
		// Combine parent and local transform
		aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

		// Process all meshes under this node
		for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
			unsigned int meshIndex = node->mMeshes[i];
			glm::mat4 transform;
			for (uint32_t j = 0; j < 4; ++j) {
				for (uint32_t k = 0; k < 4; ++k) {
					transform[k][j] = globalTransform[j][k];
				}
			}
			model->meshInfos[meshIndex].instanceTransforms.push_back(transform);
		}
		// Recurse into children
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			TraverseNode(model, node->mChildren[i], globalTransform);
		}
	}
	*/

	void TraverseNode(GenericModel* pModel, aiNode* pNode, uint32_t parentIndex = 0);

	void BuildNode(GenericModel* pModel, aiNode* pNode, GenericModel::Node& node);

	Texture TextureFromBaseColor(aiColor4D& baseColor) {
		uint8_t texel[4];
		texel[0] = static_cast<uint8_t>(glm::clamp(baseColor.r, 0.0f, 1.0f) * 255.0f);
		texel[1] = static_cast<uint8_t>(glm::clamp(baseColor.g, 0.0f, 1.0f) * 255.0f);
		texel[2] = static_cast<uint8_t>(glm::clamp(baseColor.b, 0.0f, 1.0f) * 255.0f);
		texel[3] = static_cast<uint8_t>(glm::clamp(baseColor.a, 0.0f, 1.0f) * 255.0f);
		return Texture(1, 1, texel);
	}
	
	Texture LoadTextureFromAssimp(GenericModel* model, const aiScene* scene, std::string& path, const char* filename) {
		if (path.empty()) {
			warn("Texture is requested but path is empty!");
			aiColor4D baseColor(0.5, 0.5, 0.5, 1.f);
			return TextureFromBaseColor(baseColor);
		}
		else if (path[0] == '*') {
			// Is embedded
			unsigned int texIndex = std::stoi(path.substr(1));
			const aiTexture* embeddedTex = scene->mTextures[texIndex];
			if (embeddedTex->mHeight == 0) {
				// is compressed
				return Texture((const uint8_t*)embeddedTex->pcData, embeddedTex->mWidth);
			} else {
				// is uncompressed
				return Texture((uint32_t)embeddedTex->mWidth, (uint32_t)embeddedTex->mHeight, (uint8_t*)embeddedTex->pcData);
			}
		} else {
			std::string texFilepath = GetDirectoryFromFilePath(filename) +'/' + path;
			return Texture(texFilepath.c_str());
		}
		//assert(textures[i].GetWidth() != 0 && textures[i].GetHeight() != 0);
	}
	void GetVertexPositionsAndNormals(GenericModel* pModel, const aiMesh* pMesh, GenericModel::Mesh& meshInfo) {
		for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
			auto& vert = pMesh->mVertices[j];
			//vertexPositions[m.j]
			pModel->vertices[meshInfo.vertexOffset + j].position.x = pMesh->mVertices[j].x;
			pModel->vertices[meshInfo.vertexOffset + j].position.y = pMesh->mVertices[j].y;
			pModel->vertices[meshInfo.vertexOffset + j].position.z = pMesh->mVertices[j].z;
			pModel->vertices[meshInfo.vertexOffset + j].normal.x = pMesh->mNormals[j].x;
			pModel->vertices[meshInfo.vertexOffset + j].normal.y = pMesh->mNormals[j].y;
			pModel->vertices[meshInfo.vertexOffset + j].normal.z = pMesh->mNormals[j].z;
			//vertices[meshInfo.vertexOffset + j].w = 1.0f;
		}
	}
	void GetIndices(GenericModel* pModel, const aiMesh* pMesh, GenericModel::Mesh& meshInfo) {
		for (uint32_t j = 0; j < pMesh->mNumFaces; ++j) {
			auto& smf = pMesh->mFaces[j];
			assert(smf.mNumIndices == 3);
			for (uint32_t k = 0; k < smf.mNumIndices; ++k) {
				pModel->indices[meshInfo.indexOffset + j * smf.mNumIndices + k] = smf.mIndices[k];
			}
		}
	}

	void LoadMesh(GenericModel* pModel, const aiScene* pScene, const aiMesh* pMesh, GenericModel::Mesh& meshInfo, std::vector<std::string>& diffuseTextures, const char* filename) {
		// Get vertex positions and indices
		GetIndices(pModel, pMesh, meshInfo);
		
		GetVertexPositionsAndNormals(pModel, pMesh, meshInfo);
		{
			aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];

			uint32_t uvChannelIndex = 0;

			bool hasTexture = false;
			bool hasVertexColors = false;
			bool hasBaseColor = false;
			aiString texPath;
			aiTextureType types[] = { aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR };
			for (auto type : types) {
				if (material->GetTextureCount(type) > 0) {
					// Get the first diffuse texture, along with the UV index it uses
					if (material->GetTexture(type, 0, &texPath, nullptr, &uvChannelIndex) == AI_SUCCESS) {
						bool isAlreadyInside = false;
						for (size_t i = 0; i < diffuseTextures.size(); ++i) {
							if ((diffuseTextures[i].length() == texPath.length) && strncmp(diffuseTextures[i].c_str(), texPath.C_Str(), texPath.length) == 0) {
								isAlreadyInside = true;
								meshInfo.textureIndex = (uint32_t)i;
								break;
							}
						}
						if (!isAlreadyInside) {
							meshInfo.textureIndex = (uint32_t)pModel->textures.size();
							diffuseTextures.emplace_back(texPath.C_Str());
							pModel->textures.push_back(LoadTextureFromAssimp(pModel, pScene, diffuseTextures.back(), filename));
						}

						hasTexture = true;
						break;
					}
				} else {
					meshInfo.textureIndex = UINT32_MAX;
				}
			}
			if (pMesh->HasVertexColors(0)) {
				for (size_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[j + meshInfo.vertexOffset].color.r = pMesh->mColors[0][j].r;
					pModel->vertices[j + meshInfo.vertexOffset].color.g = pMesh->mColors[0][j].g;
					pModel->vertices[j + meshInfo.vertexOffset].color.b = pMesh->mColors[0][j].b;
					pModel->vertices[j + meshInfo.vertexOffset].color.a = pMesh->mColors[0][j].a;
				}
				hasVertexColors = true;
			} else {
				aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f); // default white
				if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &baseColor) ||
					AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &baseColor)) {
					hasBaseColor = true;
					info("Base color is: ", baseColor.r, ", ", baseColor.g, ", ", baseColor.b, ", ", baseColor.a);
				}
				for (size_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[j + meshInfo.vertexOffset].color.r = baseColor.r;
					pModel->vertices[j + meshInfo.vertexOffset].color.g = baseColor.g;
					pModel->vertices[j + meshInfo.vertexOffset].color.b = baseColor.b;
					pModel->vertices[j + meshInfo.vertexOffset].color.a = baseColor.a;
				}
			}

			// Now get UVs from the correct channel
			if (hasTexture && pMesh->HasTextureCoords(uvChannelIndex)) {
				for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
					aiVector3D uv = pMesh->mTextureCoords[uvChannelIndex][j];
					pModel->vertices[meshInfo.vertexOffset + j].uv.x = uv.x;
					pModel->vertices[meshInfo.vertexOffset + j].uv.y = uv.y;
				}
			} else {
				if (hasTexture) warn("mesh has a texture but doesnt have uv coordinates!");
				for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[meshInfo.vertexOffset + j].uv.x = 2.0f;
					pModel->vertices[meshInfo.vertexOffset + j].uv.y = 2.0f;
				}
			}
			if (hasBaseColor) info("has base color");
			if (hasVertexColors) info("has vertex colors");
			if (hasTexture) info("Has texture");
			if (!hasBaseColor && !hasTexture && !hasVertexColors) warn("Mesh doesnt have any color information!");
		}
	}

	const GenericModel::Node* GenericModel::ImportModel(const char* filename) {
		//Clear();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_GenBoundingBoxes | aiProcess_FlipUVs);
		name = scene->mName.C_Str();

		const GenericModel::Node* pAttachmentNode = nullptr;
		if (scene == nullptr) {
			SGF::warn("No Scene available!");
			return nullptr;
		}
		auto* pRoot = scene->mRootNode;
		if (nodes.size() == 0) {
			// Get root from model:
			nodes.emplace_back();
			auto& root = nodes[0];
			root.parent = UINT32_MAX;
			root.name = pRoot->mName.Empty() ? "root" : pRoot->mName.C_Str();
			BuildNode(this, pRoot, root);
			root.children.reserve(pRoot->mNumChildren);
			pAttachmentNode = &root;
			for (unsigned int i = 0; i < pRoot->mNumChildren; ++i) {
				TraverseNode(this, pRoot->mChildren[i], 0);
			}
		} else {
			TraverseNode(this, pRoot, 0);
		}
		

		// Set Bounding-Boxes for each mesh and reserve the Containers
		{
			meshes.resize(scene->mNumMeshes);
			uint32_t totalIndices = (uint32_t)indices.size();
			uint32_t totalVertices = (uint32_t)vertices.size();
			for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
				auto& m = meshes[i];
				auto& sm = scene->mMeshes[i];
				m.boundingBox.max.x = sm->mAABB.mMax.x;
				m.boundingBox.max.y = sm->mAABB.mMax.y;
				m.boundingBox.max.z = sm->mAABB.mMax.z;

				m.boundingBox.min.x = sm->mAABB.mMin.x;
				m.boundingBox.min.y = sm->mAABB.mMin.y;
				m.boundingBox.min.z = sm->mAABB.mMin.z;
				
				m.vertexCount = sm->mNumVertices;
				m.vertexOffset = totalVertices;
				m.indexCount = sm->mNumFaces * 3;
				m.indexOffset = totalIndices;

				totalVertices += sm->mNumVertices;
				totalIndices += sm->mNumFaces * 3;
			}

			vertices.resize(totalVertices);
			indices.resize(totalIndices);
		}
		// Get texture uv-coordinates and texture indices
		{
			std::vector<std::string> diffuseTextures;
			diffuseTextures.reserve(scene->mNumMeshes);
			std::vector<aiColor4D> baseColors;
			textures.reserve(scene->mNumMeshes);

			for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
				aiMesh* mesh = scene->mMeshes[meshIndex];
				auto& m = meshes[meshIndex];
				LoadMesh(this, scene, mesh, m, diffuseTextures, filename);
			}
			// load all textures:
			{
				for (size_t i = 0; i < diffuseTextures.size(); ++i) {
					assert(textures[i].GetWidth() != 0 && textures[i].GetHeight() != 0);
				}
				textures.shrink_to_fit();
			}
		}
		return pAttachmentNode;
	}
	void BuildNode(GenericModel* pModel, aiNode* pNode, GenericModel::Node& node) {
		// Copy transformation matrix
		for (uint32_t j = 0; j < 4; ++j) {
			for (uint32_t k = 0; k < 4; ++k) {
				node.localTransform[k][j] = pNode->mTransformation[j][k];
			}
		}
		if (node.parent != UINT32_MAX) {
			node.globalTransform = pModel->nodes[node.parent].globalTransform * node.localTransform;
		} else {
			node.globalTransform = node.localTransform;
		}
		// Process all meshes under this node
		node.meshes.reserve(pNode->mNumMeshes);
		for (unsigned int i = 0; i < pNode->mNumMeshes; ++i) {
			unsigned int meshIndex = pNode->mMeshes[i] + pModel->meshes.size();
			node.meshes.push_back(meshIndex);
		}
	}

	void TraverseNode(GenericModel* pModel, aiNode* pNode, uint32_t parentIndex) {
		assert(pModel != nullptr);
		assert(parentIndex != UINT32_MAX); // no root node should be created here
		assert(parentIndex < pModel->nodes.size() || parentIndex == UINT32_MAX);

		uint32_t nodeIndex = (uint32_t)pModel->nodes.size();
		pModel->nodes.emplace_back();
		GenericModel::Node& node = pModel->nodes.back();
		if (!pNode->mName.Empty()) {
			node.name = pNode->mName.C_Str();
		} else {
			node.name = "_node"; 
		}

		node.parent = parentIndex;
		pModel->nodes[parentIndex].children.push_back(nodeIndex);

		BuildNode(pModel, pNode, node);

		// Recurse into children
		node.children.reserve(pNode->mNumChildren);
		for (unsigned int i = 0; i < pNode->mNumChildren; ++i) {
			TraverseNode(pModel, pNode->mChildren[i], nodeIndex);
		}
	}
}