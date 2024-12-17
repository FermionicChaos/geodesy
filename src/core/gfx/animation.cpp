#include <geodesy/core/gfx/animation.h>

#include <string>
#include <vector>
#include <map>

#include <geodesy/core/math.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// using namespace std;

namespace geodesy::core::gfx {

	using namespace math;

	// This template function finds keys in a keyframe animation.
	template <typename T> inline
	std::pair<animation::key<T>, animation::key<T>> find_value_pair(const std::vector<animation::key<T>>& aKey, double aTime) {
		animation::key<T> Key1, Key2;
		switch (aKey.size()) {
		default:
			break;
		case 0:
			return std::pair<animation::key<T>, animation::key<T>>(Key1, Key2);
		case 1:
			return std::pair<animation::key<T>, animation::key<T>>(aKey[0], aKey[0]);
		case 2:
			return std::pair<animation::key<T>, animation::key<T>>(aKey[0], aKey[1]);
		}
		// Check if aTime is in bounds.
		if ((aTime < aKey[0].Time) || (aTime > aKey[aKey.size() - 1].Time)) return std::pair<animation::key<T>, animation::key<T>>(Key1, Key2);
		// Get middle index from key vector.
		size_t LeftIndex = 0;
		size_t RightIndex = (aKey.size() - 1);
		// Binary search.
		while (true) {
			size_t MiddleIndex = (LeftIndex + RightIndex) >> 1;
			double KeyTime = aKey[MiddleIndex].Time;
			// Check if two keys are found.
			if ((aTime >= aKey[MiddleIndex].Time) && (aTime <= aKey[MiddleIndex + 1].Time)) {
				// Two Keys found, exit loop.
				Key1 = aKey[MiddleIndex];
				Key2 = aKey[MiddleIndex + 1];
				break;
			}
			else if ((aTime < aKey[MiddleIndex].Time) && (aTime < aKey[MiddleIndex + 1].Time)) {
				// If the time is less than the key time.
				RightIndex = MiddleIndex;
			}
			else if ((aTime > aKey[MiddleIndex].Time) && (aTime > aKey[MiddleIndex + 1].Time)) {
				LeftIndex = MiddleIndex;
			}
			else {
				break;
			}
		}
		return std::pair<animation::key<T>, animation::key<T>>(Key1, Key2);
	}

	template<typename T> inline
	float interpolation_factor(std::pair<animation::key<T>, animation::key<T>> aKeyPair, double aTime) {
		if (aKeyPair.first.Time == aKeyPair.second.Time) {
			return 0.0f;
		} else {
			return (aTime - aKeyPair.first.Time) / (aKeyPair.second.Time - aKeyPair.first.Time);
		}
	}

