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

float SveModel::Mesh::getNodeIntersection(const BVHNode& node, const SveRay& ray, float closest) {
	assert(closest >= 0.f);
	if (node.box.hasIntersection(ray)) {
		if (node.childIndex != 0) {
			// descend
			closest = getNodeIntersection(volumeHierarchy[node.childIndex], ray, closest);
			closest = getNodeIntersection(volumeHierarchy[node.childIndex+1], ray, closest);
		} else {
			// is root node -> get closest triangle intersection
			assert(node.indexCount % 3 == 0);
			assert(node.startIndex % 3 == 0);
			uint32_t end = node.startIndex + node.indexCount;
			for (uint32_t i = node.startIndex; i < end; i += 3) {
				float t = getTriangleIntersection(i + node.startIndex, ray);
				if (t >= 0.f && t < closest) {
					closest = t;
				}
			}
		}
	}
	else {
		shl::logDebug("no collision with bounding box!");
	}
	return closest;
}

float SveModel::Mesh::getTriangleIntersection(uint32_t startIndex, const SveRay& ray) {
    constexpr float epsilon = std::numeric_limits<float>::epsilon();
	constexpr float inf = std::numeric_limits<float>::infinity();
	const glm::vec3& a = vertices[indices[startIndex]].position;
	const glm::vec3& b = vertices[indices[startIndex+1]].position;
	const glm::vec3& c = vertices[indices[startIndex+2]].position;

    glm::vec3 edge1 = b - a;
    glm::vec3 edge2 = c - a;
    glm::vec3 ray_cross_e2 = glm::cross(ray.getDir(), edge2);
    float det = glm::dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return inf;    // This ray is parallel to this triangle.

    float inv_det = 1.0 / det;
    glm::vec3 s = ray.getOrigin() - a;
    float u = inv_det * glm::dot(s, ray_cross_e2);

    if ((u < 0 && glm::abs(u) > epsilon) || (u > 1 && glm::abs(u-1) > epsilon))
        return inf;

    glm::vec3 s_cross_e1 = glm::cross(s, edge1);
    float v = inv_det * glm::dot(ray.getOrigin(), s_cross_e1);

    if ((v < 0 && glm::abs(v) > epsilon) || (u + v > 1 && glm::abs(u + v - 1) > epsilon))
        return inf;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * glm::dot(edge2, s_cross_e1);

    if (t > epsilon) // ray intersection
    {
        return  t;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return inf;
}

float SveModel::Mesh::getIntersection(const SveRay& ray, float closest) {
	for (size_t i = 0; i < instanceTransforms.size(); ++i) {
		glm::mat4 inverse = glm::inverse(instanceTransforms[i]);
		glm::vec4 orig = inverse * glm::vec4(ray.getOrigin(), 1.f);
		glm::vec4 dir = inverse * glm::vec4(ray.getDir(), 1.f);
		SveRay inv_ray(glm::vec3(orig.x, orig.y, orig.z), glm::vec3(dir.x, dir.y, dir.z));
		float g = getNodeIntersection(volumeHierarchy[0], inv_ray, closest);
		closest = glm::min(g, closest);
	}
	return closest;
}
float SveModel::getIntersection(const SveRay& ray) {
	const auto inter = boundingBox.getIntersection(ray);
	return inter;
	shl::logInfo("Ray: \ndir: { ", ray.getDir().x, ", ", ray.getDir().y, ", ", ray.getDir().z, 
	" }\ninvDir: { ", ray.getInvDir().x, ", ", ray.getInvDir().y, ", ", ray.getInvDir().z,
	" }\nmin: { ", ray.getOrigin().x, ", ", ray.getOrigin().y, ", ", ray.getOrigin().z, " }");
	
	shl::logInfo("Model Bounding box: \nmax { ", boundingBox.max.x, ", ", boundingBox.max.y, ", ", boundingBox.max.z, 
	" }\nmin: { ", boundingBox.min.x, ", ", boundingBox.min.y, ", ", boundingBox.min.z, " }");
	float t = std::numeric_limits<float>::infinity();
	if (boundingBox.hasIntersection(ray)) {
		shl::logWarn("has intersection!!");
	}
	if (inter < 0.0f) {
		float g;
		for (size_t i = 0; i < meshes.size(); ++i) {
			g = meshes[i].getIntersection(ray, t);
			t = glm::min(g, t);
		}
	}
	else {
		shl::logInfo("No model intersection! \nmax: { x:", boundingBox.max.x, " y: ", boundingBox.max.y, " z: ", boundingBox.max.z, 
		" } \nmin: { x: ", boundingBox.min.x, " y: ", boundingBox.min.y, " z: ", boundingBox.min.z, " }");
	}
	return t;
}

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
		SveModel::Mesh& modelMesh = model.meshes.back();
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

void SveModel::Mesh::buildBVHChildren(const BVHNode& parent, uint32_t& nodeCount) {
	shl::logDebug("mesh indices count: ", indices.size());
	shl::logDebug("node count: ", nodeCount);
	shl::logDebug("index count: ", parent.indexCount);
	shl::logDebug("start index: ", parent.startIndex);
	shl::logDebug("max node count: ", volumeHierarchy.size());
	assert(parent.childIndex == nodeCount);
	assert(nodeCount <= volumeHierarchy.size());
	
	assert(parent.indexCount % 3 == 0);
	assert(parent.startIndex % 3 == 0);

	// find axis to split
	uint32_t split_axis = 0;
	float longest_distance = 0.f;
	for (uint32_t i = 0; i < 3; ++i) {
		float dist = parent.box.max[i] - parent.box.min[i];
		if (longest_distance < dist) {
			split_axis = i;
		}
	}
	
	// get triangle centers: 
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
		//std::cout << ", " << centers[i];
	}
	//std::cout << '\n';
	uint32_t median = triangle_count / 2;
	{
		// find median
		uint32_t target = triangle_count /2;
		uint32_t start = 0;
		uint32_t end = triangle_count - 1;
		uint32_t k = end;
		while (k != median) {
			//k = tryMedian(start, end):
			uint32_t l = start;
			uint32_t r = end - 1;
			const float& pivot = centers[end];
			//shl::logInfo("pivot value: ", pivot);
			//shl::logInfo("center count: ", centers.size());
			//shl::logInfo("start: ", start, " end: ", end);
			while (true) {
				while (centers[l] < pivot)
					l++;
				while (l < r && centers[r] >= pivot)
					r--;
				if (l < r) {
					//shl::logInfo("Values: ", centers[l], ", ", centers[r]);
					//shl::logInfo("Positions: ", l, ", ", r);
					assert(centers[l] > centers[r]);
					SWAP_INDICES(l, r);
					--r;
					++l;
				} else {
					assert(l <= end);
					//shl::logInfo("BREAKING....");
					//shl::logInfo("Positions: ", l, ", ", r);
					//shl::logInfo("Values: ", centers[l], ", ", pivot);
					assert(pivot <= centers[l]);
					if (l < end) {
						SWAP_INDICES(l, end);
					}
					k = l;
					if (k < target) {
						start = k+1;
					} else
						end = k-1;
					break;
				}
			}
		}
	}
	// child1: 
	BVHNode& child1 = volumeHierarchy[nodeCount];
	BVHNode& child2 = volumeHierarchy[nodeCount+1];
	nodeCount += 2;
	child1.startIndex = parent.startIndex;
	child1.indexCount = median * 3;
	child1.childIndex = 0;
	child2.startIndex = child1.startIndex + child1.indexCount;
	child2.indexCount = parent.indexCount - child1.indexCount;
	child2.childIndex = 0;

	// child1 box:
	shl::logDebug("child1: { start index: ", child1.startIndex, ", index count: ", child1.indexCount, ", child index: ", child1.childIndex, '}');
	for (uint32_t i = child1.startIndex; i < child1.startIndex + child1.indexCount; ++i) {
		child1.box.max.x = std::max(vertices[indices[i]].position.x, child1.box.max.x);
		child1.box.max.y = std::max(vertices[indices[i]].position.y, child1.box.max.y);
		child1.box.max.z = std::max(vertices[indices[i]].position.z, child1.box.max.z);
		child1.box.min.x = std::min(vertices[indices[i]].position.x, child1.box.min.x);
		child1.box.min.y = std::min(vertices[indices[i]].position.y, child1.box.min.y);
		child1.box.min.z = std::min(vertices[indices[i]].position.z, child1.box.min.z);
	}

	// child2 box:
	shl::logDebug("child2: { start index: ", child2.startIndex, ", index count: ", child2.indexCount, ", child index: ", child2.childIndex, '}');
	for (uint32_t i = child2.startIndex; i < child2.startIndex + child2.indexCount; ++i) {
		child2.box.max.x = std::max(vertices[indices[i]].position.x, child2.box.max.x);
		child2.box.max.y = std::max(vertices[indices[i]].position.y, child2.box.max.y);
		child2.box.max.z = std::max(vertices[indices[i]].position.z, child2.box.max.z);
		child2.box.min.x = std::min(vertices[indices[i]].position.x, child2.box.min.x);
		child2.box.min.y = std::min(vertices[indices[i]].position.y, child2.box.min.y);
		child2.box.min.z = std::min(vertices[indices[i]].position.z, child2.box.min.z);
	}
	if (MAX_LEAF_TRIANGLES * 3 < child1.indexCount) {
		shl::logDebug("first child");
		child1.childIndex = nodeCount;
		buildBVHChildren(child1, nodeCount);
		child2.childIndex = nodeCount;
		shl::logDebug("second child");
		buildBVHChildren(child2, nodeCount);
	}
}

