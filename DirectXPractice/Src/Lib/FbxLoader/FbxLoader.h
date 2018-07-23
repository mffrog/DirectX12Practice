#pragma once
#include "fbxsdk.h"
#include "FbxLoaderStructs.h"
#include <vector>

namespace FbxLoader {
	class Loader {
	public:
		enum AxisType {
			RightHand,
			LeftHand,
		};
		~Loader();
		bool Initialize(const char* filename);

		void LoadStaticMeshes(std::vector<StaticMesh>& meshes);
		void LoadSkinnedMesh(std::vector<SkinnedMesh>& meshes);
	private:
		void GetMeshNodes();

		void LoadStaticMesh(fbxsdk::FbxMesh* mesh, StaticMesh& meshData);
	private:
		FbxManager* manager;
		FbxScene* scene;
		AxisType axisType = RightHand;
		std::vector<fbxsdk::FbxMesh*> meshNodes;
	};
} // namespace FbxLoader