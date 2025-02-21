#include "SVE_Model.h"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>


struct GLTFData {
	tinygltf::Accessor acc;
	tinygltf::BufferView view;
	tinygltf::Buffer buf;
	void fromAccessor(int accessor, const tinygltf::Model& model) {
		assert(0 <= accessor);
		acc = model.accessors[accessor];
		view = model.bufferViews[acc.bufferView];
		buf = model.buffers[view.buffer];
	}
};

void parseNode(const tinygltf::Model& gltf, const tinygltf::Node& n, SveModel& model, const glm::mat4& base_transform) {
	glm::mat4 transform(1.0f);

	{
		glm::mat4 translation(1.0f);

		glm::mat4 rot(1.0f);

		glm::mat4 scale(1.0f);

		if (n.matrix.size() == 16) {
			for (uint32_t j = 0; j < 4; j++) {
				for (uint32_t k = 0; k < 4; k++) {
					transform[j][k] = (float)n.matrix[4 * j + k];
				}
			}
		}
		if (n.translation.size() == 3) {
			translation[3][0] = (float)n.translation[0];
			translation[3][1] = (float)n.translation[1];
			translation[3][2] = (float)n.translation[2];
			transform = transform * translation;
		}
		if (n.rotation.size() == 4) {
			auto qt = n.rotation;
			glm::vec4 q = glm::normalize(glm::vec4(qt[0], qt[1], qt[2], qt[3]));
			//double q[] = { 0.259, 0.0f, 0.0f, 0.966 };


			rot[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
			rot[0][1] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
			rot[0][2] = (float)(2.0 * (q[0] * q[2] - q[1] * q[3]));

			rot[1][0] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
			rot[1][1] = (float)(1.0 - 2.0 * (q[0] * q[0] + q[2] * q[2]));
			rot[1][2] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));

			rot[2][0] = (float)((double)2.0 * (q[0] * q[2] + q[1] * q[3]));
			rot[2][1] = (float)((double)2.0 * (q[1] * q[2] - q[0] * q[3]));
			rot[2][2] = (float)((double)1.0 - 2.0 * (q[0] * q[0] + q[1] * q[1]));

			transform = transform * rot;
		}
		if (n.scale.size() == 3) {
			scale[0][0] = (float)n.scale[0];
			scale[1][1] = (float)n.scale[1];
			scale[2][2] = (float)n.scale[2];
			transform = transform * scale;
		}

		transform = translation * rot * scale;
		transform = base_transform * transform;
	}
	
	// Check if node has a mesh
	if (n.mesh >= 0) {
		uint32_t vertex_count = 0;
		uint32_t index_count = 0;


		model.meshes.push_back({});
		Mesh& modelMesh = model.meshes.back();
		modelMesh.instanceTransforms.push_back(transform);

		auto mesh = gltf.meshes[n.mesh];

	
		//uint32_t index_offset = (uint32_t)modelMesh.vertices.size();
		for (size_t j = 0; j < mesh.primitives.size(); ++j) {
			uint32_t vertex_count = modelMesh.vertices.size();
			uint32_t index_count = modelMesh.indices.size();
	
			auto p = mesh.primitives[j];
			p.material;
			auto& material = gltf.materials[p.material];
			int base_texture_index = material.pbrMetallicRoughness.baseColorTexture.texCoord;
			int texture_index = material.pbrMetallicRoughness.baseColorTexture.index;
			bool has_texture = true;
			if (base_texture_index < 0 || texture_index < 0) {
				has_texture = false;
				shl::logWarn("doesnt have a texture!");
			}
			std::string tex_coord = std::string("TEXCOORD_" + std::to_string(base_texture_index));
			shl::logInfo("Tex coord: ", tex_coord);

			// vertices:
			GLTFData pos, normal, uv;
			if (p.attributes.find("POSITION") == p.attributes.end()) {
				shl::logWarn("Primitive: ", j, " of mesh: ", n.mesh, " is missing required attribute: POSITION!");
				continue;
			} else if (p.attributes.find("NORMAL") == p.attributes.end()) {
				shl::logWarn("Primitive: ", j, " of mesh: ", n.mesh, " is missing required attribute: NORMAL!");
				continue;
			} if (p.attributes.find(tex_coord) == p.attributes.end()) {
				has_texture = false;
				shl::logWarn("doesnt have a texture no tex_coord!");
			}

			{
				pos.fromAccessor(p.attributes.at("POSITION"), gltf);
				normal.fromAccessor(p.attributes.at("NORMAL"), gltf);
				if (has_texture) {
					uv.fromAccessor(p.attributes.at(tex_coord), gltf);

					modelMesh.imageIndex = material.pbrMetallicRoughness.baseColorTexture.index;
					assert(uv.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
					assert(uv.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					assert(uv.acc.type == TINYGLTF_TYPE_VEC2);
					assert(uv.acc.count == normal.acc.count);
				}
				else {
					modelMesh.imageIndex = UINT32_MAX;
				}
				assert(pos.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
				assert(pos.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(pos.acc.type == TINYGLTF_TYPE_VEC3);

				assert(normal.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
				assert(normal.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(normal.acc.type == TINYGLTF_TYPE_VEC3);

				assert(pos.acc.count == normal.acc.count);

				modelMesh.vertices.resize(pos.acc.count + vertex_count);
				for (size_t k = 0; k < pos.acc.count; ++k) {
					SveModelVertex vertex{};
					memcpy(&vertex.position, &pos.buf.data[pos.acc.byteOffset + pos.view.byteOffset + k * pos.view.byteStride], sizeof(vertex.position));
					memcpy(&vertex.normal, &normal.buf.data[normal.acc.byteOffset + normal.view.byteOffset + k * normal.view.byteStride], sizeof(vertex.normal));
					if (has_texture) {
						memcpy(&vertex.uvCoord, &uv.buf.data[uv.acc.byteOffset + uv.view.byteOffset + k * uv.view.byteStride], sizeof(vertex.uvCoord));
					}
					else {
						vertex.uvCoord = glm::vec2(0.f);
					}
					vertex.imageIndex = texture_index;
					modelMesh.vertices[k + vertex_count] = vertex;
				}
			}

			if (p.indices < 0) {
				shl::logWarn("no indexed geometry!");
				modelMesh.indices.resize(index_count + pos.acc.count);
				for (size_t k = 0; k < pos.acc.count; ++k) {
					modelMesh.indices[k + index_count] = k + vertex_count;
				}
			}
			// Indices:
			else {
				auto& indices = gltf.accessors[p.indices];
				assert(indices.type == TINYGLTF_TYPE_SCALAR);
				assert(indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT || indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
				auto& view = gltf.bufferViews[indices.bufferView];
				// buffer view must target index buffer
				assert(view.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
				auto buf = gltf.buffers[view.buffer];
				modelMesh.indices.resize(indices.count + index_count);
				for (size_t k = 0; k < indices.count; ++k) {
					uint32_t value;
					int comp_type = indices.componentType;
					if (comp_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						uint16_t s;
						memcpy(&s, &buf.data[indices.byteOffset + view.byteOffset + k * indices.ByteStride(view)], indices.ByteStride(view));
						value = s;
					}
					else if (comp_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						memcpy(&value, &buf.data[indices.byteOffset + view.byteOffset + k * sizeof(uint32_t)], sizeof(value));
					}
					else {
						shl::logFatal("expected int or short for indices!");
					}
					modelMesh.indices[k + index_count] = value + vertex_count;
				}
			}
		}
	}
	for (size_t i = 0; i < n.children.size(); ++i) {
		auto child_node = gltf.nodes[n.children[i]];
		parseNode(gltf, child_node, model, transform);
	}
}

bool loadGLTF(const char* filename, tinygltf::Model* model) {
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;

	bool ret = loader.LoadASCIIFromFile(model, &err, &warn, filename);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

	if (!warn.empty()) {
		shl::logWarn(warn.c_str());
		return false;
	}

	if (!err.empty()) {
		shl::logError(err.c_str());
		return false;
	}

	if (!ret) {
		shl::logError("Failed to parse GLTF: ", filename);
		return false;
	}
	shl::logInfo("GLTF loaded!");
	return true;
}

SveModel::SveModel(const char* filename) {
	shl::logInfo("loading file: ", filename);
	tinygltf::Model gltf;
	shl::Timer timer;
	if (!loadGLTF(filename, &gltf)) {
		shl::logError("failed to load GLTF file: ", filename);
		return;
	}
	shl::logInfo("model file loading: ", timer.ellapsedMillis(), "ms");
	shl::logInfo("constructing model...");
	images.resize(gltf.images.size());
	for (uint32_t i = 0; i < gltf.textures.size(); ++i) {
		images[i].height = gltf.images[i].height;
		images[i].width = gltf.images[i].width;
		images[i].pixels = gltf.images[i].image;
	}

	auto& mainScene = gltf.scenes[gltf.defaultScene];

	for (size_t i = 0; i < mainScene.nodes.size(); ++i) {
		assert(mainScene.nodes[i] >= 0);
		auto n = gltf.nodes[mainScene.nodes[i]];
		glm::mat4 transform(1.0f);
		parseNode(gltf, n, *this, transform);
	}
	double ellapsed_time = timer.ellapsedMillis();
	shl::logInfo("finished model loading! (took: ", ellapsed_time, "ms)");
	size_t total_vertex_count = 0;
	size_t total_index_count = 0;
	shl::logInfo("Model mesh count: ", meshes.size());
	for (size_t i = 0; i < meshes.size(); ++i) {
		total_vertex_count += meshes[i].vertices.size();
		total_index_count += meshes[i].indices.size();
		shl::logInfo("Mesh: ", i, "vertex count: ", meshes[i].vertices.size(), " index count: ", meshes[i].indices.size());
	}
	shl::logDebug("total vertex count: ", total_vertex_count);
	shl::logDebug("total index count: ", total_index_count);
}
