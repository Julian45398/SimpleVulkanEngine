#include "SVE_Model.h"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

void AABB::includePoint(const glm::vec3& point) {
	if (point.x < min.x)
		min.x = point.x;
	if (point.y < min.y)
		min.y = point.y;
	if (point.z < min.z)
		min.z = point.z;
	if (point.x > max.x)
		min.x = point.x;
	if (point.y > max.y)
		min.y = point.y;
	if (point.z > max.z)
		min.z = point.z;
}

void AABB::setNewStartingPoint(const glm::vec3& point) {
	min = point;
	max = point;
}
glm::vec2 AABB::getIntersection(const Ray& ray) {
	glm::vec3 tMin = (min - ray.origin) / ray.direction;
	glm::vec3 tMax = (max - ray.origin) / ray.direction;
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
	return glm::vec2(tNear, tFar);
}

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
		if (modelMesh.indices.size() % 3 != 0) {
			shl::logWarn("Indices should be a multiple of three - failed to load mesh!");
			model.meshes.pop_back();
		}
		else {
			// building BVH:
			{
				uint32_t triangle_count = modelMesh.indices.size() / 3;
				//uint32_t node_count = triangle_count / 6;
				uint32_t layer_count = 0;
				uint32_t triangles = triangle_count;
				while (10 < triangles) {
					triangles = triangles / 2;
					layer_count++;
				}
				uint32_t node_count = 1 << layer_count;
				shl::logInfo("Triangle count: ", triangle_count);
				shl::logInfo("layer count: ", layer_count);
				shl::logInfo("node count: ", node_count);
				modelMesh.volumeHierarchy.resize(node_count);
				for (uint32_t i = 0; i < layer_count; ++i) {
				}
			}
		}

	}
	for (size_t i = 0; i < n.children.size(); ++i) {
		auto child_node = gltf.nodes[n.children[i]];
		parseNode(gltf, child_node, model, transform);
	}
}

std::vector<uint32_t> TEST_ARRAY = {
	3,3,22,4,12,4,124,125,44,4,55,5,112,7,8,55,64,35,78,123,15,5,1,7,8,9,5,34,65,77,33,2,1,14,13
};

template<typename T>
void printArray(std::vector<T> toPrint) {

	std::cout << "[ " << toPrint[0];
	for (size_t i = 1; i < toPrint.size(); ++i) {
		std::cout << ", " << toPrint[i];
	}
	std::cout << " ]\n";
}

#define MAX_LEAF_TRIANGLES 10

#define SWAP_INDICES(LEFT, RIGHT) do{ \
	uint32_t* left_indices = &indices[LEFT * 3]; \
	uint32_t* right_indices = &indices[RIGHT * 3]; \
	std::swap(centers[LEFT], centers[RIGHT]); \
	std::swap(highest[LEFT], highest[RIGHT]); \
	std::swap(lowest[LEFT], lowest[RIGHT]); \
	std::swap(left_indices[0], right_indices[0]); \
	std::swap(left_indices[1], right_indices[1]); \
	std::swap(left_indices[2], right_indices[2]); \
	} while(0)

