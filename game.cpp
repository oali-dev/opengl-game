#include <iostream>
#include "game.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "mesh_list.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

Game::Game()
	: m_skybox("interstellar_up.tga", "interstellar_dn.tga", "interstellar_rt.tga", 
			   "interstellar_lf.tga", "interstellar_bk.tga", "interstellar_ft.tga"),
	  m_gravity(9.81),
	  m_particle_system("explosion.png"),
	  m_particle_system2("starburst.jpg"),
	  m_particle_system_enemy("explosion.png"),
	  m_particle_system_enemy2("starburst.jpg"),
	  m_index(1),
	  m_is_encounter_initiated(false),
	  m_encountered_enemy(NULL),
	  m_encounter_distance(50.0),
	  m_is_showing_gui(false),
	  m_player_health(300),
	  m_enemy_health(200),
	  m_can_play_defend_sound(true),

	  m_is_player_attacking(false),
	  m_player_attack_end_time(0.0),
	  m_player_heal_end_time(0.0),
	  m_is_enemy_attacking(false),
	  m_enemy_attack_end_time(0.0),
	  m_enemy_heal_end_time(0.0),
	  m_player_attack_cooldown(3.0),
	  m_player_heal_cooldown(12.0),
	  m_enemy_attack_cooldown(4.0),
	  m_enemy_heal_cooldown(10.0),
	  m_player_damage_factor(1.0),
	  m_is_enemy_dying(false),
	  m_is_player_defeated(false),

	  m_is_rendering_particles(false),

	  m_player_attack_particles_time(0.0),
	  m_player_heal_particles_time(0.0),
	  m_enemy_attack_particles_time(0.0),
	  m_enemy_heal_particles_time(0.0),

	  cameraAngleHorizontal(0.0f),
	  cameraAngleVertical(30.0f),
	  cameraDistance(80.0f),
	  cameraHeight(80.0f),
	  cameraRotationSpeed(1.8f)
{
}

Game::~Game()
{
}

