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
	void TraverseNode(GenericModel* pModel, aiNode* pNode, uint32_t parentIndex = 0);

	void BuildNode(GenericModel* pModel, aiNode* pNode, GenericModel::Node& node);

	void LoadSkeletalAnimations(GenericModel* pModel, const aiScene* pScene) {
		if (pScene->mNumAnimations == 0) {
			Log::Info("Model has no animations!");
			return;
		}

		pModel->animations.reserve(pScene->mNumAnimations);

		for (uint32_t animIndex = 0; animIndex < pScene->mNumAnimations; ++animIndex) {
			const aiAnimation* pAnim = pScene->mAnimations[animIndex];
			GenericModel::Animation animation;
			animation.name = pAnim->mName.C_Str();
			animation.duration = static_cast<float>(pAnim->mDuration);
			animation.ticksPerSecond = pAnim->mTicksPerSecond != 0.0 ? static_cast<float>(pAnim->mTicksPerSecond) : 25.0f;

			animation.channels.reserve(pAnim->mNumChannels);

			for (uint32_t channelIndex = 0; channelIndex < pAnim->mNumChannels; ++channelIndex) {
				const aiNodeAnim* pNodeAnim = pAnim->mChannels[channelIndex];

				// Find bone index by name
				uint32_t boneIndex = UINT32_MAX;
				for (uint32_t i = 0; i < pModel->bones.size(); ++i) {
					if (pModel->bones[i].name == pNodeAnim->mNodeName.C_Str()) {
						boneIndex = i;
						break;
					}
				}

				if (boneIndex == UINT32_MAX) {
					Log::Warn("Animation channel '{}' references unknown bone '{}'", pAnim->mName.C_Str(), pNodeAnim->mNodeName.C_Str());
					continue;
				}

				GenericModel::AnimationChannel channel;
				channel.boneIndex = boneIndex;

				// Load position keys
				channel.positionKeys.reserve(pNodeAnim->mNumPositionKeys);
				for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys; ++i) {
					const aiVectorKey& key = pNodeAnim->mPositionKeys[i];
					channel.positionKeys.emplace_back();
					channel.positionKeys.back().time = static_cast<float>(key.mTime);
					channel.positionKeys.back().value = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				// Load rotation keys (stored as quaternions)
				channel.rotationKeys.reserve(pNodeAnim->mNumRotationKeys);
				for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys; ++i) {
					const aiQuatKey& key = pNodeAnim->mRotationKeys[i];
					channel.rotationKeys.emplace_back();
					channel.rotationKeys.back().time = static_cast<float>(key.mTime);
					// Store quaternion as vec3 (temporary, should be quat)
					glm::quat rotation;
					rotation.x = key.mValue.x;
					rotation.y = key.mValue.y;
					rotation.z = key.mValue.z;
					rotation.w = key.mValue.w;
					channel.rotationKeys.back().value = rotation;
				}

				// Load scale keys
				channel.scaleKeys.reserve(pNodeAnim->mNumScalingKeys);
				for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys; ++i) {
					const aiVectorKey& key = pNodeAnim->mScalingKeys[i];
					channel.scaleKeys.emplace_back();
					channel.scaleKeys.back().time = static_cast<float>(key.mTime);
					channel.scaleKeys.back().value = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				animation.channels.push_back(channel);
			}

			pModel->animations.push_back(animation);
			Log::Info("Loaded animation '{}' with {} channels, duration: {:.2f}s",
				animation.name, animation.channels.size(), animation.duration);
		}
	}

	void LoadSkeletalBones(GenericModel* pModel, const aiScene* pScene) {
		if (pScene->mNumMeshes == 0) {
			Log::Info("Model has no meshes!");
			return;
		}

		// Map bone names to indices for quick lookup
		std::unordered_map<std::string, uint32_t> boneNameToIndex;

		// Collect all bones from all meshes
		for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex) {
			const aiMesh* pMesh = pScene->mMeshes[meshIndex];

			for (uint32_t boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex) {
				const aiBone* pBone = pMesh->mBones[boneIndex];
				std::string boneName = pBone->mName.C_Str();

				// Check if bone already added
				if (boneNameToIndex.find(boneName) == boneNameToIndex.end()) {
					uint32_t newBoneIndex = (uint32_t)(pModel->bones.size());
					boneNameToIndex[boneName] = newBoneIndex;

					GenericModel::Bone bone;
					bone.name = boneName;
					bone.index = newBoneIndex;
					bone.parent = UINT32_MAX;
					bone.currentTransform = glm::mat4(1.0f);

					// Store offset matrix (mesh-space to bone-space)
					for (uint32_t j = 0; j < 4; ++j) {
						for (uint32_t k = 0; k < 4; ++k) {
							bone.offsetMatrix[k][j] = pBone->mOffsetMatrix[j][k];
						}
					}

					pModel->bones.push_back(bone);
				}
			}
		}

		if (pModel->bones.empty()) {
			Log::Warn("Model has no bones!");
			return;
		}

		// Setup bone hierarchy from skeleton
		for (uint32_t boneIndex = 0; boneIndex < pModel->bones.size(); ++boneIndex) {
			const auto& boneName = pModel->bones[boneIndex].name;

			// Find bone node in armature
			std::function<aiNode* (aiNode*, const std::string&)> findBoneNode =
				[&](aiNode* node, const std::string& name) -> aiNode* {
				if (strncmp(node->mName.C_Str(), name.c_str(), name.size()) == 0) {
					return node;
				}
				for (uint32_t i = 0; i < node->mNumChildren; ++i) {
					auto* result = findBoneNode(node->mChildren[i], name);
					if (result) return result;
				}
				return nullptr;
				};

			aiNode* boneNode = findBoneNode(pScene->mRootNode, boneName);
			if (boneNode) {
				auto& bone = pModel->bones[boneIndex];
				for (uint32_t j = 0; j < 4; ++j) {
					for (uint32_t k = 0; k < 4; ++k) {
						bone.nodeTransform[k][j] = boneNode->mTransformation[j][k];
					}
				}
				if (boneNode->mParent) {
					const std::string parentName = boneNode->mParent->mName.C_Str();
					if (boneNameToIndex.find(parentName) != boneNameToIndex.end()) {
						bone.parent = boneNameToIndex[parentName];
					}
				}
			}
		}

		{
			std::vector<GenericModel::Bone> sortedBones;
			sortedBones.reserve(pModel->bones.size());
			std::unordered_map<uint32_t, uint32_t> oldToNewIndex;

			std::function<void(uint32_t)> addBoneRecursive = [&](uint32_t boneIdx) {
				if (oldToNewIndex.find(boneIdx) != oldToNewIndex.end()) {
					return; // Already added
				}

				// Add all ancestors first
				if (pModel->bones[boneIdx].parent != UINT32_MAX) {
					addBoneRecursive(pModel->bones[boneIdx].parent);
				}

				// Now add this bone
				oldToNewIndex[boneIdx] = (uint32_t)(sortedBones.size());
				sortedBones.push_back(pModel->bones[boneIdx]);
				sortedBones.back().index = (uint32_t)(sortedBones.size() - 1);
				};

			// Process all bones
			for (uint32_t i = 0; i < pModel->bones.size(); ++i) {
				addBoneRecursive(i);
			}

			// Update parent indices to reference the new ordering
			for (auto& bone : sortedBones) {
				if (bone.parent != UINT32_MAX) {
					bone.parent = oldToNewIndex[bone.parent];
				}
			}

			// Update animation channels to reference new bone indices
			for (auto& animation : pModel->animations) {
				for (auto& channel : animation.channels) {
					channel.boneIndex = oldToNewIndex[channel.boneIndex];
				}
			}
			// Update vertex weights to reference new bone indices
			for (auto& weight : pModel->vertexWeights) {
				for (int i = 0; i < 4; ++i) {
					if (weight.boneWeights[i] > 0.0f) {
						weight.boneIndices[i] = oldToNewIndex[weight.boneIndices[i]];
					}
				}
			}
			assert(sortedBones[0].parent == UINT32_MAX);
			sortedBones[0].currentTransform = sortedBones[0].nodeTransform;
			for (size_t i = 1; i < pModel->bones.size(); ++i) {
				assert(sortedBones[i].parent < i); // Ensure parents come before children
				sortedBones[i].currentTransform = sortedBones[sortedBones[i].parent].currentTransform * sortedBones[i].nodeTransform;
			}

			pModel->bones = std::move(sortedBones);
		}

		// Load vertex weights
		pModel->vertexWeights.resize(pModel->vertices.size());

		// Initialize weights to zero
		for (auto& weight : pModel->vertexWeights) {
			for (int i = 0; i < 4; ++i) {
				weight.boneIndices[i] = 0;
				weight.boneWeights[i] = 0.0f;
			}
		}

		// Load weights from meshes
		for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex) {
			const aiMesh* pMesh = pScene->mMeshes[meshIndex];
			const auto& meshData = pModel->meshes[meshIndex];

			for (uint32_t boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex) {
				const aiBone* pBone = pMesh->mBones[boneIndex];
				uint32_t globalBoneIndex = boneNameToIndex[pBone->mName.C_Str()];

				for (uint32_t weightIndex = 0; weightIndex < pBone->mNumWeights; ++weightIndex) {
					const aiVertexWeight& vWeight = pBone->mWeights[weightIndex];
					uint32_t vertexIndex = meshData.vertexOffset + vWeight.mVertexId;

					// Find first empty slot
					for (int i = 0; i < 4; ++i) {
						if (pModel->vertexWeights[vertexIndex].boneWeights[i] == 0.0f) {
							pModel->vertexWeights[vertexIndex].boneIndices[i] = globalBoneIndex;
							pModel->vertexWeights[vertexIndex].boneWeights[i] = vWeight.mWeight;
							break;
						}
					}
				}
			}
		}

		// Normalize weights
		for (auto& weight : pModel->vertexWeights) {
			float totalWeight = weight.boneWeights[0] + weight.boneWeights[1] +
				weight.boneWeights[2] + weight.boneWeights[3];
			if (totalWeight > 0.0f) {
				for (int i = 0; i < 4; ++i) {
					weight.boneWeights[i] /= totalWeight;
				}
			}
		}

		Log::Info("Loaded {} bones with vertex weights", pModel->bones.size());
	}

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
			Log::Warn("Texture is requested but path is empty!");
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
				if (hasTexture) Log::Warn("mesh has a texture but doesnt have uv coordinates!");
				for (uint32_t j = 0; j < pMesh->mNumVertices; ++j) {
					pModel->vertices[meshInfo.vertexOffset + j].uv.x = 2.0f;
					pModel->vertices[meshInfo.vertexOffset + j].uv.y = 2.0f;
				}
			}
			if (!hasBaseColor && !hasTexture && !hasVertexColors) Log::Warn("Mesh doesnt have any color information!");
		}
	}

	const GenericModel::Node* GenericModel::ImportModel(const char* filename) {
		Timer importTime;
		//Clear();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_GenBoundingBoxes | aiProcess_FlipUVs);
		auto assimpLoadTime = importTime.currentMillis();
		if (scene == nullptr) {
			SGF::Log::Error("No Scene available for file: {}", filename);
			return nullptr;
		}
		name = scene->mName.C_Str();

		const GenericModel::Node* pAttachmentNode = nullptr;
		
		auto* pRoot = scene->mRootNode;
		if (nodes.size() == 0) {
			// Get root from model:
			nodes.emplace_back();
			auto& root = nodes[0];
			root.parent = UINT32_MAX;
			root.name = pRoot->mName.Empty() ? "root" : pRoot->mName.C_Str();
			root.index = 0;
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
		LoadSkeletalBones(this, scene);
		LoadSkeletalAnimations(this, scene);

		SGF::Log::Info("Loading model file: {} finished, took: {} milliseconds, time for Assimp Scene: {}", filename, importTime.currentMillis(), assimpLoadTime);
		return pAttachmentNode;
	}
	void BuildNode(GenericModel* pModel, aiNode* pNode, GenericModel::Node& node) {
		// Copy transformation matrix
		glm::mat4 localTransform(1.f);
		for (uint32_t j = 0; j < 4; ++j) {
			for (uint32_t k = 0; k < 4; ++k) {
				localTransform[k][j] = pNode->mTransformation[j][k];
			}
		}
		if (node.parent != UINT32_MAX) {
			node.globalTransform = pModel->nodes[node.parent].globalTransform * localTransform;
		} else {
			node.globalTransform = localTransform;
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
		node.index = nodeIndex;
		pModel->nodes[parentIndex].children.push_back(nodeIndex);

		BuildNode(pModel, pNode, node);

		// Recurse into children
		node.children.reserve(pNode->mNumChildren);
		for (unsigned int i = 0; i < pNode->mNumChildren; ++i) {
			TraverseNode(pModel, pNode->mChildren[i], nodeIndex);
		}
	}

	// Apply a delta transform to the node and all its descendants
	void GenericModel::TransformNodeRecursive(GenericModel::Node& node, const glm::mat4& deltaTransform) {
		TransformNode(node, deltaTransform);
		for (uint32_t childIndex : node.children) {
			TransformNodeRecursive(nodes[childIndex], deltaTransform);
		}
	}
}