void Mesh::buildBVHChildren(BVHNode parent) {
	shl::logDebug("index count: ", parent.indexCount);
	shl::logDebug("start index: ", parent.startIndex);
	BVHNode& child1 = volumeHierarchy.emplace_back();
	BVHNode& child2 = volumeHierarchy.emplace_back();
	assert(parent.indexCount % 3 == 0);
	assert(parent.startIndex % 3 == 0);

	// find axis to split
	shl::logDebug("find axis to split");
	uint32_t split_axis = 0;
	float longest_distance = 0.f;
	for (uint32_t i = 0; i < 3; ++i) {
		float dist = parent.box.max[i] - parent.box.min[i];
		if (longest_distance < dist) {
			split_axis = i;
		}
	}
	
	// get triangle centers: 
	shl::logDebug("get triangle centers");
	uint32_t triangle_count = parent.indexCount / 3;
	std::vector<float> centers(triangle_count);
	std::vector<float> highest(triangle_count);
	std::vector<float> lowest(triangle_count);
	for (uint32_t i = 0; i < triangle_count; ++i) {
		uint32_t j = i * 3;
		const float first = vertices[indices[j + parent.startIndex]].position[split_axis];
		const float second = vertices[indices[j + parent.startIndex + 1]].position[split_axis];
		const float third = vertices[indices[j + parent.startIndex + 2]].position[split_axis];
		centers[i] = (first + second + third) * (1.f / 3.f);
		lowest[i] = std::min(first, std::min(second, third));
		highest[i] = std::max(first, std::max(second, third));
		std::cout << ", " << centers[i];
	}
	std::cout << '\n';
	uint32_t median = triangle_count / 2;
	shl::logDebug("median: ", median);

	{
		uint32_t sorting_point = triangle_count;
		uint32_t last = triangle_count - 1;
		uint32_t first = 0;
		while (sorting_point != median) {
			// partition algorithm:
			size_t right = last;
			size_t left = first;
			size_t pivot = shl::pickPivot(centers.data(), first, last);
			SWAP_INDICES(pivot, last);
			while (true) {
				while (left < right && centers[right] > centers[pivot]) {
					--right;
				}
				while (left < right && centers[left] < centers[pivot]) {
					++left;
				}

				if (left < right) {
					shl::logDebug("swapping", left, ", ", right);
					// swap left - right:
					SWAP_INDICES(left, right);
				}
				else {
					// swap left - last:
					SWAP_INDICES(left, last);
					break;
				}
			}
			sorting_point = left;
			if (sorting_point < median)
				first = sorting_point;
			else
				last = sorting_point;

			shl::logDebug("sorting point: ", sorting_point, " median: ", median, " first: ", first, " last: ", last);
		}
	}
	// child1: 
	child1.startIndex = parent.startIndex;
	child1.indexCount = median * 3;
	child1.childIndex = volumeHierarchy.size();
	shl::logDebug("child1: { start index: ", child1.startIndex, ", index count: ", child1.indexCount, ", child index: ", child1.childIndex, '}');
	for (uint32_t i = child1.startIndex; i < child1.startIndex + child1.indexCount; ++i) {
		child1.box.max.x = std::max(vertices[indices[i]].position.x, child1.box.max.x);
		child1.box.max.y = std::max(vertices[indices[i]].position.y, child1.box.max.y);
		child1.box.max.z = std::max(vertices[indices[i]].position.z, child1.box.max.z);
		child1.box.min.x = std::min(vertices[indices[i]].position.x, child1.box.min.x);
		child1.box.min.y = std::min(vertices[indices[i]].position.y, child1.box.min.y);
		child1.box.min.z = std::min(vertices[indices[i]].position.z, child1.box.min.z);
	}
	if (MAX_LEAF_TRIANGLES * 3 < child1.indexCount) {
		buildBVHChildren(child1);
	}

	// child2:
	child2.startIndex = parent.startIndex + median * 3;
	child2.indexCount = parent.indexCount - median * 3;
	child2.childIndex = volumeHierarchy.size();
	shl::logDebug("child2: { start index: ", child1.startIndex, ", index count: ", child1.indexCount, ", child index: ", child1.childIndex, '}');
	for (uint32_t i = child2.startIndex; i < child2.startIndex + child2.indexCount; ++i) {
		child2.box.max.x = std::max(vertices[indices[i]].position.x, child2.box.max.x);
		child2.box.max.y = std::max(vertices[indices[i]].position.y, child2.box.max.y);
		child2.box.max.z = std::max(vertices[indices[i]].position.z, child2.box.max.z);
		child2.box.min.x = std::min(vertices[indices[i]].position.x, child2.box.min.x);
		child2.box.min.y = std::min(vertices[indices[i]].position.y, child2.box.min.y);
		child2.box.min.z = std::min(vertices[indices[i]].position.z, child2.box.min.z);
	}

	if (MAX_LEAF_TRIANGLES * 3 < child2.indexCount) {
		buildBVHChildren(child2);
	}
}

void Mesh::buildBVH() {
	assert(indices.size() % 3 == 0);
	uint32_t triangle_count = indices.size() / 3;
	//uint32_t node_count = triangle_count / 6;
	uint32_t layer_count = 0;
	uint32_t triangles = triangle_count;
	// getLayerCount:
	while (MAX_LEAF_TRIANGLES < triangles) {
		triangles = triangles / 2;
		layer_count++;
	}
	uint32_t node_count = (1 << layer_count) + 1;
	shl::logInfo("Triangle count: ", triangle_count);
	shl::logInfo("layer count: ", layer_count);
	shl::logInfo("node count: ", node_count);
	volumeHierarchy.reserve(node_count);
	BVHNode& root = volumeHierarchy.emplace_back();

	root.childIndex = 1;
	root.indexCount = (uint32_t)indices.size();
	root.startIndex = 0;

	shl::logInfo("index count: ", root.indexCount);
	shl::logInfo("start index: ", root.startIndex);
	for (size_t i = 0; i < vertices.size(); ++i) {
		root.box.max.x = std::max(vertices[indices[i]].position.x, root.box.max.x);
		root.box.max.y = std::max(vertices[indices[i]].position.y, root.box.max.y);
		root.box.max.z = std::max(vertices[indices[i]].position.z, root.box.max.z);

		root.box.min.x = std::min(vertices[indices[i]].position.x, root.box.min.x);
		root.box.min.y = std::min(vertices[indices[i]].position.y, root.box.min.y);
		root.box.min.z = std::min(vertices[indices[i]].position.z, root.box.min.z);
	}
	buildBVHChildren(root);
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
	shl::logInfo("TEST: ");
	shl::logInfo("Before sorting: ");
	printArray(TEST_ARRAY);
	size_t median = shl::quickMedian(TEST_ARRAY, 5);
	shl::logInfo("estimated median: ", TEST_ARRAY[median], " at position: ", median, " array size: ", TEST_ARRAY.size());
	shl::logInfo("After median: ");
	printArray(TEST_ARRAY);
	shl::quickSort(TEST_ARRAY);
	shl::logInfo("After sorting: ");
	printArray(TEST_ARRAY);
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
		meshes[i].buildBVH();
		total_vertex_count += meshes[i].vertices.size();
		total_index_count += meshes[i].indices.size();
		shl::logInfo("Mesh: ", i, "vertex count: ", meshes[i].vertices.size(), " index count: ", meshes[i].indices.size());
	}
	shl::logDebug("total vertex count: ", total_vertex_count);
	shl::logDebug("total index count: ", total_index_count);
}