void Game::initializeProgram()
{
	//std::vector<GLuint> shaderList;

	//std::string vertexShaderCode = parseShader("vertexshader.glsl");
	//std::string fragmentShaderCode = parseShader("fragmentshader.glsl");

	//shaderList.push_back(createShader(GL_VERTEX_SHADER, vertexShaderCode));
	//shaderList.push_back(createShader(GL_FRAGMENT_SHADER, fragmentShaderCode));

	//programID = createProgram(shaderList);

	//programID = LoadShaders("diffuse.vert", "diffuse.frag");
	programID = LoadShaders("toon.vert", "toon.frag");
	skyboxProgramID = LoadShaders("skybox.vert", "skybox.frag");
	frameBufferProgramID = LoadShaders("sobel_outline.vert", "sobel_outline.frag");
	particlesProgramID = LoadShaders("particles.vert", "particles.frag");
	shadowsProgramID = LoadShaders("shadows.vert", "shadows.frag");
	quadProgramID = LoadShaders("quadshader.vert", "quadshader.frag");
	quadTextureUnif = glGetUniformLocation(quadProgramID, "texture");
	glUniform1i(quadTextureUnif, 0);

	samplerUnif = glGetUniformLocation(programID, "sampler");
	lightPositionUnif = glGetUniformLocation(programID, "LightPosition");
	kaUnif = glGetUniformLocation(programID, "Ka");
	kdUnif = glGetUniformLocation(programID, "Kd");
	ksUnif = glGetUniformLocation(programID, "Ks");
	shininessUnif = glGetUniformLocation(programID, "shininess");
	laUnif = glGetUniformLocation(programID, "La");
	ldUnif = glGetUniformLocation(programID, "Ld");
	lsUnif = glGetUniformLocation(programID, "Ls");
	modelViewMatrixUnif = glGetUniformLocation(programID, "ModelViewMatrix");
	normalMatrixUnif = glGetUniformLocation(programID, "NormalMatrix");
	projectionMatrixUnif = glGetUniformLocation(programID, "ProjectionMatrix");
	MVPUnif = glGetUniformLocation(programID, "MVP");
	depthBiasMVPUnif = glGetUniformLocation(programID, "depthBiasMVP");
	shadowMapUnif = glGetUniformLocation(programID, "shadowMap");

	skyboxMVPUnif = glGetUniformLocation(skyboxProgramID, "MVP");
	skyboxSamplerUnif = glGetUniformLocation(skyboxProgramID, "CubeMap");
	for(int i = 0; i < MAX_BONES; i++)
	{
		char Name[128];
		memset(Name, 0, sizeof(Name));
		SNPRINTF(Name, sizeof(Name), "boneMatrices[%d]", i);
		boneMatricesUnif[i] = glGetUniformLocation(programID, Name);
		//std::cerr << boneMatricesUnif[i] << std::endl;
		/*std::string boneMatricesPosition = "boneMatrices[";
		ss << i;
		boneMatricesPosition += ss.str();
		boneMatricesPosition += "]";
		ss.str(std::string());
		ss.clear();
		std::cerr << boneMatricesPosition << std::endl;
		boneMatricesUnif[i] = glGetUniformLocation(programID, boneMatricesPosition.c_str());*/
	}
	isDynamicUnif = glGetUniformLocation(programID, "isDynamic");

	fbTextureUnif = glGetUniformLocation(frameBufferProgramID, "renderedTexture");
	fbTimeUnif = glGetUniformLocation(frameBufferProgramID, "time");

	particlesViewProjMatrixUnif = glGetUniformLocation(particlesProgramID, "viewProjMatrix");
	particlesPositionUnif = glGetUniformLocation(particlesProgramID, "position");
	particlesCurTimeUnif = glGetUniformLocation(particlesProgramID, "curTime");
	particlesStartTimeUnif = glGetUniformLocation(particlesProgramID, "startTime");
	particlesTextureUnif = glGetUniformLocation(particlesProgramID, "texture");

	shadowsMVPUnif = glGetUniformLocation(shadowsProgramID, "MVP");
	
	glm::vec4 lightPosition(0.0f, 4.0f, 5.0f, 1.0f);
	glm::vec3 ka(0.6f, 0.8f, 0.8f);
	glm::vec3 kd(0.6f, 0.8f, 0.8f);
	glm::vec3 ks(0.6f, 0.8f, 0.9f);
	float shininess = 10.0;
	glm::vec3 la(0.6f, 0.8f, 0.8f);
	glm::vec3 ld(0.6f, 0.6f, 0.8f);
	glm::vec3 ls(0.7f, 0.7f, 0.9f);
	
	projectionMatrix = glm::perspective(45.0f, 4.0f/3.0f, 0.1f, 10000.f);

	glUseProgram(programID);
	glUniform1i(samplerUnif, 0);
	glUniform4fv(lightPositionUnif, 1, glm::value_ptr(lightPosition));
	glUniform3fv(kaUnif, 1, glm::value_ptr(ka));
	glUniform3fv(kdUnif, 1, glm::value_ptr(kd));
	glUniform3fv(ksUnif, 1, glm::value_ptr(ks));
	glUniform1f(shininessUnif, shininess);
	glUniform3fv(laUnif, 1, glm::value_ptr(la));
	glUniform3fv(ldUnif, 1, glm::value_ptr(ld));
	glUniform3fv(lsUnif, 1, glm::value_ptr(ls));
	glUniformMatrix4fv( projectionMatrixUnif, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform1i(shadowMapUnif, 1);
	glUseProgram(0);

	glUseProgram(skyboxProgramID);
	glUniform1i(skyboxSamplerUnif, 1);
	
	glUseProgram(frameBufferProgramID);
	glUniform1i(fbTextureUnif, 0);

	glUseProgram(particlesProgramID);
	glUniform1f(particlesStartTimeUnif, 0.0);
	glUniform1i(particlesTextureUnif, 0);

	glUseProgram(0);
	//std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}

void Game::display()
{
	// clear the screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use our shader program
	glUseProgram(programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbDepthTexture);
	glUniform1i(shadowMapUnif, 1);
	
	glm::mat4 biasMatrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f
	);
	glm::vec3 lightPosition(0.0f, 2000.0f, 5.0f);
	glm::mat4 dProjMatrix = glm::ortho<float>(-2000, 2000, -2000, 2000, -1, 2100);
	glm::mat4 dViewMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), 
										glm::vec3(0, 1, 0));

	viewMatrix = glm::lookAt(cameraPos, // world space pos of camera
						     m_player->getTransform() + glm::vec3(0.0f, 30.0f, 0.0f),// where to look at
							 glm::vec3(0.0, 1.0f, 0.0f));   // up direction
	glUniform1i(isDynamicUnif, 0);

	// Render all our static meshes
	for(std::vector<StaticRenderable*>::iterator it = m_static_renderables.begin();
		it != m_static_renderables.end();
		it++)
	{
		glm::mat4 modelMatrix = (*it)->getTranslation() * (*it)->getRotation() * (*it)->getScale();
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
		glm::mat4 MVP = projectionMatrix * modelViewMatrix;
		glm::mat4 dMVP = biasMatrix * dProjMatrix * dViewMatrix * modelMatrix;

		glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv( depthBiasMVPUnif, 1, GL_FALSE, glm::value_ptr(dMVP));

		(*it)->render();
	}

	//dProjMatrix = glm::ortho<float>(-2000, 2000, -2000, 2000, -1, 2300);
	glUniform1i(isDynamicUnif, 1);

	// Render all our dynamic meshes
	for(std::vector<DynamicRenderable*>::iterator it = m_dynamic_renderables.begin();
		it != m_dynamic_renderables.end();
		it++)
	{
		if((*it)->m_isVisible == false)
		{
			continue;
		}
		glm::mat4 modelMatrix = (*it)->getTranslation() * (*it)->getRotation() * (*it)->getScale();
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
		glm::mat4 MVP = projectionMatrix * modelViewMatrix;
		glm::mat4 dMVP = biasMatrix * dProjMatrix * dViewMatrix * modelMatrix;

		glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv( depthBiasMVPUnif, 1, GL_FALSE, glm::value_ptr(dMVP));
		
		(*it)->UpdateTransforms(m_clock.getElapsedTime().asSeconds());
		for(unsigned int i = 0; i < (*it)->getTransforms().size(); i++)
		{
			glUniformMatrix4fv(boneMatricesUnif[i], 1, GL_TRUE,
							   (const GLfloat*)(*it)->getTransforms()[i].m);
		}

		(*it)->render();
	}

	// render the player
	glm::mat4 modelMatrix = m_player->getTranslation() * m_player->getRotation() * m_player->getScale();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
	glm::mat4 MVP = projectionMatrix * modelViewMatrix;
	glm::mat4 dMVP = biasMatrix * dProjMatrix * dViewMatrix * modelMatrix;

	glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
	glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv( depthBiasMVPUnif, 1, GL_FALSE, glm::value_ptr(dMVP));
		
	m_player->UpdateTransforms(m_clock.getElapsedTime().asSeconds());
	for(unsigned int i = 0; i < m_player->getTransforms().size(); i++)
	{
		glUniformMatrix4fv(boneMatricesUnif[i], 1, GL_TRUE,
						   (const GLfloat*)m_player->getTransforms()[i].m);
	}

	m_player->render();

	// render the skybox
	glUseProgram(skyboxProgramID);
	glm::mat4 skyboxMVP = projectionMatrix * 
						  viewMatrix * 
						  glm::translate(glm::mat4(1.0f), cameraPos - glm::vec3(0, 20, 0)) *
						  skyboxScale;
	glUniformMatrix4fv(skyboxMVPUnif, 1, GL_FALSE, glm::value_ptr(skyboxMVP));
	m_skybox.render();

	glUseProgram(0);
}

