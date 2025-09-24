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
	void TraverseNode(GenericModel* model, aiNode* node, const aiMatrix4x4& parentTransform = aiMatrix4x4());

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

			vertexPositions.resize(totalVertices);
			vertexNormals.resize(totalVertices);
			uvCoordinates.resize(totalVertices);
			indices.resize(totalIndices);
		}
		// Get texture uv-coordinates and texture indices
		{
			std::vector<std::string> diffuseTextures;
			diffuseTextures.reserve(scene->mNumMeshes);

			for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
				aiMesh* mesh = scene->mMeshes[meshIndex];
				auto& meshInfo = meshInfos[meshIndex];

				// Get vertex positions and indices
				for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
					auto& smf = mesh->mFaces[j];
					assert(smf.mNumIndices == 3);
					for (uint32_t k = 0; k < smf.mNumIndices; ++k) {
						indices[meshInfo.indexOffset + j * smf.mNumIndices + k] = smf.mIndices[k];
					}
				}
				for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
					auto& vert = mesh->mVertices[j];
					//vertexPositions[m.j]

					vertexPositions[meshInfo.vertexOffset + j].x = mesh->mVertices[j].x;
					vertexPositions[meshInfo.vertexOffset + j].y = mesh->mVertices[j].y;
					vertexPositions[meshInfo.vertexOffset + j].z = mesh->mVertices[j].z;
					vertexPositions[meshInfo.vertexOffset + j].w = 1.0f;

					vertexNormals[meshInfo.vertexOffset + j].x = mesh->mNormals[j].x;
					vertexNormals[meshInfo.vertexOffset + j].y = mesh->mNormals[j].y;
					vertexNormals[meshInfo.vertexOffset + j].z = mesh->mNormals[j].z;
					vertexNormals[meshInfo.vertexOffset + j].w = 1.0f;
				}

				{
					uint32_t matIndex = mesh->mMaterialIndex;
					aiMaterial* material = scene->mMaterials[matIndex];

					uint32_t uvChannelIndex = 0;
					uint32_t textureIndex = 0;

					aiString texPath;
					if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
						// Get the first diffuse texture, along with the UV index it uses
						if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath, nullptr, &uvChannelIndex) == AI_SUCCESS) {
							bool isAlreadyInside = false;
							for (size_t i = 0; i < diffuseTextures.size(); ++i) {
								if ((diffuseTextures[i].length() == texPath.length) && strncmp(diffuseTextures[i].c_str(), texPath.C_Str(), texPath.length) == 0) {
									isAlreadyInside = true;
									textureIndex = (uint32_t)i;
									break;
								}
							}
							if (!isAlreadyInside) {
								textureIndex = (uint32_t)diffuseTextures.size();
								diffuseTextures.emplace_back(texPath.C_Str());
							}
						}
					} else {
						warn("mesh doesnt have a diffuse texture!");
						// TODO: handle case!
					}
					meshInfo.textureIndex = textureIndex;

					// Now get UVs from the correct channel
					if (mesh->HasTextureCoords(uvChannelIndex)) {
						for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
							aiVector3D uv = mesh->mTextureCoords[uvChannelIndex][j];
							uvCoordinates[meshInfo.vertexOffset + j].x = uv.x;
							uvCoordinates[meshInfo.vertexOffset + j].y = uv.y;
						}
					} else {
						warn("mesh doesnt have uv coordinates!");
						//  TODO: handle case!
						for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
							uvCoordinates[meshInfo.vertexOffset + j].x = 0.0f;
							uvCoordinates[meshInfo.vertexOffset + j].y = 0.0f;
						}
					}
				}
			}
			// load all textures:
			{
				textures.reserve(diffuseTextures.size());
				for (size_t i = 0; i < diffuseTextures.size(); ++i) {
					auto& path = diffuseTextures[i];
					if (path.empty()) {
						warn("path is empty!");
					}
					else if (path[0] == '*') {
						// Is embedded
						unsigned int texIndex = std::stoi(path.substr(1));
						const aiTexture* embeddedTex = scene->mTextures[texIndex];
						if (embeddedTex->mHeight == 0) {
							// is compressed
							textures.emplace_back((const uint8_t*)embeddedTex->pcData, embeddedTex->mWidth);
						} else {
							// is uncompressed
							textures.emplace_back((uint32_t)embeddedTex->mWidth, (uint32_t)embeddedTex->mHeight, (uint8_t*)embeddedTex->pcData);
						}
					} else {
						std::string texFilepath = GetDirectoryFromFilePath(filename) +'/' + path;
						textures.emplace_back(texFilepath.c_str());
					}
					assert(textures[i].GetWidth() != 0 && textures[i].GetHeight() != 0);
				}
			}
		}
		TraverseNode(this, scene->mRootNode);
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
}