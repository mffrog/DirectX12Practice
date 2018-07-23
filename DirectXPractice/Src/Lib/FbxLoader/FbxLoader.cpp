#include "FbxLoader.h"
#include <iostream>

using namespace fbxsdk;

namespace FbxLoader {
	template<typename T>
	Vec2 ToVec2(const T& v) {
		return Vec2(v[0], v[1]);
	}
	template<typename T>
	Vec3 ToVec3(const T& v) {
		return Vec3(v[0], v[1], v[2]);
	}
	template<typename T>
	Vec4 ToVec4(const T& v) {
		return Vec4(v[0], v[1], v[2], v[3]);
	}

	Loader::~Loader() {
		if (scene) {
			scene->Destroy();
		}
		if (manager) {
			manager->Destroy();
		}
	}

	bool Loader::Initialize(const char* filename) {
		manager = fbxsdk::FbxManager::Create();
		if (!manager) {
			return false;
		}
		FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
		manager->SetIOSettings(ios);

		fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(manager, "");
		if (!importer) {
			return false;
		}

		if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
			std::cerr << "FbxImporter::Initialize is failed." << std::endl;
			std::cerr << "Error returned :" << importer->GetStatus().GetErrorString() << "\n" << std::endl;
			return false;
		}

		scene = fbxsdk::FbxScene::Create(manager, "");
		if (!scene) {
			return false;
		}

		fbxsdk::FbxGeometryConverter geometryConverter(manager);
		geometryConverter.Triangulate(scene, true);
		GetMeshNodes();
		return true;
	}

	void Loader::LoadStaticMeshes(std::vector<StaticMesh>& meshes) {
		size_t meshCount = meshNodes.size();
		for (size_t i = 0; i < meshCount; ++i) {

		}
	}

	void Loader::GetMeshNodes() {
		int count = scene->GetSrcObjectCount<FbxMesh>();
		for (int i = 0; i < count; ++i) {
			meshNodes.push_back(scene->GetSrcObject<FbxMesh>());
		}
	}

	void Loader::LoadStaticMesh(fbxsdk::FbxMesh * mesh, StaticMesh & meshData) {
		meshData.name = mesh->GetName();

		//Vertex Count
		size_t cpCount = mesh->GetControlPointsCount();
		//Vertices
		FbxVector4* controlPoinnts = mesh->GetControlPoints();

		FbxNode* node = mesh->GetNode();
		FbxAMatrix mat = node->EvaluateGlobalTransform();
		FbxAMatrix rot(FbxVector4(0.0, 0.0, 0.0), mat.GetR(), FbxVector4(1.0, 1.0, 1.0));

		size_t materialCount = mesh->GetNode()->GetMaterialCount();
		size_t elementMaterial = mesh->GetElementMaterialCount();
		if (materialCount != elementMaterial) {
			std::cout << "not equal!" << std::endl;
		}
		materialCount = materialCount ? materialCount : 1;

		auto materials = meshData.materials;
		materials.resize(materialCount);
		for (size_t i = 0; i < materialCount; ++i) {
			FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);
			if (surfaceMaterial) {
				materials[i].name = surfaceMaterial->GetName();
			}
		}

		size_t polygonOrder[3];
		if (axisType == RightHand) {
			polygonOrder[0] = 0;
			polygonOrder[1] = 1;
			polygonOrder[2] = 2;
		}
		else {
			polygonOrder[0] = 0;
			polygonOrder[1] = 2;
			polygonOrder[2] = 1;
		}

		size_t polygonCount = mesh->GetPolygonCount();
		for (size_t polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex) {
			for (size_t posInPolygon : polygonOrder) {

			}
		}
	}



} // namespace FbxLoader