int Game::startGame()
{
    // create the window
    m_window.create(sf::VideoMode(600, 600), "OpenGL", sf::Style::Default, sf::ContextSettings(32));
    m_window.setVerticalSyncEnabled(true);

	glewExperimental = true;
	if(glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

    // load resources, initialize the OpenGL states, ...
	init();

	gameLoop();

	return 0;
}

void Game::init()
{
	m_particle_system.init();
	m_particle_system2.init2();
	m_particle_system_enemy.init();
	m_particle_system_enemy2.init2();
	m_skybox.init();
	skyboxScale = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
	if(!m_battle_theme.openFromFile("battle_theme.ogg"))
	{
		std::cerr << "Could not load music" << std::endl;
	}
	else
	{
		m_battle_theme.setLoop(true);
	}
	if(!m_forest_theme.openFromFile("forest_theme.ogg"))
	{
		std::cerr << "Could not load music" << std::endl;
	}
	else
	{
		m_forest_theme.setLoop(true);
	}
	m_sword_attack.loadFromFile("Sword2.ogg");
	m_defend.loadFromFile("sword-unsheathe2.wav");
	m_heal.loadFromFile("spell.wav");
	m_bite.loadFromFile("bite-small.wav");

	m_gui_texture.loadFromFile("buttons2.png");
	m_gui_sprite.setTexture(m_gui_texture);
	cameraPos = glm::vec3(0.0f, 30.0f, 80.0f);
	
	loadAllMeshes();
	
	m_player = player_renderable;
	m_static_renderables = static_renderables;
	m_dynamic_renderables = dynamic_renderables;

	for(std::vector<btRigidBody*>::iterator it = static_rigidbodies.begin();
	    it != static_rigidbodies.end();
	    it++)
	{
		dynamicsWorld->addRigidBody( (*it) );
	}

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(15.0, 10.0, 0.0));
	ghostObject = new btPairCachingGhostObject();
	ghostObject->setWorldTransform(startTransform);
	overlappingPairCache->getOverlappingPairCache()->
		setInternalGhostPairCallback(new btGhostPairCallback());
	btConvexShape* capsule = new btCapsuleShape(2.0, 7.0);
	ghostObject->setCollisionShape(capsule);
	ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	btScalar stepHeight = btScalar(1.5);
	character = new btKinematicCharacterController(ghostObject, capsule, stepHeight);
	dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter, 
									  btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	dynamicsWorld->addAction(character);

	initializeProgram();
	initFramebuffer();
	//initializeVertexBuffer();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}

void Game::gameLoop()
{
	// start the game clock
	m_clock.restart();

    // run the main loop
    bool running = true;
    while (running)
    {
		dynamicsWorld->stepSimulation(1.0f/60.0f, 10);
		btTransform trans;
		trans = ghostObject->getWorldTransform();
		//fallRigidBody->getMotionState()->getWorldTransform(trans);
		//std::cerr << trans.getOrigin().getY() << std::endl;
		m_player->setTransform(trans.getOrigin().getX(), trans.getOrigin().getY() - 5.5f, 
							   trans.getOrigin().getZ());
		for(std::vector<DynamicRenderable*>::iterator it = m_dynamic_renderables.begin();
		    it != m_dynamic_renderables.end();
		    it++)
		{
			trans = (*it)->getController()->getWorldTransform();
			(*it)->setTransform(trans.getOrigin().getX(), trans.getOrigin().getY() - 5.5f, 
					    trans.getOrigin().getZ());	
			(*it)->move();	
		}

		if(!m_is_encounter_initiated)
		{
			m_player_health = 300;
			m_enemy_health = 200;
			m_player_damage_factor = 1.0;
			m_is_player_attacking = false;
			m_is_rendering_particles = false;
			m_is_showing_gui = false;
			if(m_forest_theme.getStatus() != sf::SoundSource::Playing)
			{
				m_battle_theme.stop();
				m_forest_theme.play();
			}
			for(std::vector<DynamicRenderable*>::iterator it = m_dynamic_renderables.begin();
				it != m_dynamic_renderables.end();
				it++)
			{
				float distance = glm::length(m_player->getTransform() - (*it)->getTransform());
				if(distance < m_encounter_distance && !m_is_enemy_dying && !m_is_player_defeated 
						  						   && (*it)->m_isVisible)
				{
					m_encountered_enemy = (*it);
					m_is_encounter_initiated = true;
					break;
				}
			}
		}
		else
		{
			m_is_rendering_particles = true;
			m_is_showing_gui = true;
			if(m_battle_theme.getStatus() != sf::SoundSource::Playing)
			{
				m_forest_theme.stop();
				m_battle_theme.play();
			}
			if(sf::Joystick::isButtonPressed(0, 0))
			{
				if(m_clock.getElapsedTime().asSeconds() > m_player_attack_end_time)
				{
					m_player_attack_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_player_attack_cooldown;
					m_is_player_attacking = true;
					m_player_attack_anim_end_time = m_clock.getElapsedTime().asSeconds() + 1.0f;
					m_sound.setBuffer(m_sword_attack);
					m_sound.play();
					m_enemy_health -= 20;
					m_player_attack_particles_time = m_clock.getElapsedTime().asSeconds();
				}
				m_can_play_defend_sound = true;
				m_player_damage_factor = 1.0;
				//std::cerr << "Button 0" << std::endl;
			}
			else if(sf::Joystick::isButtonPressed(0, 1))
			{
				if(m_can_play_defend_sound)
				{
					m_sound.setBuffer(m_defend);
					m_sound.play();
				}
				m_can_play_defend_sound = false;
				m_player_damage_factor = 0.5;
				//std::cerr << "Button 2" << std::endl;
			}
			else if(sf::Joystick::isButtonPressed(0, 2))
			{
				if(m_clock.getElapsedTime().asSeconds() > m_player_heal_end_time)
				{
					m_player_heal_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_player_heal_cooldown;
					m_sound.setBuffer(m_heal);
					m_sound.play();
					m_player_health += 10;
					std::cerr << m_player_health << std::endl;
					m_player_heal_particles_time = m_clock.getElapsedTime().asSeconds();
				}
				m_can_play_defend_sound = true;
				m_player_damage_factor = 1.0;
				//std::cerr << "Button 1" << std::endl;
			}
			else
			{
				m_can_play_defend_sound = true;
				m_player_damage_factor = 1.0;
			}
			if(m_clock.getElapsedTime().asSeconds() > m_player_attack_anim_end_time)
			{
				m_is_player_attacking = false;
			}
			if(m_player_health <= 0)
			{
				m_is_player_defeated = true;
				m_player_defeated_end_time = m_clock.getElapsedTime().asSeconds() + 4.0f;
				m_is_encounter_initiated = false;
			}

			if(m_enemy_health > 150)
			{
				if(m_clock.getElapsedTime().asSeconds() > m_enemy_attack_end_time)
				{
					m_enemy_attack_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_enemy_attack_cooldown;
					m_is_enemy_attacking = true;
					m_enemy_attack_anim_end_time = m_clock.getElapsedTime().asSeconds() + 1.5f;
					m_sound2.setBuffer(m_bite);
					m_sound2.play();
					m_player_health -= 20 * m_player_damage_factor;
					m_enemy_attack_particles_time = m_clock.getElapsedTime().asSeconds();
				}
				//std::cerr << "Button 0" << std::endl;
			}
			else if(m_enemy_health > 100 && m_clock.getElapsedTime().asSeconds() > 
											m_enemy_attack_end_time)
			{
				m_enemy_attack_end_time = m_clock.getElapsedTime().asSeconds() + 
										   m_enemy_attack_cooldown;
				m_is_enemy_attacking = true;
				m_enemy_attack_anim_end_time = m_clock.getElapsedTime().asSeconds() + 1.5f;
				m_sound2.setBuffer(m_bite);
				m_sound2.play();
				m_player_health -= 20 * m_player_damage_factor;
				m_enemy_attack_particles_time = m_clock.getElapsedTime().asSeconds();
			}
			else if(m_enemy_health > 100 && m_clock.getElapsedTime().asSeconds() <
											m_enemy_attack_end_time)
			{
				if(m_clock.getElapsedTime().asSeconds() > m_enemy_heal_end_time)
				{
					m_enemy_heal_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_enemy_heal_cooldown;
					m_sound2.setBuffer(m_heal);
					m_sound2.play();
					m_enemy_health += 10;
					m_enemy_heal_particles_time = m_clock.getElapsedTime().asSeconds();
				}
			}
			else if(m_enemy_health > 0 && m_enemy_health <= 100)
			{
				if(m_clock.getElapsedTime().asSeconds() > m_enemy_heal_end_time)
				{
					m_enemy_heal_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_enemy_heal_cooldown;
					m_sound2.setBuffer(m_heal);
					m_sound2.play();
					m_enemy_health += 10;
					m_enemy_heal_particles_time = m_clock.getElapsedTime().asSeconds();
				}
				else if(m_clock.getElapsedTime().asSeconds() > m_enemy_attack_end_time)
				{
					m_enemy_attack_end_time = m_clock.getElapsedTime().asSeconds() + 
											   m_enemy_attack_cooldown;
					m_is_enemy_attacking = true;
					m_enemy_attack_anim_end_time = m_clock.getElapsedTime().asSeconds() + 1.5f;
					m_sound2.setBuffer(m_bite);
					m_sound2.play();
					m_player_health -= 30 * m_player_damage_factor;
					m_enemy_attack_particles_time = m_clock.getElapsedTime().asSeconds();
				}
			}
			else if(m_enemy_health <= 0)
			{
				m_is_enemy_dying = true;
				m_enemy_dying_end_time = m_clock.getElapsedTime().asSeconds() + 4.0f;
				m_is_encounter_initiated = false;
			}
			if(m_clock.getElapsedTime().asSeconds() > m_enemy_attack_anim_end_time)
			{
				if(m_encountered_enemy != NULL)
					m_encountered_enemy->setAnimation(0);
				m_is_enemy_attacking = false;
			}
		}
		//trans = ghostObject2->getWorldTransform();
		//snake_renderable->setTransform(trans.getOrigin().getX(), trans.getOrigin().getY() - 
		//			                   5.5f, trans.getOrigin().getZ());
		//snake_renderable->move();
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		{
			// end the program
			running = false;
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			btTransform original_position;
			original_position.setIdentity();
			original_position.setOrigin(btVector3(15.0, -45.9, 0.0078));
			ghostObject->setWorldTransform(original_position);
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			m_is_encounter_initiated = true;
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			m_is_encounter_initiated = false;
		}

		float axisPosX = sf::Joystick::getAxisPosition(0, sf::Joystick::X) / 100.0f;
		float axisPosY = sf::Joystick::getAxisPosition(0, sf::Joystick::Y) / 100.0f;
		btVector3 movement_direction(axisPosX / 1.5f, btScalar(0), axisPosY / 1.5f);
	
		// The joysticks are very sensitive, so we set the movement direction to 0
		// if the axis position is below a certain threshold
		if(fabs(movement_direction.getX()) < 0.2)
		{
			movement_direction.setX(btScalar(0));
		}
		if(fabs(movement_direction.getZ()) < 0.2)
		{
			movement_direction.setZ(btScalar(0));
		}
		if(movement_direction.getX() || movement_direction.getZ())
		{
			m_player->setRotation(atan2(movement_direction.getZ(), 
										 -movement_direction.getX()) - 3.14f/2.0f + 
										 cameraAngleHorizontal * 3.14f/180.0f);
		}
		
		float cameraAxisX = sf::Joystick::getAxisPosition(0, sf::Joystick::U) / 100.0f;
		float cameraAxisY = sf::Joystick::getAxisPosition(0, sf::Joystick::V) / 100.0f;

		if(fabs(cameraAxisX) > 0.2f)
		{
			cameraAngleHorizontal -= cameraAxisX * cameraRotationSpeed;
		}
		if(fabs(cameraAxisY) > 0.2f)
		{
			cameraAngleVertical += cameraAxisY * cameraRotationSpeed;
			if(cameraAngleVertical > 90.0f)
				cameraAngleVertical = 90.0f;
			if(cameraAngleVertical < 0.0f)
				cameraAngleVertical = 0.0f;
		}
		/*if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			//mesh2.AnimateNodes(m_clock.getElapsedTime().asSeconds());
			movement_direction.setZ(-0.5);
			//character->setWalkDirection(btVector3(0.0, 0.0, -0.5));
			//fallRigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, -1.0f));
			m_player->rotateForward();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			movement_direction.setZ(0.5);
			//character->setWalkDirection(btVector3(0.0, 0.0, 0.5));
			m_player->rotateBackward();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			movement_direction.setX(-0.5);
			//character->setWalkDirection(btVector3(-0.5, 0.0, 0.0));
			m_player->rotateLeft();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			movement_direction.setX(0.5);
			//character->setWalkDirection(btVector3(0.5, 0.0, 0.0));
			m_player->rotateRight();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			//character->jump();
		}*/
		if(m_is_encounter_initiated)
		{
			m_player->setAnimation(2);
		}
		else if(movement_direction.isZero())
		{
			m_player->setAnimation(0);
			character->setGravity(btScalar(0));
		}
		else
		{
			m_player->setAnimation(1);
			character->setGravity(m_gravity);
		}

		if(m_is_player_attacking)
		{
			m_player->setAnimation(3);
		}
		if(m_is_enemy_attacking)
		{
			if(m_encountered_enemy != NULL)
				m_encountered_enemy->setAnimation(2);
		}
		if(m_is_enemy_dying)
		{
			m_encountered_enemy->setAnimation(3);
			if(m_clock.getElapsedTime().asSeconds() > m_enemy_dying_end_time)
			{
				m_is_enemy_dying = false;
				m_encountered_enemy->m_isVisible = false;
				m_encountered_enemy = NULL;
			}
		}
		if(m_is_player_defeated)
		{
			m_player->setAnimation(4);
			if(m_clock.getElapsedTime().asSeconds() > m_player_defeated_end_time)
			{
				m_is_player_defeated = false;
				m_encountered_enemy->setAnimation(0);
				m_encountered_enemy = NULL;
				btTransform original_position;
				original_position.setIdentity();
				original_position.setOrigin(btVector3(15.0, -45.9, 0.0078));
				ghostObject->setWorldTransform(original_position);
			}
		}

		/*if(sf::Joystick::isButtonPressed(0, 2))
		{
			glUseProgram(particlesProgramID);
			glUniform1f(particlesStartTimeUnif, m_clock.getElapsedTime().asSeconds());
			glUseProgram(0);
		}
		if(sf::Joystick::isButtonPressed(0, 1))
			m_player->setAnimation(3);*/

		btQuaternion camera_orientation(btVector3(0.0f, 1.0f, 0.0f), cameraAngleHorizontal * 3.14f/180.0f);
		btMatrix3x3 rotationMatrix(camera_orientation);
		movement_direction = rotationMatrix * movement_direction; 

		if(!m_is_encounter_initiated && !m_is_player_defeated)
		{
			character->setWalkDirection(movement_direction * 2);
			m_player->move();
		}
		else
		{
			character->setGravity(btScalar(0));
			character->setWalkDirection(btVector3(0, 0, 0));
		}

		cameraPos.x = cameraDistance * sin(cameraAngleHorizontal * 3.14f/180.0f) + m_player->getTransform().x;
		cameraPos.y = cameraHeight * sin(cameraAngleVertical * 3.14f/180.0f) + 
					  m_player->getTransform().y * 0.7;
		cameraPos.z = cameraDistance * cos(cameraAngleHorizontal * 3.14f/180.0f) + m_player->getTransform().z;

        // handle events
        sf::Event event;
        while (m_window.pollEvent(event))
        {
			if (event.type == sf::Event::Closed)
            {
                // end the program
                running = false;
            }
            else if (event.type == sf::Event::Resized)
            {
				float scaleX = sf::VideoMode::getDesktopMode().width * 0.4 / m_window.getSize().x;
				float scaleY = sf::VideoMode::getDesktopMode().height * 0.65 / m_window.getSize().y;
				m_gui_sprite.setScale(scaleX, scaleY);

                // adjust the viewport when the window is resized
				projectionMatrix = glm::perspective(45.0f, 
													(float)event.size.width / (float)event.size.height, 
													0.1f, 
													10000.f);
				glUseProgram(programID);
				glUniformMatrix4fv( projectionMatrixUnif, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
				glUseProgram(0);

				glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, event.size.width, event.size.height,
				 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
				glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
									  m_window.getSize().x, m_window.getSize().y);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

                glViewport(0, 0, event.size.width, event.size.height);
				//initFramebuffer();
            }

        }

		renderDepthMap();
        // display our image
		display();
	
		// write and display framebuffer contents
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		display();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(frameBufferProgramID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, frameBufferQuadVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		glUseProgram(0);
		glEnable(GL_BLEND);

		glUseProgram(quadProgramID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbDepthTexture);

		/*glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, frameBufferQuadVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		glUseProgram(0);*/


		//renderParticles(m_player->getTransform() + glm::vec3(0.0f, 8.0f, 0.0f), m_index);
		if(m_is_rendering_particles)
		{
			renderPlayerAttackParticles();
			renderPlayerHealParticles();
			renderEnemyAttackParticles();
			renderEnemyHealParticles();
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		if(m_is_showing_gui)
		{
			m_window.pushGLStates();
			m_window.draw(m_gui_sprite);
			m_window.popGLStates();
		}
        // end the current frame (internally swaps the front and back buffers)
        m_window.display();
    }

    // release resources...
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}

void Game::renderPlayerAttackParticles()
{
	glUseProgram(particlesProgramID);
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(particlesViewProjMatrixUnif, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));
	glUniform1f(particlesCurTimeUnif, m_clock.getElapsedTime().asSeconds());
	glUniform1f(particlesStartTimeUnif, m_player_attack_particles_time);

	glUniform3fv(particlesPositionUnif, 1, glm::value_ptr(m_encountered_enemy->getTransform()));

	m_particle_system.render(20.0f);

	glUseProgram(0);
}

void Game::renderPlayerHealParticles()
{
	glUseProgram(particlesProgramID);
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(particlesViewProjMatrixUnif, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));
	glUniform1f(particlesCurTimeUnif, m_clock.getElapsedTime().asSeconds());
	glUniform1f(particlesStartTimeUnif, m_player_heal_particles_time);

	glUniform3fv(particlesPositionUnif, 1, glm::value_ptr(m_player->getTransform()));

	m_particle_system2.render(15.0f);

	glUseProgram(0);
}

void Game::renderEnemyAttackParticles()
{
	glUseProgram(particlesProgramID);
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(particlesViewProjMatrixUnif, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));
	glUniform1f(particlesCurTimeUnif, m_clock.getElapsedTime().asSeconds());
	glUniform1f(particlesStartTimeUnif, m_enemy_attack_particles_time);

	glUniform3fv(particlesPositionUnif, 1, glm::value_ptr(m_player->getTransform()));

	m_particle_system_enemy.render(20.0f);

	glUseProgram(0);
}

