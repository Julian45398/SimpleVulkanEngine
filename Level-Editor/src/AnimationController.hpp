#pragma once

#include <glm/glm.hpp>
#include "Model.hpp"

namespace SGF {
    class AnimationController {
    public:
        AnimationController(GenericModel& model);

        void PlayAnimation(const std::string& animationName, bool loop = true);
        void Update(float deltaTime);
        void GetBonePalette(std::vector<glm::mat4>& outPalette) const;

        inline bool IsAnimationPlaying() const { return currentAnimationIndex != UINT32_MAX; }
        inline float GetCurrentTime() const { return currentTime; }

    private:
        GenericModel& model;
        uint32_t currentAnimationIndex = UINT32_MAX;
        float currentTime = 0.0f;
        bool isLooping = true;

        glm::vec3 InterpolatePosition(const std::vector<GenericModel::KeyFrame>& keys, float time) const;
		inline glm::vec3 InterpolateScale(const std::vector<GenericModel::KeyFrame>& keys, float time) const { return InterpolatePosition(keys, time); }
        glm::quat InterpolateRotation(const std::vector<GenericModel::KeyFrame>& keys, float time) const;

        void UpdateBoneTransforms(const GenericModel::Animation& animation, float animationTime);
    };
}