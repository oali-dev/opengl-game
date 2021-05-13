#include <time.h>
#include <iostream>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "particlesystem.hpp"

ParticleSystem::ParticleSystem(std::string texture_filename)
	: m_particle_system_lifetime(5.0f),
	  m_position(0.0f, 0.0f, 0.0f),
	  m_num_particles(0),
	  m_texture_filename(texture_filename)
{
	srand((int)time(NULL));
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::init()
{
	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		//m_particles[i].m_start_time = 0.0;
		//m_particles[i].m_age = 0.0;
		//m_particles[i].m_life_span = 5.0f;
		m_particles[i].m_velocity = glm::vec3((float)(rand() % 100) - 50,
											  (float)(rand() % 100) - 50,
											  (float)(rand() % 100) - 50);
		m_particles[i].m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		//m_particles[i].m_acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
		//m_particles[i].m_isActive = false;
	}

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_particles), m_particles, GL_STATIC_DRAW);

	if(!m_image.loadFromFile(m_texture_filename))
	{
		std::cerr << "Error loading texture " << m_texture_filename << std::endl;
	}

	glGenTextures(1, &m_particle_texture);
	glBindTexture(GL_TEXTURE_2D, m_particle_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.getSize().x, m_image.getSize().y, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, m_image.getPixelsPtr());
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glPointSize(15.0f);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void ParticleSystem::init2()
{
	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		//m_particles[i].m_start_time = 0.0;
		//m_particles[i].m_age = 0.0;
		//m_particles[i].m_life_span = 5.0f;
		m_particles[i].m_velocity = glm::vec3(0.0f, 
											  1.0f + (float)(rand() % 1200) / 100.0f, 
											  0.0f);
		m_particles[i].m_position = glm::vec3(((float)(rand() % 1000) / 50.0f) - 10,
											  ((float)(rand() % 1000) / 50.0f) - 10,
											  ((float)(rand() % 1000) / 50.0f) - 10);
		//m_particles[i].m_acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
		//m_particles[i].m_isActive = false;
	}

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_particles), m_particles, GL_STATIC_DRAW);

	if(!m_image.loadFromFile(m_texture_filename))
	{
		std::cerr << "Error loading texture " << m_texture_filename << std::endl;
	}

	glGenTextures(1, &m_particle_texture);
	glBindTexture(GL_TEXTURE_2D, m_particle_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.getSize().x, m_image.getSize().y, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, m_image.getPixelsPtr());
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glPointSize(15.0f);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void ParticleSystem::update(float elapsedTime)
{
}

void ParticleSystem::render(float particleSize)
{
	glPointSize(particleSize);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_particle_texture);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)12);
	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
