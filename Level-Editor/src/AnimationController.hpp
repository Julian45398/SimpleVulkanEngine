#pragma once

#include <glm/glm.hpp>
#include "Model.hpp"

namespace SGF {
    class AnimationController {
    public:
        AnimationController(GenericModel* model);

        void PlayAnimation(const std::string& animationName, bool loop = true);
        void Update(float deltaTime);
        void GetBonePalette(std::vector<glm::mat4>& outPalette) const;

        inline bool IsPlaying() const { return isPlaying; }
		inline void Stop() { currentAnimationIndex = UINT32_MAX; isPlaying = false; }
		inline void Pause() { isPlaying = false; }
		inline void Continue() { if (currentAnimationIndex != UINT32_MAX) isPlaying = true; }
		inline void SetLooping(bool loop) { isLooping = loop; }
        inline float GetCurrentTime() const { return currentTime; }
		inline GenericModel* GetModel() const { return pModel; }
        void SetCurrentTime(float time);

    private:
        GenericModel* pModel;
        uint32_t currentAnimationIndex = UINT32_MAX;
        float currentTime = 0.0f;
        bool isLooping = true;
		bool isPlaying = false;

        glm::vec3 InterpolatePosition(const std::vector<GenericModel::KeyFrame>& keys, float time) const;
		inline glm::vec3 InterpolateScale(const std::vector<GenericModel::KeyFrame>& keys, float time) const { return InterpolatePosition(keys, time); }
        glm::quat InterpolateRotation(const std::vector<GenericModel::RotationKeyFrame>& keys, float time) const;

        void UpdateBoneTransforms(const GenericModel::Animation& animation, float animationTime);
    };
}