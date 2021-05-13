#include "renderable.hpp"

StaticRenderable::StaticRenderable(Mesh* mesh, glm::vec3 translation, glm::vec3 rotation, 
								   glm::vec3 scale, bool isVisible)
{
	m_mesh = mesh;
	m_translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::quat rotation_quat = glm::quat(rotation);
	m_rotation_matrix = glm::mat4_cast(rotation_quat);
	m_scale_matrix = glm::scale(glm::mat4(1.0f), scale);
}

StaticRenderable::~StaticRenderable()
{
}

DynamicRenderable::DynamicRenderable(std::vector<Mesh*>& meshes, glm::vec3 translation, 
									 glm::vec3 rotation, glm::vec3 scale, bool isVisible,
									 btPairCachingGhostObject* controller)
	: m_direction(FORWARD),
	  m_rotation(0.0f, 0.0f, 0.0f),
	  m_animation_index(0),
	  m_ghost_object(controller),
	  m_isVisible(isVisible)
{
	m_animation_meshes = meshes;
	m_translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::quat rotation_quat = glm::quat(rotation);
	m_rotation_matrix = glm::mat4_cast(rotation_quat);
	m_scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	m_translation = translation;
	m_initial_rotation = rotation_quat;
	m_scale = scale;
}

DynamicRenderable::~DynamicRenderable()
{	
}

void DynamicRenderable::rotateForward()
{
	if(m_direction != FORWARD)
	{
		m_direction = FORWARD;
		m_rotation.y = 0.0f;
	}
}

void DynamicRenderable::rotateBackward()
{
	if(m_direction != BACKWARD)
	{
		m_direction = BACKWARD;
		m_rotation.y = 3.14f;
	}
}

void DynamicRenderable::rotateLeft()
{
	if(m_direction != LEFT)
	{
		m_direction = LEFT;
		m_rotation.y = 3.14f / 2.0f;
	}
}

void DynamicRenderable::rotateRight()
{
	if(m_direction != RIGHT)
	{
		m_direction = RIGHT;
		m_rotation.y = -3.14f / 2.0f;
	}
}

const glm::vec3& DynamicRenderable::getTransform()
{
	return m_translation;
}

void DynamicRenderable::setTransform(float x, float y, float z)
{
	m_translation = glm::vec3(x, y, z);
}

void DynamicRenderable::move()
{
	glm::quat rotation_quat = glm::quat(m_rotation);
	rotation_quat = rotation_quat * m_initial_rotation;
	glm::normalize(rotation_quat);
	m_rotation_matrix = glm::mat4_cast(rotation_quat);
	m_translation_matrix = glm::translate(glm::mat4(1.0f), m_translation);
}
