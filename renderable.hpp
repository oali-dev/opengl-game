#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include <iostream>
#include "mesh.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/constants.hpp"
#include "math_3d.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class Renderable
{
public:
	Renderable() {}
	virtual ~Renderable() {}
	virtual void render() { m_mesh->render(); }
	const glm::mat4& getTranslation() { return m_translation_matrix; }
	const glm::mat4& getRotation() { return m_rotation_matrix; }
	const glm::mat4& getScale() { return m_scale_matrix; }

protected:
	Mesh* m_mesh;

	glm::mat4 m_translation_matrix;
	glm::mat4 m_rotation_matrix;
	glm::mat4 m_scale_matrix;

};

class StaticRenderable : public Renderable
{
public:
	StaticRenderable(Mesh* mesh, glm::vec3 translation, glm::vec3 rotation,
					 glm::vec3 scale, bool isVisible);
	virtual ~StaticRenderable();

protected:
};

class DynamicRenderable : public Renderable
{
public:
	enum Direction
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};
	DynamicRenderable(std::vector<Mesh*>& meshes, glm::vec3 translation, glm::vec3 rotation,
					  glm::vec3 scale, bool isVisible, btPairCachingGhostObject* controller);
	virtual ~DynamicRenderable();
	void rotateForward();
	void rotateBackward();
	void rotateLeft();
	void rotateRight();
	void setRotation(float angle)
	{
		m_rotation.y = angle;
	}
	void setRotation(glm::mat3 rotationMatrix)
	{
		m_rotation = rotationMatrix * m_rotation;
	}
	const glm::vec3& getTransform();
	void setTransform(float x, float y, float z);
	void move();
	void UpdateTransforms(float Time)
	{
		m_animation_meshes[m_animation_index]->boneTransform(Time, m_transforms);
	}
	const std::vector<Matrix4f>& getTransforms()
	{
		return m_transforms;
	}
	void render()
	{
		m_animation_meshes[m_animation_index]->render();
	}
	void setAnimation(int index)
	{
		m_animation_index = index;
	}
	const btPairCachingGhostObject* getController()
	{
		return m_ghost_object;
	}
	bool m_isVisible;

protected:
	glm::vec3 m_translation;
	glm::quat m_initial_rotation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;
	std::vector<Matrix4f> m_transforms;
	std::vector<Mesh*> m_animation_meshes;
	unsigned int m_animation_index;

	Direction m_direction;
	btPairCachingGhostObject* m_ghost_object;
};

#endif
