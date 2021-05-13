#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <string>

class Skybox
{
public:
	Skybox(std::string top, std::string bottom, std::string front,
		   std::string back, std::string left, std::string right);
	~Skybox();
	void init();
	void render();

private:
	sf::Image m_top;
	sf::Image m_bottom;
	sf::Image m_front;
	sf::Image m_back;
	sf::Image m_left;
	sf::Image m_right;

	GLuint m_texture;
	GLuint m_VBO;
	GLuint m_IBO;
};

#endif
