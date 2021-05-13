#ifndef PARTICLESYSTEM_HPP
#define PARTICLESYSTEM_HPP

#include <string>
#include "glm/glm.hpp"

#define MAX_PARTICLES 180

struct Particle
{
	glm::vec3 m_velocity;
	glm::vec3 m_position;
	float m_start_time;
	float m_life_span;
	float m_age;
	//float m_init_size;
	//float m_final_size;
	//int m_energy;
	//glm::vec3 m_init_velocity;
	//glm::vec3 m_init_displacement;
	//glm::vec3 m_prev_position;
	glm::vec3 m_acceleration;
	//glm::vec4 m_initial_color;
	//glm::vec4 m_final_color;
	bool m_isActive;
};

class ParticleSystem
{
public:
	ParticleSystem(std::string texture_filename);
	~ParticleSystem();
	// make these virtual functions
	void init();
	void init2();
	void update(float elapsedTime);
	void render(float particleSize);

private:
	float m_particle_system_lifetime;
	unsigned int m_num_particles;
	Particle m_particles[MAX_PARTICLES];
	glm::vec3 m_position;
	sf::Image m_image;
	std::string m_texture_filename;
	GLuint m_VBO;
	GLuint m_particle_texture;
};

#endif
