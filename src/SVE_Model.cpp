#include "SVE_Model.h"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"


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
		shl::logInfo("transformation matrix found");
		for (uint32_t j = 0; j < 4; j++) {
			for (uint32_t k = 0; k < 4; k++) {
				transform[j][k] = n.matrix[4 * j + k];
			}
		}
	}
	if (n.translation.size() == 3) {
		shl::logDebug("translation found");
		translation[3][0] = n.translation[0];
		translation[3][1] = n.translation[1];
		translation[3][2] = n.translation[2];
		transform = transform * translation;
	}
	if (n.rotation.size() == 4) {
		auto qt = n.rotation;
		glm::vec4 q = glm::normalize(glm::vec4(qt[0], qt[1], qt[2], qt[3]));
		//double q[] = { 0.259, 0.0f, 0.0f, 0.966 };
		shl::logDebug("quaternion: [", q.x, ", ", q.y, ", ", q.z, ", ", q.w, "]");


		rot[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
		rot[0][1] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
		rot[0][2] = (float)(2.0 * (q[0] * q[2] - q[1] * q[3]));

		rot[1][0] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
		rot[1][1] = (float)(1.0 - 2.0 * (q[0] * q[0] + q[2] * q[2]));
		rot[1][2] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));

		rot[2][0] = (float)((double)2.0 * (q[0] * q[2] + q[1] * q[3]));
		rot[2][1] = (float)((double)2.0 * (q[1] * q[2] - q[0] * q[3]));
		rot[2][2] = (float)((double)1.0 - 2.0 * (q[0] * q[0] + q[1] * q[1]));

		shl::logInfo("rotation matrix: ");
		shl::logDebug("[", rot[0][0], ", ", rot[1][0], ", ", rot[2][0], ", ", rot[3][0], "]");
		shl::logDebug("[", rot[0][1], ", ", rot[1][1], ", ", rot[2][1], ", ", rot[3][1], "]");
		shl::logDebug("[", rot[0][2], ", ", rot[1][2], ", ", rot[2][2], ", ", rot[3][2], "]");
		shl::logDebug("[", rot[0][3], ", ", rot[1][3], ", ", rot[2][3], ", ", rot[3][3], "]");

		transform = transform * rot;
	}
	if (n.scale.size() == 3) {
		shl::logDebug("scale found!");
		scale[0][0] = (float)n.scale[0];
		scale[1][1] = (float)n.scale[1];
		scale[2][2] = (float)n.scale[2];
		transform = transform * scale;
	}

	transform = translation * rot * scale;
	transform = base_transform * transform;

	if (n.mesh >= 0) {
		shl::logDebug("mesh found!");

		auto mesh = gltf.meshes[n.mesh];
		uint32_t index_offset = model.vertices.size();
		for (size_t j = 0; j < mesh.primitives.size(); ++j) {
			auto p = mesh.primitives[j];
			for (auto const& [key, val] : p.attributes)
			{
				shl::logInfo("key: ", key, " value: ", val);
			}

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
				shl::logDebug("vertex count: ", pos.acc.count, " pos buffer size: ", pos.view.byteLength);

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
				shl::logDebug("index count: ", indices.count, " buf size: ", view.byteLength);
				for (size_t k = 0; k < indices.count; ++k) {
					uint32_t value;
					int comp_type = indices.componentType;
					if (comp_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						uint16_t s;
						memcpy(&s, &buf.data[indices.byteOffset + view.byteOffset + k * indices.ByteStride(view)], indices.ByteStride(view));
						value = s;
					}
					else if (comp_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						shl::logDebug("type is a int with stride: ", indices.ByteStride(view));
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
		shl::logError("Failed to parse glTF: ", filename);
		return false;
	}
	shl::logInfo("GLTF loaded!");
	return true;
}
SveModel::SveModel() {
	imageHeight = 0;
	imageWidth = 0;
}

SveModel::SveModel(const char* filename) {
	tinygltf::Model gltf;
	if (!loadGLTF(filename, &gltf)) {
		shl::logError("failed to load model: ", filename);
		return;
	}
	uint32_t index_offset = 0;
	auto texture = gltf.textures[0];
	auto image = gltf.images[0];
	shl::logInfo("Bits per channel: ", image.bits);
	imageWidth = image.width;
	imageHeight = image.height;
	pixelData = image.image;

	for (size_t i = 0; i < gltf.scenes[0].nodes.size(); ++i) {
		assert(gltf.scenes[0].nodes[i] >= 0);
		auto n = gltf.nodes[gltf.scenes[0].nodes[i]];
		glm::mat4 transform(1.0f);
		parseNode(gltf, n, *this, transform);
	}
	shl::logInfo("finished model loading!");
	shl::logDebug("vertex count: ", vertices.size());
	shl::logDebug("index count: ", indices.size());
}