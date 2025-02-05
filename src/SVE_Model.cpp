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

	if (n.mesh >= 0) {
		auto mesh = gltf.meshes[n.mesh];
		uint32_t index_offset = (uint32_t)model.vertices.size();
		for (size_t j = 0; j < mesh.primitives.size(); ++j) {
			auto p = mesh.primitives[j];

			// vertices:
			GLTFData pos, normal, uv;
			if (p.attributes.find("POSITION") == p.attributes.end()
				|| p.attributes.find("NORMAL") == p.attributes.end()
				|| p.attributes.find("TEXCOORD_0") == p.attributes.end()
				) {
				shl::logError("failed to find required vertex data!");
				continue;
			}
			else {
				pos.fromAccessor(p.attributes.at("POSITION"), gltf);
				normal.fromAccessor(p.attributes.at("NORMAL"), gltf);
				uv.fromAccessor(p.attributes.at("TEXCOORD_0"), gltf);
				assert(pos.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
				assert(pos.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(pos.acc.type == TINYGLTF_TYPE_VEC3);

				assert(normal.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
				assert(normal.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(normal.acc.type == TINYGLTF_TYPE_VEC3);

				assert(uv.view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
				assert(uv.acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(uv.acc.type == TINYGLTF_TYPE_VEC2);


				assert(pos.acc.count == uv.acc.count && uv.acc.count == normal.acc.count);

				for (size_t k = 0; k < pos.acc.count; ++k) {
					SveModelVertex vertex{};
					memcpy(&vertex.position, &pos.buf.data[pos.acc.byteOffset + pos.view.byteOffset + k * pos.view.byteStride], sizeof(vertex.position));
					memcpy(&vertex.normal, &normal.buf.data[normal.acc.byteOffset + normal.view.byteOffset + k * normal.view.byteStride], sizeof(vertex.normal));
					memcpy(&vertex.uvCoord, &uv.buf.data[uv.acc.byteOffset + uv.view.byteOffset + k * uv.view.byteStride], sizeof(vertex.uvCoord));
					glm::vec4 transform_pos = glm::vec4(vertex.position, 1.0f);
					glm::vec4 transformed_pos = transform * transform_pos;
					vertex.position.x = transformed_pos.x;
					vertex.position.z = transformed_pos.y;
					vertex.position.y = transformed_pos.z;
					model.vertices.push_back(vertex);
				}
			}

			if (p.indices < 0) {
				shl::logWarn("no indexed geometry!");
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
				model.indices.reserve(model.indices.size() + indices.count);
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
					value += index_offset;
					model.indices.push_back(value);
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
	if (!loadGLTF(filename, &gltf)) {
		shl::logError("failed to load GLTF file: ", filename);
		return;
	}
	shl::logInfo("constructing model...");
	shl::Timer timer;
	images.resize(gltf.images.size());
	for (uint32_t i = 0; i < gltf.textures.size(); ++i) {
		images[i].height = gltf.images[i].height;
		images[i].width = gltf.images[i].width;
		images[i].pixels = gltf.images[i].image;
	}
	uint32_t mainScene = gltf.defaultScene;

	for (size_t i = 0; i < gltf.scenes[mainScene].nodes.size(); ++i) {
		assert(gltf.scenes[mainScene].nodes[i] >= 0);
		auto n = gltf.nodes[gltf.scenes[mainScene].nodes[i]];
		glm::mat4 transform(1.0f);
		parseNode(gltf, n, *this, transform);
	}
	double ellapsed_time = timer.ellapsedMillis();
	shl::logInfo("finished model loading! (took: ", ellapsed_time, "ms)");
	shl::logDebug("vertex count: ", vertices.size());
	shl::logDebug("index count: ", indices.size());
}
