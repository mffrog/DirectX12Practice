#pragma once
#include <vector>
#include <string>

namespace FbxLoader {
	struct Vec2 {
		Vec2(float v = 0.0f) :x(v), y(v) {}
		Vec2(float x, float y) : x(x), y(y) {}
		float x, y;
	};
	struct Vec3{
		Vec3(float v = 0.0f) : x(v), y(v), z(v){}
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
		float x, y, z;
	};
	struct Vec4 {
		Vec4(float v = 0.0f) : x(v), y(v), z(v), w(1.0f){}
		Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		float x, y, z, w;
	};
	struct Mat {

		union {
			float m[16];
			Vec4 v[4];
		};
	};


	struct StaticVertex {
		Vec3 position;
		Vec4 color;
		Vec2 texcoord;
		Vec3 normal;
		Vec4 tangent;
	};

	struct SkinnedVertex {
		Vec3 position;
		Vec4 color;
		Vec2 texcoord;
		Vec3 normal;
		Vec4 tangent;
		unsigned int boneIndex[4];
		Vec4 weights;
	};

	struct Bone {
		std::string name;
		int boneId = -1;
		int parentId = -1;
		std::vector<int> children;
		Mat offset;
	};

	struct BoneTree {
		std::vector<Bone> bones;
	};

	struct StaticMeshMaterial {
		std::string name;
		std::vector<StaticVertex> vertices;
		std::vector<unsigned int> indices;
	};

	struct StaticMesh {
		std::string name;
		std::vector<StaticMeshMaterial> materials;
	};

	struct SkinnedMeshMaterial {
		std::string name;
		std::vector<SkinnedVertex> vertices;
		std::vector<unsigned int> indices;
	};
	struct SkinnedMesh {
		std::string name;
		BoneTree boneTree;
		std::vector<SkinnedMeshMaterial> materials;
	};

} // namespace FbxLoader