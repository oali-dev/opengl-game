#include <iostream>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "skybox.hpp"

const GLfloat skybox_vertices[] = 
{
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
};

const GLuint skybox_indices[] =
{
	0, 1, 2, 3,
	3, 2, 6, 7,
	7, 6, 5, 4,
	4, 5, 1, 0,
	0, 3, 7, 4,
	5, 6, 2, 1,
};

Skybox::Skybox(std::string top, std::string bottom, std::string front,
			   std::string back, std::string left, std::string right)
{
	if(!m_top.loadFromFile(top))
	{
		std::cerr << "Could not load skybox texture " << top << std::endl;
	}
	if(!m_bottom.loadFromFile(bottom))
	{
		std::cerr << "Could not load skybox texture " << bottom << std::endl;
	}
	if(!m_front.loadFromFile(front))
	{
		std::cerr << "Could not load skybox texture " << front << std::endl;
	}
	if(!m_back.loadFromFile(back))
	{
		std::cerr << "Could not load skybox texture " << back << std::endl;
	}
	if(!m_left.loadFromFile(left))
	{
		std::cerr << "Could not load skybox texture " << left << std::endl;
	}
	if(!m_right.loadFromFile(right))
	{
		std::cerr << "Could not load skybox texture " << right << std::endl;
	}
}

Skybox::~Skybox()
{
	glDeleteTextures(1, &m_texture);
}

void Skybox::init()
{
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, m_right.getSize().x, m_right.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_right.getPixelsPtr());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, m_left.getSize().x, m_left.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_left.getPixelsPtr());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, m_top.getSize().x, m_top.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_top.getPixelsPtr());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, m_bottom.getSize().x, m_bottom.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_bottom.getPixelsPtr());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, m_front.getSize().x, m_front.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_front.getPixelsPtr());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, m_back.getSize().x, m_back.getSize().y,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_back.getPixelsPtr());

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skybox_indices), skybox_indices, GL_STATIC_DRAW);
}

void Skybox::render()
{
	glFrontFace(GL_CW);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glDrawElements(GL_QUADS, sizeof(skybox_indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glFrontFace(GL_CCW);
}
