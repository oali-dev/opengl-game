#ifndef MESH_HPP
#define MESH_HPP

#include <map>
#include <vector>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util.h"
#include "math_3d.h"
#include "texture.h"
#include "btBulletDynamicsCommon.h"

class Mesh
{
public:
	struct BoneInfo
	{
		Matrix4f BoneOffset;
		Matrix4f FinalTransformation;

		BoneInfo()
		{
			BoneOffset.SetZero();
			FinalTransformation.SetZero();
		}
	};
	struct VertexBoneData
	{
		unsigned int IDs[4];
		float Weights[4];

		VertexBoneData()
		{
			ZERO_MEM(IDs);
			ZERO_MEM(Weights);
		}

		void addBoneData(unsigned BoneID, float Weight);
	};

	Mesh();
	~Mesh();
	bool loadMesh(const std::string& Filename);
	void render();
	void boneTransform(float TimeInSeconds, std::vector<Matrix4f>& Transforms);

	void loadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones);

protected:

	bool initScene(const aiScene* pScene, const std::string& Filename);
	virtual void initMesh(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<Vector3f>& Positions, 
				 		  std::vector<Vector3f>& Normals, std::vector<Vector2f>& TexCoords,
				  		  std::vector<VertexBoneData>& Bones, std::vector<unsigned int>& Indices);
	bool initMaterials(const aiScene* pScene, const std::string& Filename);
	void clear();
	void interpolateRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void interpolatePosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void interpolateScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int findRotationIndex(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int findPositionIndex(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int findScalingIndex(float AnimationTime, const aiNodeAnim* pNodeAnim);
	void readNodeHierarchy(float AnimationTime, const aiNode* pNode, const Matrix4f& ParentTransform);
	const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

	struct MeshEntry
	{
		MeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = 0xFFFFFFFF;
		}

		unsigned int NumIndices;
		unsigned int MaterialIndex;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
	};

	GLuint m_VAO;
	GLuint m_Buffers[5];

	const aiScene* m_pScene;
	Assimp::Importer m_Importer;
	Matrix4f m_GlobalInverseTransform;
	std::vector<MeshEntry> m_Entries;
	std::vector<Texture*> m_Textures;
	std::map<std::string, unsigned int> m_BoneMapping;
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;
};

class TerrainMesh : public Mesh
{
public:
	void initMesh(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<Vector3f>& Positions, 
		 		  std::vector<Vector3f>& Normals, std::vector<Vector2f>& TexCoords,
		  		  std::vector<VertexBoneData>& Bones, std::vector<unsigned int>& Indices);
	btBvhTriangleMeshShape* getTerrainCollisionBody() { return m_terrain_mesh; }

private:
	btTriangleMesh m_triangle_mesh;
	btBvhTriangleMeshShape* m_terrain_mesh;
};
#endif
