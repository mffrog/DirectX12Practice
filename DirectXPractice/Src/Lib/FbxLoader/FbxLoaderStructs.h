#ifndef FbxLoaderDataStructs_h
#define FbxLoaderDataStructs_h

#include "../../Math/Vector/Vector2.h"
#include "../../Math/Vector/Vector2.h"
#include "../../Math/Vector/Vector2.h"
#include "../../Math/Matrix/Matrix4x4.h"
#include <vector>

namespace FbxLoader {
	struct StaticVertex {
		mff::Vector3<float> position;
		mff::Vector4<float> color = { 1,1,1,1 };
		mff::Vector2<float> texCoord;
		mff::Vector3<float> normal;
		mff::Vector4<float> tangent;
	};

	struct SkinnedVertex {
		mff::Vector3<float> position;
		mff::Vector4<float> color = mff::Vector4<float>(1, 1, 1, 1);
		mff::Vector2<float> texCoord;
		mff::Vector3<float> normal;
		mff::Vector4<float> tangent;
		unsigned int boneIndex[4] = { 0,0,0,0 };
		mff::Vector4<float> weights = { 0.0,0.0,0.0,0.0 };
	};

	//コントロールポイント毎のウェイト取得
	struct PerCpBoneIndexAndWeight {
		std::vector<std::pair<int, double>> weights;
	};

	//各Boneのデータ
	struct BoneData {
		std::string name;
		int boneId = -1;
		int parentId = -1;
		std::vector<int> children;
		mff::Matrix4x4<float>  baseInv;
	};

	struct BoneTreeData {
		std::vector<BoneData> data;
		BoneData* FindBone(const std::string& name) {
			for (auto itr = data.begin(); itr != data.end(); ++itr) {
				if (itr->name == name) {
					return &(*itr);
				}
			}
			return nullptr;
		}

		BoneData* FindBone(int boneId) {
			for (auto itr = data.begin(); itr != data.end(); ++itr) {
				if (itr->boneId == boneId) {
					return &(*itr);
				}
			}
			return nullptr;
		}
	};

	template<typename VertType>
	struct Material {
		std::string name;
		std::vector<unsigned int> indeces;
		std::vector<VertType> verteces;
		std::vector<std::string> textureName;
	};

	struct StaticMesh {
		std::string name;
		std::vector<Material<StaticVertex>> materials;
	};

	struct SkinnedMesh {
		std::string name;
		std::vector<Material<SkinnedVertex>> materials;
		std::vector<mff::Matrix4x4<float> > boneBaseInvs;
	};

	struct Relation {
		/**
		* インデックスバッファの値
		*/
		std::vector<int> relatedIndex;
	};

	struct BoneAnimationData {
		typedef std::pair<float, mff::Matrix4x4<float> > TimeMatPair;
		std::vector<std::pair<float, mff::Matrix4x4<float> >> animDatas;
		int current = -1;
		int next = -1;
		float animationTime = 0;
		mff::Matrix4x4<float>  GetMat(float time) {
			if (!animDatas.size()) {
				return {};
			}

			if (animDatas.size() == 1) {
				return animDatas[0].second;
			}

			if (time > animationTime) {
				while (time > animationTime) {
					time -= animationTime;
				}
				current = 0;
				next = 1;
			}

			if (current == -1) {
				for (int i = 0; i < static_cast<int>(animDatas.size()); ++i) {
					if (time < animDatas[i].first) {
						next = i;
						current = i - 1;
						break;
					}
				}
			}

			if (time < animDatas[current].first) {
				Reset();
			}

			if (current == -1) {
				for (int i = 0; i < static_cast<int>(animDatas.size()); ++i) {
					if (time < animDatas[i].first) {
						next = i;
						current = i - 1;
						break;
					}
				}
			}

			while (time > animDatas[next].first) {
				current++;
				next++;
				if (next >= static_cast<int>(animDatas.size())) {
					next = static_cast<int>(animDatas.size()) - 1;
					current = next - 1;
					return animDatas.back().second;
				}
			}

			float duration = animDatas[next].first - animDatas[current].first;
			float rate = (time - animDatas[current].first) / duration;
			if (rate > 1 || rate < 0) {
				std::cout << "irregular happened" << std::endl;
			}
			return animDatas[current].second * (1 - rate) + animDatas[next].second * rate;
		}

		void Reset() {
			current = -1;
			next = -1;
		}
	};

	struct Animation {
		std::string name;
		float animationTime = 0;
		//[bone]
		std::vector<BoneAnimationData> boneAnimationData;
		std::vector<mff::Matrix4x4<float> > boneMats;
		const std::vector<mff::Matrix4x4<float> >& GetMat(float time) {
			size_t boneNum = boneAnimationData.size();
			if (boneMats.size() < boneNum) {
				boneMats.resize(boneNum);
			}
			for (size_t i = 0; i < boneNum; ++i) {
				if (time > animationTime || time == 0.0f) {
					boneAnimationData[i].Reset();
					time = 0;
				}
				boneMats[i] = boneAnimationData[i].GetMat(time);
			}
			return boneMats;
		}
	};

}// namespace FbxLoader

#endif /* FbxLoaderDataStructs_h */

