#pragma once

#include "Scene.hpp"

namespace SGF {
    class SceneHierarchyPanel
    {
    public:
        enum class SelectionMode {
            NONE,
            MODEL,
            MESH,
            NODE_RECURSIVE
        };
        SceneHierarchyPanel() = default;
        void Draw();
        void AddSelection(uint32_t modelIndex);
        void Deselect(uint32_t modelIndex);
        void AddSelection(uint32_t modelIndex, uint32_t nodeIndex);
        void Deselect(uint32_t modelIndex, uint32_t nodeIndex);

        //inline void SetSelectionMode(SelectionMode mode) { ClearSelection(); m_SelectionMode = mode; }
        inline void SetScene(const Scene* pScene) { ClearSelection(); m_Scene = pScene; }
        inline std::map<uint32_t, std::set<uint32_t>>& GetCurrentSelection() { return m_SelectedModels; }
        inline const std::map<uint32_t, std::set<uint32_t>>& GetCurrentSelection() const { return m_SelectedModels; }
        inline bool HasSelection() const { return !m_SelectedModels.empty(); }
        inline void ClearSelection() { m_SelectedModels.clear(); }

        inline bool IsSelected(uint32_t modelIndex) const { return m_SelectedModels.count(modelIndex); }
        inline bool IsSelected(uint32_t modelIndex, uint32_t nodeIndex) const { 
            auto it = m_SelectedModels.find(modelIndex); 
            return (it != m_SelectedModels.end()) && (*it).second.find(nodeIndex) != (*it).second.end();
        }
        inline bool IsSelected(uint32_t modelIndex, const GenericModel::Node& node) const { return IsSelected(modelIndex, node.index); }
        inline bool IsSelected(const GenericModel& model, const GenericModel::Node& node) const { 
            auto it = std::find(m_Scene->GetModels().begin(), m_Scene->GetModels().end(), model);
            size_t modelIndex = std::distance(m_Scene->GetModels().begin(), it);
            return IsSelected(modelIndex, node); 
        }
    private:
        std::map<uint32_t, std::set<uint32_t>> m_SelectedModels;
        //SelectionMode m_SelectionMode = SelectionMode::NONE;
        const Scene* m_Scene = nullptr;
    private:
        void AddNodeSelection(uint32_t modelIndex, uint32_t nodeIndex);
        void DrawModelNode(uint32_t modelIndex, const GenericModel::Node& node);
        //void DrawModelMesh(const GenericModel& model, const GenericModel::Node& node, const GenericModel::Mesh& mesh);
        // Functions
    };
}