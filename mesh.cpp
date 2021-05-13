#include <assert.h>
#include <iostream>
#include "mesh.hpp"

Mesh::Mesh()
	: m_VAO(0),
	  m_NumBones(0),
	  m_pScene(NULL)
{
	ZERO_MEM(m_Buffers);
}

Mesh::~Mesh()
{
	clear();
}

void Mesh::clear()
{
	for(unsigned int i = 0; i < m_Textures.size(); i++)
	{	
		SAFE_DELETE(m_Textures[i]);
	}

	if(m_Buffers[0] != 0)
	{
		glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
	}

	if(m_VAO != 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}

bool Mesh::loadMesh(const std::string& Filename)
{
	clear();

	// Create our VAO and the generate the buffers for the vertex attributes
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

	bool Ret = false;

	m_pScene = m_Importer.ReadFile(Filename.c_str(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
			aiProcess_FlipUVs);

	if(m_pScene)
	{
		m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
		m_GlobalInverseTransform.Inverse();
		Ret = initScene(m_pScene, Filename);
	}
	else
	{
		printf("Error parsing '%s': '%s'\n", Filename.c_str(),
			   m_Importer.GetErrorString());
	}

	// Just to be safe, unbind the VAO
	glBindVertexArray(0);

	return Ret;
}

bool Mesh::initScene(const aiScene* pScene, const std::string& Filename)
{
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	// Our vertex attribute arrays
	std::vector<Vector3f> Positions;
	std::vector<Vector3f> Normals;
	std::vector<Vector2f> TexCoords;
	std::vector<unsigned int> Indices;
	std::vector<VertexBoneData> Bones;

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	for(unsigned int i = 0; i < m_Entries.size(); i++)
	{
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex = NumVertices;
		m_Entries[i].BaseIndex = NumIndices;

		NumVertices += m_pScene->mMeshes[i]->mNumVertices;
		NumIndices += m_Entries[i].NumIndices;
	}

	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Indices.reserve(NumIndices);
	Bones.resize(NumVertices);

	for(unsigned int i = 0; i < m_Entries.size(); i++)
	{
		const aiMesh* paiMesh = m_pScene->mMeshes[i];
		initMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
	}

	if(!initMaterials(m_pScene, Filename))
	{
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
	// Bone IDs
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	// Bone weights
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), 
				 &Indices[0], GL_STATIC_DRAW);

	return true;
}

void Mesh::initMesh(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<Vector3f>& Positions,
					std::vector<Vector3f>& Normals, std::vector<Vector2f>& TexCoords,
					std::vector<VertexBoneData>& Bones, std::vector<unsigned int>& Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for(unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
			&(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(Vector3f(pPos->x, pPos->y, pPos->z));
		Normals.push_back(Vector3f(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(Vector2f(pTexCoord->x, pTexCoord->y));
	}

	loadBones(MeshIndex, paiMesh, Bones);

	for(unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
}

bool Mesh::initMaterials(const aiScene* pScene, const std::string& Filename)
{
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if(SlashIndex == std::string::npos)
	{
		Dir = ".";
	}
	else if(SlashIndex == 0)
	{
		Dir = "/";
	}
	else
	{
		Dir = Filename.substr(0, SlashIndex);
	}

	bool Ret = true;

	for(unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = NULL;
		//std::cerr << pMaterial->GetTextureCount(aiTextureType_EMISSIVE) << std::endl;
		if(pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString Path;

			if(pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path,
			   NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string FullPath = Dir + "/" + Path.data;
				m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

				if(!m_Textures[i]->Load())
				{
					printf("Error loading texture '%s'\n", FullPath.c_str());
					delete m_Textures[i];
					m_Textures[i] = NULL;
					Ret = false;
				}
			}
		}

		if(!m_Textures[i])
		{
			m_Textures[i] = new Texture(GL_TEXTURE_2D, "./white.png");

			Ret = m_Textures[i]->Load();
		}
	}

	return Ret;
}

void Mesh::render()
{
	glBindVertexArray(m_VAO);

	for(unsigned int i = 0; i < m_Entries.size(); i++)
	{
		unsigned int MaterialIndex = m_Entries[i].MaterialIndex;
		assert(MaterialIndex < m_Textures.size());

		if(m_Textures[MaterialIndex])
		{
			m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
		}

		glDrawElementsBaseVertex(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex), 
								 m_Entries[i].BaseVertex);
	}

	glBindVertexArray(0);
}

void Mesh::loadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	for(unsigned int i = 0; i < pMesh->mNumBones; i++)
	{
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if(m_BoneMapping.find(BoneName) == m_BoneMapping.end())
		{
			BoneIndex = m_NumBones; // This will give us a new index
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneMapping[BoneName];
		}

		for(unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++)
		{
			unsigned int VertexID = m_Entries[MeshIndex].BaseVertex + 
									pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].addBoneData(BoneIndex, Weight);
		}
	}
}

void Mesh::VertexBoneData::addBoneData(unsigned int BoneID, float Weight)
{
	for(unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
	{
		if(Weights[i] == 0.0)
		{
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	//assert(0);
}

void Mesh::boneTransform(float TimeInSeconds, std::vector<Matrix4f>& Transforms)
{
	Matrix4f Identity;
	Identity.InitIdentity();

	// In Assimp, if it cannot extract the ticks per second from the given
	// file then it sets the animations ticks-per-second to 0, so we just
	// check for this and set it to 25.0f. In Blender, ticksPerSecond usually
	// gets set to 1, so this shouldn't be a problem
	float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ?
						   m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

	readNodeHierarchy(AnimationTime, m_pScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);

	for(unsigned int i = 0; i < m_NumBones; i++)
	{
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

/*void Mesh::AnimateNodes(float current_time, aiString animName)
{
	aiAnimation* animation = m_animations[0];
	current_time = fmod(current_time, animation->mDuration);

	for(unsigned int i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* current_channel = animation->mChannels[i];

		aiVector3D bone_position;
		aiQuaternion bone_rotation;
		aiVector3D bone_scale;

		// possible bottleneck, consider looking for the affected node once and saving it
		aiNode* affected_node = findAffectedNode(m_pScene->mRootNode, current_channel->mNodeName);

		unsigned int position_index = 0, rotation_index = 0;
		for(position_index = 0; position_index < current_channel->mNumPositionKeys - 1; position_index++)
		{
			if(current_channel->mPositionKeys[position_index + 1].mTime > current_time)
				break;
		}
		bone_position = current_channel->mPositionKeys[position_index].mValue;

		for(rotation_index = 0; rotation_index < current_channel->mNumRotationKeys - 1; rotation_index++)
		{
			if(current_channel->mRotationKeys[rotation_index + 1].mTime > current_time)
				break;

		}
		bone_rotation = current_channel->mRotationKeys[rotation_index].mValue;
		
		aiMatrix4x4 transformation_matrix = aiMatrix4x4(bone_rotation.GetMatrix());
		transformation_matrix.a4 = bone_position.x;
		transformation_matrix.b4 = bone_position.y;
		transformation_matrix.c4 = bone_position.z;

		affected_node->mTransformation = transformation_matrix;
	}
	for(unsigned int i = 0; i < m_pScene->mNumMeshes; i++)
	{

		loadBoneMatrices(m_pScene->mMeshes[i], m_bone_matrices);
	}
}*/

const aiNodeAnim* Mesh::findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for(unsigned int i = 0; i < pAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if(std::string(pNodeAnim->mNodeName.data) == NodeName)
		{
			return pNodeAnim;
		}
	}

	return NULL;
}

/*aiNode* Mesh::findAffectedNode(aiNode* root_node, const aiString& node_name)
{
	if(root_node->mName == node_name)
	{
		return root_node;
	}
	else
	{
		for(unsigned int i = 0; i < root_node->mNumChildren; i++)
		{
			aiNode* child_node = findAffectedNode(root_node->mChildren[i], node_name);
			if(child_node)
			{
				return child_node;
			}
		}
	}
	return NULL;
}*/

void Mesh::readNodeHierarchy(float AnimationTime, const aiNode* pNode, const Matrix4f& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[0];
	if(m_pScene->mNumAnimations > 1)
	{
		std::cerr << m_pScene->mNumAnimations << std::endl;
	}
	Matrix4f NodeTransformation(pNode->mTransformation);
	const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, NodeName);

	if(pNodeAnim)
	{
		aiVector3D Scaling;
		interpolateScaling(Scaling, AnimationTime, pNodeAnim);
		Matrix4f ScalingMatrix;
		ScalingMatrix.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

		aiQuaternion RotationQuat;
		interpolateRotation(RotationQuat, AnimationTime, pNodeAnim);
		Matrix4f RotationMatrix = Matrix4f(RotationQuat.GetMatrix());

		aiVector3D Translation;
		interpolatePosition(Translation, AnimationTime, pNodeAnim);
		Matrix4f TranslationMatrix;
		TranslationMatrix.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

		NodeTransformation = TranslationMatrix * RotationMatrix * ScalingMatrix;
	}

	Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;
	
	if(m_BoneMapping.find(NodeName) != m_BoneMapping.end())
	{
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation *
													m_BoneInfo[BoneIndex].BoneOffset;
	}

	for(unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		readNodeHierarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

/*void Mesh::loadBoneMatrices(const aiMesh* pMesh, std::vector<aiMatrix4x4>& bone_matrices)
{
	if(!pMesh->HasBones())
		return;
	bone_matrices.resize(pMesh->mNumBones);
	for(unsigned int i = 0; i < pMesh->mNumBones; i++)
	{
		const aiBone* bone = pMesh->mBones[i];
		aiNode* affected_node = findAffectedNode(m_pScene->mRootNode, bone->mName);

		bone_matrices[i] = bone->mOffsetMatrix;
		const aiNode* temp = affected_node;
		while(temp)
		{
			bone_matrices[i] *= temp->mTransformation;
			temp = temp->mParent;
		}
	}
}*/

/*void Mesh::loadBoneIndices(const aiMesh* pMesh, std::vector<Vertex>& vertices)
{
	if(!pMesh->HasBones())
		return;
	std::vector<aiVector3D> vertex_positions;
	std::vector<aiVector3D> vertex_normals;

	vertex_positions.resize(pMesh->mNumVertices);
	vertex_normals.resize(pMesh->mNumVertices);

	for(unsigned int i = 0; i < pMesh->mNumBones; i++)
	{
		const aiBone* bone = pMesh->mBones[i];
		const aiMatrix4x4& bone_matrix = m_bone_matrices[i];
		aiMatrix3x3 bone_normal_matrix = aiMatrix3x3(bone_matrix);
		for(unsigned int j = 0; j < bone->mNumWeights; j++)
		{
			const aiVertexWeight& weight = bone->mWeights[j];
			unsigned int vertex_ID = weight.mVertexId;
			vertices[vertex_ID].addBone(i, weight.mWeight);
		}
	}
}*/

void Mesh::interpolateRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if(pNodeAnim->mNumRotationKeys == 1)
	{
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = findRotationIndex(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime -
					  		  pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = fabs(AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQuat = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQuat = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQuat, EndRotationQuat, Factor);
	Out = Out.Normalize();
}

void Mesh::interpolatePosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if(pNodeAnim->mNumPositionKeys == 1)
	{
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = findPositionIndex(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime -
					  		  pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = fabs(AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void Mesh::interpolateScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if(pNodeAnim->mNumScalingKeys == 1)
	{
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = findScalingIndex(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime -
							  pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = fabs(AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	/*std::cerr << "Factor" << Factor << std::endl;
	std::cerr << "AnimationTime: " << AnimationTime << std::endl;
	std::cerr << "ScaleKey Time: " << (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime << std::endl;
	std::cerr << "DeltaTime: " << DeltaTime << std::endl;*/
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

unsigned int Mesh::findRotationIndex(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);
	{
		for(unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
		{
			if(AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
			{
				return i;
			}
		}
	}

	assert(0);

	return 0;
}

unsigned int Mesh::findPositionIndex(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mPositionKeys > 0);
	{
		for(unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if(AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
			{
				return i;
			}
		}
	}

	assert(0);

	return 0;
}

unsigned int Mesh::findScalingIndex(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mScalingKeys > 0);
	{
		//std::cerr << "Node Name: " << pNodeAnim->mNodeName.data << std::endl;
		//std::cerr << "Num Scaling Keys: " << pNodeAnim->mNumScalingKeys << std::endl;
		//std::cerr << "Animation Time: " << AnimationTime << std::endl;
		for(unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		{
			//std::cerr << "Time: " << (float)pNodeAnim->mScalingKeys[i + i].mTime << std::endl;
			if(AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
			{
				return i;
			}
		}
	}

	assert(0);

	return 0;
}

void TerrainMesh::initMesh(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<Vector3f>& Positions,
						   std::vector<Vector3f>& Normals, std::vector<Vector2f>& TexCoords,
						   std::vector<VertexBoneData>& Bones, std::vector<unsigned int>& Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for(unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
			&(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(Vector3f(pPos->x, pPos->y, pPos->z));
		Normals.push_back(Vector3f(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(Vector2f(pTexCoord->x, pTexCoord->y));
	}

	for(unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
		Vector3f vertex1 = Positions[Face.mIndices[0]];
		Vector3f vertex2 = Positions[Face.mIndices[1]];
		Vector3f vertex3 = Positions[Face.mIndices[2]];
		m_triangle_mesh.addTriangle(btVector3(vertex1.x, vertex1.y, vertex1.z), 
									btVector3(vertex2.x, vertex2.y, vertex2.z), 
									btVector3(vertex3.x, vertex3.y, vertex3.z), 
									false);
	}
	m_terrain_mesh = new btBvhTriangleMeshShape(&m_triangle_mesh, true, true);
}
