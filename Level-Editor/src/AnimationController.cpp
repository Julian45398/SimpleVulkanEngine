#include "AnimationController.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

namespace SGF {
    AnimationController::AnimationController(GenericModel& model) : model(model) {}

    void AnimationController::PlayAnimation(const std::string& animationName, bool loop) {
        for (size_t i = 0; i < model.animations.size(); ++i) {
            if (model.animations[i].name == animationName) {
                currentAnimationIndex = static_cast<uint32_t>(i);
                currentTime = 0.0f;
                isLooping = loop;
                return;
            }
        }
        Log::Warn("Animation '{}' not found!", animationName);
    }

    void AnimationController::Update(float deltaTime) {
        if (currentAnimationIndex == UINT32_MAX) return;

        const auto& animation = model.animations[currentAnimationIndex];
        currentTime += deltaTime * animation.ticksPerSecond;

        if (currentTime > animation.duration) {
            if (isLooping) {
                currentTime = std::fmod(currentTime, animation.duration);
            }
            else {
                currentAnimationIndex = UINT32_MAX;
                return;
            }
        }

        UpdateBoneTransforms(animation, currentTime);
    }

    void AnimationController::UpdateBoneTransforms(const GenericModel::Animation& animation, float animationTime) {
        // Reset all bones to identity
        for (auto& bone : model.bones) {
            bone.currentTransform = glm::mat4(1.0f);
        }

        // Apply animation channels
        for (const auto& channel : animation.channels) {
            if (channel.boneIndex >= model.bones.size()) continue;

            auto& bone = model.bones[channel.boneIndex];

            glm::vec3 position(0.0f);
            glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.0f);

            if (!channel.positionKeys.empty()) {
                position = InterpolatePosition(channel.positionKeys, animationTime);
            }
            if (!channel.rotationKeys.empty()) {
                rotation = InterpolateRotation(channel.rotationKeys, animationTime);
            }
            if (!channel.scaleKeys.empty()) {
                scale = InterpolateScale(channel.scaleKeys, animationTime);
            }

            // Build transform: Scale * Rotation * Translation
            bone.currentTransform = glm::translate(glm::mat4(1.0f), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
        }

        // Update parent-child hierarchy
        for (size_t i = 1; i < model.bones.size(); ++i) {
            auto& bone = model.bones[i];
            if (bone.parent != UINT32_MAX) {
                bone.currentTransform = model.bones[bone.parent].currentTransform * bone.currentTransform;
            }
        }
    }

    glm::vec3 AnimationController::InterpolatePosition(const std::vector<GenericModel::KeyFrame>& keys, float time) const {
        if (keys.empty()) return glm::vec3(0.0f);
        if (keys.size() == 1) return keys[0].value;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (time >= keys[i].time && time <= keys[i + 1].time) {
                float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
                return glm::mix(keys[i].value, keys[i + 1].value, t);
            }
        }
        return keys.back().value;
    }

    glm::quat AnimationController::InterpolateRotation(const std::vector<GenericModel::KeyFrame>& keys, float time) const {
        if (keys.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (keys.size() == 1) return glm::quat_cast(glm::mat4(1.0f)); // Placeholder

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (time >= keys[i].time && time <= keys[i + 1].time) {
                float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
                // Hier würde echte Quaternion-Interpolation stattfinden
                return glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), t);
            }
        }
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    void AnimationController::GetBonePalette(std::vector<glm::mat4>& outPalette) const {
        outPalette.clear();
        outPalette.reserve(model.bones.size());
        for (const auto& bone : model.bones) {
            outPalette.push_back(bone.currentTransform * bone.offsetMatrix);
        }
    }
}