	mat<float, 4, 4> animation::node_anim::operator[](double aTime) const {
		mat<float, 4, 4> T = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		mat<float, 4, 4> R = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		mat<float, 4, 4> S = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		// Calculates interpolated translation matrix
		if (this->PositionKey.size() > 0) {
			std::pair<key<vec<float, 3>>, key<vec<float, 3>>> TP = find_value_pair(this->PositionKey, aTime);
			float p = interpolation_factor(TP, aTime);
			vec<float, 3> Tf = (1.0f - p) * TP.first.Value + p * TP.second.Value;
			T = mat<float, 4, 4>(
				1.0f, 0.0f, 0.0f, Tf[0],
				0.0f, 1.0f, 0.0f, Tf[1],
				0.0f, 0.0f, 1.0f, Tf[2],
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// Calculates interpolated quaternion.
		if (this->RotationKey.size() > 0){
			std::pair<key<quaternion<float>>, key<quaternion<float>>> RP = find_value_pair(this->RotationKey, aTime);
			float p = interpolation_factor(RP, aTime);
			quaternion<float> Q1 = RP.first.Value;
			quaternion<float> Q2 = RP.second.Value;
			float CosTheta = Q1[0]*Q2[0] + Q1[1]*Q2[1] + Q1[2]*Q2[2] + Q1[3]*Q2[3];
			if (std::abs(CosTheta) > 0.999f) {
				// Cartesian LERP.
				quaternion<float> Qf = (1.0f - p) * Q1 + p * Q2;
				R = rotation(Qf);
			}
			else {
				// Spherical LERP.
				float Theta = std::acos(CosTheta);
				quaternion<float> Qf = ((std::sin((1.0f - p) * Theta) * Q1 + std::sin(p * Theta) * Q2) / std::sin(Theta));
				R = rotation(Qf);
			}
		}

		// Calculates interpolated scaling matrix
		if (this->ScalingKey.size() > 0) {
			std::pair<key<vec<float, 3>>, key<vec<float, 3>>> SP = find_value_pair(this->ScalingKey, aTime);
			float p = interpolation_factor(SP, aTime);
			vec<float, 3> Sf = (1.0f - p) * SP.first.Value + p * SP.second.Value;
			T = mat<float, 4, 4>(
				Sf[0], 0.0f, 0.0f, 0.0f,
				0.0f, Sf[1], 0.0f, 0.0f,
				0.0f, 0.0f, Sf[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// Order matters, scaling is applied first, then the object is rotated, then translated.
		return T * R * S;
	}

	animation::animation() {
		this->Name = "";
		this->Duration = 0.0;
		this->TicksPerSecond = 0.0;
	}

	animation::animation(const aiAnimation* aAnimation) {
		this->Name 				= aAnimation->mName.C_Str();
		this->Duration 			= aAnimation->mDuration;
		this->TicksPerSecond 	= aAnimation->mTicksPerSecond;
		for (uint i = 0; i < aAnimation->mNumChannels; i++) {
			aiNodeAnim* RNA = aAnimation->mChannels[i];
			std::string NodeName = RNA->mNodeName.C_Str();
			this->NodeAnimMap[NodeName] = animation::node_anim();
			animation::node_anim& LNA = this->NodeAnimMap[NodeName];

			// Get Position Keys, size vector, then fill with data.
			LNA.PositionKey = std::vector<animation::key<math::vec<float, 3>>>(RNA->mNumPositionKeys);
			for (uint j = 0; j < RNA->mNumPositionKeys; j++) {
				LNA.PositionKey[j].Time = RNA->mPositionKeys[j].mTime;
				LNA.PositionKey[j].Value = {
					RNA->mPositionKeys[j].mValue.x,
					RNA->mPositionKeys[j].mValue.y,
					RNA->mPositionKeys[j].mValue.z
				};
			}

			// Get Rotation Keys, size vector, then fill with data.
			LNA.RotationKey = std::vector<animation::key<math::quaternion<float>>>(RNA->mNumRotationKeys);
			for (uint j = 0; j < RNA->mNumRotationKeys; j++) {
				LNA.RotationKey[j].Time = RNA->mRotationKeys[j].mTime;
				LNA.RotationKey[j].Value = {
					RNA->mRotationKeys[j].mValue.w,
					RNA->mRotationKeys[j].mValue.x,
					RNA->mRotationKeys[j].mValue.y,
					RNA->mRotationKeys[j].mValue.z
				};
			}

			// Get Scaling Keys, size vector, then fill with data.
			LNA.ScalingKey = std::vector<animation::key<math::vec<float, 3>>>(RNA->mNumScalingKeys);
			for (uint j = 0; j < RNA->mNumScalingKeys; j++) {
				LNA.ScalingKey[j].Time = RNA->mScalingKeys[j].mTime;
				LNA.ScalingKey[j].Value = {
					RNA->mScalingKeys[j].mValue.x,
					RNA->mScalingKeys[j].mValue.y,
					RNA->mScalingKeys[j].mValue.z
				};
			}
		}
	}

	const animation::node_anim& animation::operator[](std::string aNodeName) const {
		auto it = NodeAnimMap.find(aNodeName);
    	if (it == NodeAnimMap.end()) {
    	    static const node_anim EmptyNodeAnim;  // Static empty node animation to return if not found
    	    return EmptyNodeAnim;
    	}
    	return it->second;
	}

}
