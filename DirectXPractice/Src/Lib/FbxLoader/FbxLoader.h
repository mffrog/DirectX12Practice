#ifndef FbxLoader_h
#define FbxLoader_h

#include <fbxsdk.h>
#include "FbxLoaderStructs.h"
#include <string>
#include <vector>

namespace FbxLoader {
	/*
	Fbx読み込みクラス
	*/
	class Loader {
	public:
		~Loader();
		bool Initialize(const std::string& filename);
		void SetBoneBaseGetFromLink(bool flag) { boneBaseGetFromLink = flag; }
		void LoadBone(BoneTreeData& boneTree);
		void LoadAllMesh(std::vector<StaticMesh>& staticMeshes, std::vector<SkinnedMesh>& skinnedMeshes);
		void LoadSkinnedMesh(std::vector<SkinnedMesh>& meshes);
		void LoadStaticMesh(std::vector<StaticMesh>& meshes);
		void LoadAnimation(std::vector<Animation>& animations);

	private:

		fbxsdk::FbxNode* FindRootBone(fbxsdk::FbxNode* node);
		fbxsdk::FbxMesh* FindIncludedMesh(fbxsdk::FbxCluster* cluster);
		void LoadSkinnedMesh(fbxsdk::FbxMesh* mesh, SkinnedMesh& meshRef);
		void LoadStaticeMesh(fbxsdk::FbxMesh* mesh, StaticMesh& meshRef);


		bool isBoneTreeInitialized = false;
		bool boneBaseGetFromLink = true;
		BoneTreeData publicBoneTree;

		fbxsdk::FbxManager* pManager = nullptr;
		fbxsdk::FbxImporter* pImporter = nullptr;
		fbxsdk::FbxScene* pScene = nullptr;

		std::vector<fbxsdk::FbxMesh*> includedMeshes;
	};

}// namespace FbxLoader
#endif /* FbxLoader_h */

