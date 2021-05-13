#ifndef GAME_HPP
#define GAME_HPP

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "renderable.hpp"
#include "skybox.hpp"
#include "particlesystem.hpp"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#define MAX_BONES 64

class Game
{
public:
	Game();
	~Game();
	int startGame();
	void initializeProgram();
	void initializeVertexBuffer();
	void display();
	void init();
	void initFramebuffer();
	void renderPlayerAttackParticles(/*glm::vec3 position, int index*/);
	void renderPlayerHealParticles(/*glm::vec3 position, int index*/);
	void renderEnemyAttackParticles(/*glm::vec3 position, int index*/);
	void renderEnemyHealParticles(/*glm::vec3 position, int index*/);
	void renderDepthMap();
	GLuint createShader(GLenum eShaderType, const std::string &strShaderFile);
	GLuint createProgram(const std::vector<GLuint> &shaderList);
	std::string parseShader(const char* filename);

private:
	void gameLoop();
	sf::Clock m_clock;
	sf::Clock m_battle_clock;
	sf::RenderWindow m_window;
	//std::vector<Renderable*> m_renderables;
	std::vector<StaticRenderable*> m_static_renderables;
	std::vector<DynamicRenderable*> m_dynamic_renderables;
	DynamicRenderable* m_player;
	Skybox m_skybox;
	float m_gravity;
	ParticleSystem m_particle_system;
	ParticleSystem m_particle_system2;
	ParticleSystem m_particle_system_enemy;
	ParticleSystem m_particle_system_enemy2;
	int m_index;

	sf::Music m_battle_theme;
	sf::Music m_forest_theme;
	bool m_is_encounter_initiated;
	DynamicRenderable* m_encountered_enemy;
	float m_encounter_distance;
	bool m_is_showing_gui;
	sf::Texture m_gui_texture;
	sf::Sprite m_gui_sprite;
	sf::SoundBuffer m_sword_attack;
	sf::SoundBuffer m_defend;
	sf::SoundBuffer m_heal;
	sf::SoundBuffer m_bite;
	sf::Sound m_sound;
	sf::Sound m_sound2;
	glm::vec3 m_particle_target;

	int m_player_health;
	int m_enemy_health;

	float m_player_attack_anim_end_time;
	float m_enemy_attack_anim_end_time;
	bool m_can_play_defend_sound;

	bool m_is_player_attacking;
	float m_player_attack_end_time;
	float m_player_heal_end_time;

	bool m_is_enemy_attacking;
	float m_enemy_attack_end_time;
	float m_enemy_heal_end_time;

	float m_player_attack_cooldown;
	float m_player_heal_cooldown;

	float m_enemy_attack_cooldown;
	float m_enemy_heal_cooldown;

	float m_player_damage_factor;
	bool m_is_enemy_dying;
	float m_enemy_dying_end_time;

	bool m_is_player_defeated;
	float m_player_defeated_end_time;

	bool m_is_rendering_particles;

	float m_player_attack_particles_time;
	float m_player_heal_particles_time;
	float m_enemy_attack_particles_time;
	float m_enemy_heal_particles_time;
	//btPairCachingGhostObject* ghostObject2;

	GLuint frameBufferProgramID;
	GLuint frameBufferObject;
	GLuint frameBufferTexture;
	GLuint frameBufferQuadVAO;
	GLuint frameBufferQuadVBO;
	GLuint depthBuffer;
	GLuint fbTextureUnif;
	GLuint fbTimeUnif;

	GLuint shadowFrameBuffer;
	GLuint fbDepthTexture;

	GLuint shadowsProgramID;
	GLuint shadowsMVPUnif;
	GLuint quadProgramID;
	GLuint quadTextureUnif;

	GLuint programID;
	GLuint samplerUnif;
	GLuint lightPositionUnif;
	GLuint kaUnif;
	GLuint kdUnif;
	GLuint ksUnif;
	GLuint shininessUnif;
	GLuint laUnif;
	GLuint ldUnif;
	GLuint lsUnif;
	GLuint modelViewMatrixUnif;
	GLuint normalMatrixUnif;
	GLuint projectionMatrixUnif;
	GLuint MVPUnif;
	GLuint boneMatricesUnif[MAX_BONES];
	GLuint isDynamicUnif;
	GLuint depthBiasMVPUnif;
	GLuint shadowMapUnif;

	GLuint skyboxProgramID;
	GLuint skyboxMVPUnif;
	GLuint skyboxSamplerUnif;
	glm::mat4 skyboxScale;

	GLuint particlesProgramID;
	GLuint particlesViewProjMatrixUnif;
	GLuint particlesPositionUnif;
	GLuint particlesCurTimeUnif;
	GLuint particlesStartTimeUnif;
	GLuint particlesTextureUnif;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 cameraPos;
	float cameraAngleHorizontal;
	float cameraAngleVertical;
	float cameraDistance;
	float cameraHeight;
	float cameraRotationSpeed;
};

#endif
