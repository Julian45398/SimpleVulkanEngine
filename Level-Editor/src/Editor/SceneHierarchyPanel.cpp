#include "SceneHierarchyPanel.hpp"
#include <SGF.hpp>

namespace SGF {
    void SceneHierarchyPanel::AddNodeSelection(uint32_t modelIndex, uint32_t nodeIndex) {
        auto it = m_SelectedModels.find(modelIndex);
        if (it != m_SelectedModels.end()) {
            // Model already selected, add node index
            auto& updatedSelection = (*it).second;
            updatedSelection.insert(nodeIndex);
        } else {
            // Model not selected yet, create new selection
            m_SelectedModels.insert({modelIndex, {nodeIndex}});
        }
    }
    void SceneHierarchyPanel::AddSelection(uint32_t modelIndex) {
        const auto& model = m_Scene->GetModel(modelIndex);
        auto it = m_SelectedModels.find(modelIndex);
        if (it != m_SelectedModels.end()) {
            // Model already selected
            for (const auto& node : model.nodes) {
                (*it).second.insert(node.index);
            }
            return;
        }
        // In model selection mode, select the entire model
        std::set<uint32_t> allNodeIndices;
        for (const auto& node : model.nodes) {
            allNodeIndices.insert(node.index);
        }
        m_SelectedModels.insert({modelIndex, allNodeIndices});
    }

    void SceneHierarchyPanel::AddSelection(uint32_t modelIndex, uint32_t nodeIndex) {
        //auto it = m_SelectedModels.find(modelIndex);
        AddNodeSelection(modelIndex, nodeIndex);
        //if (m_SelectionMode == SelectionMode::MESH) {
        //} else if (m_SelectionMode == SelectionMode::MODEL) {
            //if (it != m_SelectedModels.end()) {
                //// Model already selected
                //return;
            //}
            //// In model selection mode, select the entire model
            //m_SelectedModels.insert(m_SelectedModels);
        //} else if (m_SelectionMode == SelectionMode::NODE_RECURSIVE) {
            //// In recursive node selection mode, select all child nodes
            //const auto& model = m_Scene->GetModel(modelIndex);
            //const auto& node = model.GetNode(nodeIndex);
            //std::function<void(const GenericModel::Node&)> selectRecursive = [&](const GenericModel::Node& n) {
                //AddNodeSelection(modelIndex, n.index);
                //for (uint32_t childIndex : n.children) {
                    //selectRecursive(model.GetNode(childIndex));
                //}
            //};
            //selectRecursive(node);
        //}
    }
    void SceneHierarchyPanel::Deselect(uint32_t modelIndex) {
        m_SelectedModels.erase(modelIndex);
    }
    void SceneHierarchyPanel::Deselect(uint32_t modelIndex, uint32_t nodeIndex) {
        auto it = m_SelectedModels.find(modelIndex);
        if (it != m_SelectedModels.end()) {
            auto& updatedSelection = (*it).second;
            updatedSelection.erase(nodeIndex);
            if (updatedSelection.empty()) {
                m_SelectedModels.erase(it);
            }
        }
    }
    void SceneHierarchyPanel::Draw() {
        ImGui::Begin("Scene Hierarchy");
        if (m_Scene) {
            for (uint32_t i = 0; i < m_Scene->GetModelCount(); ++i) {
                const auto& model = m_Scene->GetModel(i);
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesToNodes;
                bool isSelected = IsSelected(i);
                if (isSelected) { flags |= ImGuiTreeNodeFlags_Selected; }
                bool open = ImGui::TreeNodeEx((void*)(uintptr_t)i, flags, "Model %d: %s", i, model.GetName().c_str());
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                    if (isSelected) {
                        // Deselect
                        Deselect(i);
                    } else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
                        // Select
                        AddSelection(i);
                    } else {
                        ClearSelection();
                        AddSelection(i);
                    }
                }
                if (open) {
                    DrawModelNode(i, model.GetRoot());
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }
    void SceneHierarchyPanel::DrawModelNode(uint32_t modelIndex, const GenericModel::Node& node) {
        auto& model = m_Scene->GetModel(modelIndex);
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_OpenOnArrow;
		if (node.children.size() == 0) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
        bool isSelected = IsSelected(modelIndex, node);
        if (isSelected) { flags |= ImGuiTreeNodeFlags_Selected; }
		bool open = ImGui::TreeNodeEx(&node, flags, "%s", node.name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			//ImGui::IsItemToggledSelection
            if (isSelected) {
                // Deselect
                Deselect(modelIndex, node.index);
            } else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
                // Select
                AddSelection(modelIndex, node.index);
            } else {
                ClearSelection();
                AddSelection(modelIndex, node.index);
            }
		}
		if (open) {
			// Draw Subnodes:
			for (uint32_t child : node.children)
				DrawModelNode(modelIndex, model.nodes[child]);
			// Draw Meshes:
			//for (auto& m : node.meshes) {
				//ImGuiTreeNodeFlags mflags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				//ImGui::TreeNodeEx(&m, mflags, "Mesh %d", m);
				//if (ImGui::IsItemClicked()) {
					//info("Mesh clicked");
				//}
			//}
			ImGui::TreePop();
		}
    }
}