void Game::renderEnemyHealParticles()
{
	glUseProgram(particlesProgramID);
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(particlesViewProjMatrixUnif, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));
	glUniform1f(particlesCurTimeUnif, m_clock.getElapsedTime().asSeconds());
	glUniform1f(particlesStartTimeUnif, m_enemy_heal_particles_time);

	glUniform3fv(particlesPositionUnif, 1, glm::value_ptr(m_encountered_enemy->getTransform() +
														  glm::vec3(0.0f, 20.0f, 0.0f)));

	m_particle_system_enemy2.render(25.0f);

	glUseProgram(0);
}

void Game::initFramebuffer()
{
	// Setting up frame buffer
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

	// Setting up frame buffer texture
	glGenTextures(1, &frameBufferTexture);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_window.getSize().x, m_window.getSize().y,
				 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Setting up the depth buffer
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
						  m_window.getSize().x, m_window.getSize().y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBufferTexture, 0);

	// Setting up the draw buffer
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Could not initialize framebuffer." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);

	// Setting up depth texture for shadows
	glGenTextures(1, &fbDepthTexture);
	glBindTexture(GL_TEXTURE_2D, fbDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 2048, 2048, 0, GL_DEPTH_COMPONENT,
				 GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbDepthTexture, 0);
	glDrawBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Could not initialize shadow framebuffer." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setting up our screen quad for rendering
	glGenVertexArrays(1, &frameBufferQuadVAO);
	glBindVertexArray(frameBufferQuadVAO);

	const GLfloat frameBufferQuadVertices[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	glGenBuffers(1, &frameBufferQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, frameBufferQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frameBufferQuadVertices), 
				 frameBufferQuadVertices, GL_STATIC_DRAW);
}

