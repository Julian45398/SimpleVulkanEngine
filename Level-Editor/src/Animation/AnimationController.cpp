#include "AnimationController.hpp"
#include "SGF.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

namespace SGF {
    AnimationController::AnimationController(GenericModel* model) : pModel(model) {
		assert(pModel);
    }

    void AnimationController::PlayAnimation(const std::string& animationName, bool loop) {
        assert(pModel);
		auto& model = *pModel;
        for (size_t i = 0; i < model.animations.size(); ++i) {
            if (model.animations[i].name == animationName) {
                currentAnimationIndex = static_cast<uint32_t>(i);
                currentTime = 0.0f;
                isLooping = loop;
                isPlaying = true;
                return;
            }
        }
        Log::Warn("Animation '{}' not found!", animationName);
		currentAnimationIndex = UINT32_MAX;
    }

    void AnimationController::Update(float deltaTime) {
        if (currentAnimationIndex == UINT32_MAX || !isPlaying) return;

        const auto& animation = pModel->animations[currentAnimationIndex];
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
		assert(currentAnimationIndex != UINT32_MAX);
		auto& model = *pModel;
        for (auto& bone : model.bones) {
            bone.currentTransform = bone.nodeTransform;
        }
        for (const auto& channel : animation.channels) {
            if (channel.boneIndex >= model.bones.size()) continue;

            auto& bone = model.bones[channel.boneIndex];

            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;
            DecomposeTransformationMatrix(bone.nodeTransform, &position, &rotation, &scale);

            if (!channel.positionKeys.empty()) {
                position = InterpolatePosition(channel.positionKeys, animationTime);
            }
            if (!channel.rotationKeys.empty()) {
                rotation = InterpolateRotation(channel.rotationKeys, animationTime);
            }
            if (!channel.scaleKeys.empty()) {
                scale = InterpolateScale(channel.scaleKeys, animationTime);
            }
            // Build transform: Translation * Rotation * Scale 
            bone.currentTransform = glm::translate(glm::mat4(1.0f), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
        }
        // Update parent-child hierarchy
        for (size_t i = 1; i < model.bones.size(); ++i) {
            auto& bone = model.bones[i];
            if (bone.parent != UINT32_MAX) {
				assert(bone.parent < model.bones.size());
				assert(bone.parent < i);
                bone.currentTransform = model.bones[bone.parent].currentTransform * bone.currentTransform;
            }
        }
    }

    glm::vec3 AnimationController::InterpolatePosition(const std::vector<GenericModel::KeyFrame>& keys, float time) const {
        if (keys.empty())
            return glm::vec3(0.0f);
        if (keys.size() == 1)
            return keys[0].value;
        if (time <= keys.front().time)
            return keys.front().value;
        if (time >= keys.back().time)
            return keys.back().value;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (time >= keys[i].time && time <= keys[i + 1].time) {
                float delta = keys[i + 1].time - keys[i].time;
                if (delta <= 0.00001f)
                    return keys[i].value;
                float t = (time - keys[i].time) / delta;
                return glm::mix(keys[i].value, keys[i + 1].value, t);
            }
        }
        return keys.back().value;
    }

    glm::quat AnimationController::InterpolateRotation(const std::vector<GenericModel::RotationKeyFrame>& keys, float time) const {
        if (keys.empty())
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (keys.size() == 1)
            return keys[0].value;
        if (time <= keys.front().time)
            return keys.front().value;
        if (time >= keys.back().time)
            return keys.back().value;
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (time >= keys[i].time && time <= keys[i + 1].time) {
                float delta = keys[i + 1].time - keys[i].time;
                if (delta <= 0.00001f)
                    return keys[i].value;
                float t = (time - keys[i].time) / delta;
                glm::quat q1 = keys[i].value;
                glm::quat q2 = keys[i + 1].value;
                // Ensure shortest path
                if (glm::dot(q1, q2) < 0.0f)
                    q2 = -q2;
                return glm::normalize(glm::slerp(q1, q2, t));
            }
        }
        return keys.back().value;
    }

    void AnimationController::GetBonePalette(std::vector<glm::mat4>& outPalette) const {
		auto& model = *pModel;
        outPalette.clear();
        outPalette.reserve(model.bones.size());
        glm::mat4 globalInverse = glm::inverse(pModel->GetRoot().globalTransform);
        for (const auto& bone : model.bones) {
            outPalette.push_back(bone.currentTransform * bone.offsetMatrix);
        }
    }
    void AnimationController::SetCurrentTime(float time) {
		currentTime = time;
        if (currentAnimationIndex == UINT32_MAX);
		UpdateBoneTransforms(pModel->animations[currentAnimationIndex], currentTime);
    }
}