void SveModel::Mesh::buildBVH() {
	assert(indices.size() % 3 == 0);
	uint32_t triangle_count = indices.size() / 3;
	//uint32_t node_count = triangle_count / 6;
	uint32_t triangles = triangle_count;
	// getLayerCount:
	uint32_t max_node_count = 1;
	for (uint32_t i = 1; MAX_LEAF_TRIANGLES < triangles; ++i) {
		triangles = triangles / 2;
		max_node_count += (1 << i);
	}
	//uint32_t max_node_count = (1 << layer_count) + 1;
	shl::logInfo("Triangle count: ", triangle_count);
	shl::logInfo("node count: ", max_node_count);
	volumeHierarchy.resize(max_node_count);
	BVHNode& root = volumeHierarchy[0];

	root.childIndex = 1;
	root.indexCount = (uint32_t)indices.size();
	root.startIndex = 0;
	uint32_t node_count = 1;

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
	if (max_node_count > 1) {
		buildBVHChildren(root, node_count);
	} else {
		root.childIndex = 0;
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

	glm::vec4 p = meshes[0].instanceTransforms[0] * glm::vec4(meshes[0].vertices[0].position, 1.f);
	boundingBox.set(meshes[0].vertices[0].position);
	boundingBox.set(glm::vec3(p.x, p.y, p.z));
	for (size_t i = 0; i < meshes.size(); ++i) {
		meshes[i].buildBVH();
		for (size_t j = 0; j < meshes[i].instanceTransforms.size(); ++j) {
			for (size_t k = 0; k < meshes[k].vertices.size(); ++k) {
				glm::vec4 p = meshes[i].instanceTransforms[j] * glm::vec4(meshes[i].vertices[k].position, 1.f);
				boundingBox.addPoint(glm::vec3(p.x, p.y, p.z));
			}
		}
		total_vertex_count += meshes[i].vertices.size();
		total_index_count += meshes[i].indices.size();
		shl::logInfo("Mesh: ", i, "vertex count: ", meshes[i].vertices.size(), " index count: ", meshes[i].indices.size());
	}
	shl::logDebug("total vertex count: ", total_vertex_count);
	shl::logDebug("total index count: ", total_index_count);
}