GLuint Game::createShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch(eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	}

	return shader;
}

GLuint Game::createProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}

std::string Game::parseShader(const char* filename)
{
	std::string ShaderCode;
	std::ifstream ShaderStream(filename, std::ios::in);
	if(ShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(ShaderStream, Line))
		{
			ShaderCode += "\n" + Line;
		}
		ShaderStream.close();
	}
	else
	{
		std::cerr << "Could not open shader file" << std::endl;
		return "";
	}

	return ShaderCode;
}


void Game::renderDepthMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 2048, 2048);
	//glCullFace(GL_FRONT);
	glUseProgram(shadowsProgramID);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbDepthTexture);
	glUniform1i(shadowMapUnif, 1);*/

	glm::vec3 lightPosition(0.0f, 2000.0f, 5.0f);
	glm::mat4 dProjMatrix = glm::ortho<float>(-2000, 2000, -2000, 2000, -1, 2100);
	//glm::mat4 dProjMatrix = glm::perspective(45.0f, 1.0f, 1.0f, 15.0f);
	glm::mat4 dViewMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), 
										glm::vec3(0, 1, 0));
	/*glm::mat4 biasMatrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f
	);
	glm::mat4 depthBiasMVP = biasMatrix * dMVP;
	glUniformMatrix4fv(depthBiasMVPUnif, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));*/

	/*viewMatrix = glm::lookAt(cameraPos, // world space pos of camera
						     m_player->getTransform() + glm::vec3(0.0f, 30.0f, 0.0f),// where to look at
							 glm::vec3(0.0, 1.0f, 0.0f));   // up direction
	glUniform1i(isDynamicUnif, 0);*/

	// Render all our static meshes
	for(std::vector<StaticRenderable*>::iterator it = m_static_renderables.begin();
		it != m_static_renderables.end();
		it++)
	{
		glm::mat4 modelMatrix = (*it)->getTranslation() * (*it)->getRotation() * (*it)->getScale();
		glm::mat4 MVP = dProjMatrix * dViewMatrix * modelMatrix;
		//glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		//glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
		//glm::mat4 MVP = projectionMatrix * modelViewMatrix;

		//glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		//glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		//glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(shadowsMVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));

		(*it)->render();
	}

	//glUniform1i(isDynamicUnif, 1);

	//dProjMatrix = glm::ortho<float>(-2000, 2000, -2000, 2000, -1, 2300);
	// Render all our dynamic meshes
	//for(std::vector<DynamicRenderable*>::iterator it = m_dynamic_renderables.begin();
	//	it != m_dynamic_renderables.end();
	//	it++)
	//{
		//glm::mat4 modelMatrix = (*it)->getTranslation() * (*it)->getRotation() * (*it)->getScale();
		//glm::mat4 MVP = dProjMatrix * dViewMatrix * modelMatrix;
		//glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		//glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
		//glm::mat4 MVP = projectionMatrix * modelViewMatrix;

		//glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		//glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		//glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		//glUniformMatrix4fv(shadowsMVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		
		/*(*it)->UpdateTransforms(m_clock.getElapsedTime().asSeconds());
		for(unsigned int i = 0; i < (*it)->getTransforms().size(); i++)
		{
			glUniformMatrix4fv(boneMatricesUnif[i], 1, GL_TRUE,
							   (const GLfloat*)(*it)->getTransforms()[i].m);
		}*/

		//(*it)->render();
	//}

	//glm::mat4 modelMatrix = m_player->getTranslation() * m_player->getRotation() * m_player->getScale();
	//glm::mat4 MVP = dProjMatrix * dViewMatrix * modelMatrix;
	//glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	//glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
	//glm::mat4 MVP = projectionMatrix * modelViewMatrix;

	//glUniformMatrix4fv( modelViewMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
	//glUniformMatrix3fv( normalMatrixUnif, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	//glUniformMatrix4fv( MVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
	//glUniformMatrix4fv(shadowsMVPUnif, 1, GL_FALSE, glm::value_ptr(MVP));
		
	/*m_player->UpdateTransforms(m_clock.getElapsedTime().asSeconds());
	for(unsigned int i = 0; i < m_player->getTransforms().size(); i++)
	{
		glUniformMatrix4fv(boneMatricesUnif[i], 1, GL_TRUE,
						   (const GLfloat*)m_player->getTransforms()[i].m);
	}*/

	//m_player->render();

	/*glm::mat4 skyboxMVP = projectionMatrix * 
						  viewMatrix * 
						  glm::translate(glm::mat4(1.0f), cameraPos - glm::vec3(0, 20, 0)) *
						  skyboxScale;
	glUniformMatrix4fv(skyboxMVPUnif, 1, GL_FALSE, glm::value_ptr(skyboxMVP));
	m_skybox.render();*/

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_window.getSize().x, m_window.getSize().y);
	//glCullFace(GL_BACK);
}
