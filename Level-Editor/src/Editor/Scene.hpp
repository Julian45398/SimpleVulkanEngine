#pragma once

#include "Model.hpp"

namespace SGF {
    class Scene {
    public:
        inline Scene(const char* name) : m_Name(name) {};
        inline Scene(const std::string& name) : m_Name(name) {};
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        inline const std::string& GetName() const { return m_Name; }
        inline void AddModel(const GenericModel& model) { models.push_back(model); }
        inline void RemoveModel(size_t index) { models.erase(models.begin() + index); }
        inline void ClearModels() { models.clear(); }
        inline void ImportModel(const char* filename) { models.emplace_back(filename); }
        inline const GenericModel& GetModel(size_t index) const { return models[index]; }
        inline GenericModel& GetModel(size_t index) { return models[index]; }
        inline std::vector<GenericModel>& GetModels() { return models; }
        inline const std::vector<GenericModel>& GetModels() const { return models; }
        inline size_t GetModelCount() const { return models.size(); }
    private:
        std::vector<GenericModel> models;
        std::string m_Name;
    };
}