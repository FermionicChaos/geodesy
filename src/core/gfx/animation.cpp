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

	mat<float, 4, 4> animation::node_anim::operator[](double aTime) const {
		mat<float, 4, 4> T;
		mat<float, 4, 4> R;
		mat<float, 4, 4> S;

		// Calculates interpolated translation matrix
		{
			float p = 0.0f;
			float t = aTime;
			vec<float, 3> T1, T2, Tf;
			for (size_t i = 0; i < this->PositionKey.size() - 1; i++) {
				float t1 = this->PositionKey[i].Time;
				float t2 = this->PositionKey[i + 1].Time;
				if ((aTime >= t1) && (aTime <= t2)) {
					p = ((t - t1) / (t2 - t1));
					T1 = this->PositionKey[i].Value;
					T2 = this->PositionKey[i + 1].Value;
					break;
				}
			}
			Tf = (1.0f - p) * T1 + p * T2;
			T = mat<float, 4, 4>(
				1.0f, 0.0f, 0.0f, Tf[0],
				0.0f, 1.0f, 0.0f, Tf[1],
				0.0f, 0.0f, 1.0f, Tf[2],
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// Calculates interpoted quaternion.
		{
			float p = 0.0f;
			float t = aTime;
			quaternion<float> Q1, Q2, Qf;
			for (size_t i = 0; i < this->RotationKey.size() - 1; i++) {
				float t1 = this->RotationKey[i].Time;
				float t2 = this->RotationKey[i + 1].Time;
				if ((aTime >= t1) && (aTime <= t2)) {
					p = ((t - t1) / (t2 - t1));
					Q1 = this->RotationKey[i].Value;
					Q2 = this->RotationKey[i + 1].Value;
					break;
				}
			}
			float Theta = std::acos(Q1[0]*Q2[0] + Q1[1]*Q2[1] + Q1[2]*Q2[2] + Q1[3]*Q2[3]);
			Qf = ((std::sin((1.0f - p) * Theta) * Q1 + std::sin(p * Theta) * Q2) / std::sin(Theta));
			R = rotation(Qf);
		}

		// Calculates interpolated scaling matrix
		{
			float p = 0.0f;
			float t = aTime;
			vec<float, 3> S1, S2, Sf;
			for (size_t i = 0; i < this->ScalingKey.size() - 1; i++) {
				float t1 = this->ScalingKey[i].Time;
				float t2 = this->ScalingKey[i + 1].Time;
				if ((aTime >= t1) && (aTime <= t2)) {
					p = ((t - t1) / (t2 - t1));
					S1 = this->ScalingKey[i].Value;
					S2 = this->ScalingKey[i + 1].Value;
					break;
				}
			}
			Sf = (1.0f - p) * S1 + p * S2;
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
		this->Weight = 0.0f;
		this->Duration = 0.0;
		this->TicksPerSecond = 0.0;
	}

	animation::animation(const aiAnimation* aAnimation) {
		this->Name 				= aAnimation->mName.C_Str();
		this->Weight 			= 0.0f;
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

	animation::node_anim animation::operator[](std::string aNodeName) {
		return this->NodeAnimMap[aNodeName];
	}

}
