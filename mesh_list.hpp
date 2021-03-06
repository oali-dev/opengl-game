/* This is a file that contains all the mesh and renderable information for the game 
scene. It also contains the physics world, so we can do all the adding of game objects
to the world strictly in here instead of having it take up a huge amount of space in 
the game's init code. */

#ifndef MESH_LIST_HPP
#define MESH_LIST_HPP

#include <vector>
#include "mesh.hpp"
#include "renderable.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

std::vector<StaticRenderable*> static_renderables;
std::vector<DynamicRenderable*> dynamic_renderables;
std::vector<btRigidBody*> static_rigidbodies;
std::vector<btPairCachingGhostObject*> dynamic_object_controllers;

TerrainMesh terrain_mesh;
TerrainMesh pine_mesh;
TerrainMesh pine_mesh_large;
TerrainMesh oak_mesh;
TerrainMesh oak_mesh_large;
TerrainMesh small_tree_mesh;
TerrainMesh medium_tree_mesh;

TerrainMesh ivy_leaf_mesh;
TerrainMesh oak_leaf_mesh;
TerrainMesh maple_leaf_mesh;
TerrainMesh popular_leaf_mesh;
TerrainMesh leaves_mesh;

Mesh player_walk;
Mesh player_idle;
Mesh player_battle_idle;
Mesh player_attack;
Mesh player_defeat;

Mesh snake_walk;
Mesh snake_idle;
Mesh snake_attack;
Mesh snake_death;
Mesh snake_death_still;

DynamicRenderable* player_renderable;

btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btBroadphaseInterface* overlappingPairCache;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;
btPairCachingGhostObject* ghostObject;	// this ghost object is special since it belongs to the player
btKinematicCharacterController* character;

void addStaticObjectToWorld(TerrainMesh& mesh, glm::vec3 translation, glm::vec3 rotation, 
			    glm::vec3 scale, btBvhTriangleMeshShape* shape)
{
	StaticRenderable* renderable = new StaticRenderable(&mesh, translation, rotation, scale, true);
	static_renderables.push_back(renderable);
	
	btDefaultMotionState* motionState = 
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), 
						     btVector3(translation.x, 
						     	       translation.y, 
						     	       translation.z)));
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape,
							     btVector3(0, 0, 0));
	btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
	static_rigidbodies.push_back(rigidBody);
}

void addDynamicObjectToWorld(std::vector<Mesh*>& meshes, glm::vec3 translation, 
			     glm::vec3 rotation, glm::vec3 scale, float colliderRadius, 
			     float colliderHeight)
{
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(translation.x, translation.y, translation.z));
	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	ghostObject->setWorldTransform(startTransform);
	overlappingPairCache->getOverlappingPairCache()->
		setInternalGhostPairCallback(new btGhostPairCallback());
	btConvexShape* capsule = new btCapsuleShape(colliderRadius, colliderHeight);
	ghostObject->setCollisionShape(capsule);
	ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	btScalar stepHeight = btScalar(1.5);
	btKinematicCharacterController* controller = new btKinematicCharacterController(ghostObject, 
										capsule, stepHeight);
	dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter, 
		       btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	dynamicsWorld->addAction(controller);
	
	DynamicRenderable* renderable = new DynamicRenderable(meshes, translation, 
							      rotation, scale, true, ghostObject);
	dynamic_renderables.push_back(renderable);	
}

void loadAllMeshes()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, 
						    solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	terrain_mesh.loadMesh("terrain2.obj");
	pine_mesh.loadMesh("pine_tree.obj");
	pine_mesh_large.loadMesh("pine_tree_large.obj");
	oak_mesh.loadMesh("oak_tree.obj");
	oak_mesh_large.loadMesh("oak_tree_large.obj");
	small_tree_mesh.loadMesh("small_tree.obj");
	
	// Decided to replace the leaves with ferns, since leaves don't need
	// rigid bodies and I can just import them all as one mesh. The names
	// are left the same since going back and changing all of them will take
	// a fairly long time
	leaves_mesh.loadMesh("leaves.obj");
	ivy_leaf_mesh.loadMesh("fern1.obj");
	oak_leaf_mesh.loadMesh("fern2.obj");
	maple_leaf_mesh.loadMesh("fern3.obj");
	popular_leaf_mesh.loadMesh("fern4.obj");
	
	player_idle.loadMesh("frog_idle.dae");
	player_walk.loadMesh("frog_walk.dae");
	player_battle_idle.loadMesh("frog_battle_idle.dae");
	player_attack.loadMesh("frog_attack.dae");
	player_defeat.loadMesh("frog_defeat.dae");
	
	std::vector<Mesh*> meshes;
	meshes.push_back(&player_idle);
	meshes.push_back(&player_walk);
	meshes.push_back(&player_battle_idle);
	meshes.push_back(&player_attack);	
	meshes.push_back(&player_defeat);	
	
	snake_idle.loadMesh("snake_idle.dae");
	snake_walk.loadMesh("snake_walk.dae");
	snake_attack.loadMesh("snake_attack.dae");
	snake_death.loadMesh("snake_death.dae");
	snake_death_still.loadMesh("snake_death_still.dae");
	
	std::vector<Mesh*> meshes2;
	meshes2.push_back(&snake_idle);
	meshes2.push_back(&snake_walk);
	meshes2.push_back(&snake_attack);
	meshes2.push_back(&snake_death);
	meshes2.push_back(&snake_death_still);

	btBvhTriangleMeshShape* groundShape = terrain_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* pineShape = pine_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* pineShapeLarge = pine_mesh_large.getTerrainCollisionBody();
	btBvhTriangleMeshShape* oakShape = oak_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* oakLargeShape = oak_mesh_large.getTerrainCollisionBody();
	btBvhTriangleMeshShape* smallTreeShape = small_tree_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* mediumTreeShape = medium_tree_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* ivyLeafShape = ivy_leaf_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* oakLeafShape = oak_leaf_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* mapleLeafShape = maple_leaf_mesh.getTerrainCollisionBody();
	btBvhTriangleMeshShape* popularLeafShape = popular_leaf_mesh.getTerrainCollisionBody();

	player_renderable = new DynamicRenderable(meshes, glm::vec3(0.0f, 0.0f, 0.0f), 
			    glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f), true,
			    ghostObject);
	addDynamicObjectToWorld(meshes2, glm::vec3(-564.0f, 87.0f, -468.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(-427.0f, 87.0f, 605.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(-694.0f, 151.0f, 342.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(421.0f, 108.0f, 936.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(1181.0f, 87.0f, 43.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(-440.0f, 132.0f, -982.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(644.0f, -33.0f, -215.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(-1089.0f, 58.0f, 552.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(203.0f, 60.0f, -749.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	addDynamicObjectToWorld(meshes2, glm::vec3(861.0f, 60.0f, -664.0f), 
				glm::vec3(-3.14f/2.0f, 0.0f, 0.0f), glm::vec3(7.0f, 7.0f, 7.0f),
				2.0, 7.0);
	

	StaticRenderable* leaves_renderable = new StaticRenderable(&leaves_mesh, 
	glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), true);
	static_renderables.push_back(leaves_renderable);

	addStaticObjectToWorld(terrain_mesh, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 
			       glm::vec3(1.0f, 1.0f, 1.0f), groundShape);
			       
	addStaticObjectToWorld(oak_mesh, glm::vec3(282.67599f, -79.71706f, -143.08386f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(1158.72510f, -56.0f, 477.04410f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(-127.6f, 13.8f, -668.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(1147.6f, 20.02f, -1072.6f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(460.6f, 74.1f, 566.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(312.9f, 44.3f, -1067.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);	
	addStaticObjectToWorld(oak_mesh, glm::vec3(171.9f, 48.4f, -1113.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(1292.9f, -84.0f, -143.1f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(1158.7f, 24.0f, 1234.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(828.0f, 34.55f, 1220.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
	addStaticObjectToWorld(oak_mesh, glm::vec3(538.7f, 70.9f, 1220.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakShape);
		
	addStaticObjectToWorld(pine_mesh, glm::vec3(551.3f, 63.3f, 501.6f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(621.5f, -96.2f, -346.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(625.5f, -54.3f, 156.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(-44.0f, -30.25f, 406.4f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(405.3f, 70.36f, 624.4f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(498.4f, 73.2f, 616.44f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(1258.11f, -97.5f, 766.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
	addStaticObjectToWorld(pine_mesh, glm::vec3(422.2f, -52.7f, -649.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShape);
		
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(1337.75f, -20.02f, 1090.74f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(610.47f, -115.2f, -1287.23f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(814.1f, -138.5f, -1299.05f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(767.3f, -138.5f, -1151.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(347.7f, -138.5f, -1316.7f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(1284.6f, 6.3f, -819.95f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-314.77f, 17.8f, -185.03f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1335.85f, -47.35f, 833.64f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1158.81f, 9.11f, 331.99f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1326.75f, -31.5f, 464.54f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1198.86f, 26.81f, -325.08f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1294.39f, 5.25f, 44.15f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
	addStaticObjectToWorld(pine_mesh_large, glm::vec3(-1073.55f, 25.89f, -614.43f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), pineShapeLarge);
		
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(-256.25f, -6.35f, 109.86f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(-47.3f, 7.2f, -356.18f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(-159.8f, -25.55f, 203.7f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(-180.5f, -14.1f, 162.76f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(-210.67f, -27.0f, 97.05f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(931.9f, -58.0f, -22.14f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(755.83f, 47.2f, 455.93f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(771.51953, 31.26f, 414.83f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(small_tree_mesh, glm::vec3(837.76, 11.84f, 908.6f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), smallTreeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(500.22f, -54.66f, 61.58f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-544.88f, 106.7f, 351.42f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-628.41f, 39.96f, 578.12f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-744.98f, 96.0f, 209.13f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-574.58f, 8.13f, -114.2f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
		
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(402.92f, 69.6f, 1241.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(670.4f, 58.5f, 1104.4f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(528.05f, 24.8f, 1327.45f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-951.87f, 70.0f, 39.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-854.89f, 48.22f, -229.14f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-843.36f, 61.88f, -514.42f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-669.17f, 36.86f, -414.97f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-1269.15f, -2.04f, -1112.34f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-1028.02f, 20.14f, -1200.11f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-746.92f, -51.42f, 1174.91f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-733.71f, 88.41f, 1280.56f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-588.51f, 61.58f, 1265.1f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-442.82f, 45.64f, 1330.96f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-204.93f, 45.64f, 1267.55f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-422.15f, 16.79f, 1157.98f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-699.6f, 25.6f, 981.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-162.1f, -112.93f, 1102.16f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-290.6f, -80.23f, 885.17f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-556.87f, 36.86f, -808.92f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-464.6f, 96.68f, -1327.1f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-280.48f, 34.2f, -1260.4f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(-105.7f, 24.94f, -1355.7f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(48.97f, 24.94f, -1292.7f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(380.69f, 48.62f, -1111.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(547.069f, 37.77f, -1040.54f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(1223.58f, -24.0f, -939.2f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(894.5f, 59.5f, -997.24f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(670.48f, -6.42f, -928.8f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
	addStaticObjectToWorld(oak_mesh_large, glm::vec3(844.66f, -6.42f, -908.175f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLargeShape);
		
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(224.43f, 61.15f, 704.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(-375.84f, 69.05f, 654.6f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(-558.8f, 28.04f, 96.4f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(-728.93f, 66.61f, -657.17f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(-540.8f, 45.1f, -677.03f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
	addStaticObjectToWorld(medium_tree_mesh, glm::vec3(972.6f, -96.61f, -360.64f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mediumTreeShape);
		
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(255.28f, -71.05f, -55.68f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(71.33f, -27.15f, -134.8f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(408.08f, -19.61f, 204.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(475.7f, -67.2f, -49.3f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(311.22f, -61.05f, -433.07f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(-146.97f, 8.53f, -476.15f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(-170.46f, -28.235f, -33.63f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(121.84f, -17.96f, -518.55f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
	addStaticObjectToWorld(ivy_leaf_mesh, glm::vec3(212.66f, -44.85f, 595.73f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), ivyLeafShape);
		
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(83.98f, -34.66f, -121.44f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(75.488f, -55.46f, 16.83f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(187.94f, -59.24f, -26.95f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(134.64f, -50.7077f, 185.305), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(-262.733f, 36.52f, -261.078f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(231.476f, -34.6f, -649.975f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(149.53f, 20.3f, 558.695f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(1214.8f, -87.89f, -420.53f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(1296.746f, -12.43f, 157.17f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(891.124f, 6.25f, 644.736f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(809.18f, 18.1f, 812.72f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(1095.98f, 36.29f, 1021.68f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(347.45f, 52.125f, 535.68f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);
	addStaticObjectToWorld(maple_leaf_mesh, glm::vec3(-165.58f, 88.15f, 669.07f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), mapleLeafShape);

	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(0.0f, -43.265f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(122.8f, -36.195f, -176.49f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(213.17f, 51.22f, 616.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(-247.62f, 59.36f, 662.17f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(412.43f, 77.92f, 1015.03f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(321.1f, 89.366f, 1193.54f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(76.18f, 83.67f, 1010.88f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(-753.97f, 28.65f, 809.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(657.66f, -49.24f, -592.24f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(73.7f, 22.92f, -911.41f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(-189.89f, -9.54f, -508.46f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(1230.28f, -40.6f, -638.997f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(401.8f, 86.2f, 1073.46f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(104.15f, 72.77f, 847.11f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	addStaticObjectToWorld(oak_leaf_mesh, glm::vec3(270.33f, 30.88f, 490.69f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), oakLeafShape);
	
	addStaticObjectToWorld(popular_leaf_mesh, glm::vec3(192.0f, -54.7f, 55.11f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), popularLeafShape);
	addStaticObjectToWorld(popular_leaf_mesh, glm::vec3(317.0f, 28.24f, 434.9f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), popularLeafShape);
	addStaticObjectToWorld(popular_leaf_mesh, glm::vec3(1051.0f, -78.29f, -381.43f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), popularLeafShape);
	addStaticObjectToWorld(popular_leaf_mesh, glm::vec3(1121.45f, -24.5f, 905.07f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), popularLeafShape);
	addStaticObjectToWorld(popular_leaf_mesh, glm::vec3(-217.86f, 46.16f, 587.86f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), popularLeafShape);
			
	/*renderable3 = new StaticRenderable(&mesh2, glm::vec3(30.0f, 0.0f, 30.0f),
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), true);
			
	renderable4 = new StaticRenderable(&mesh2, glm::vec3(-30.0f, 0.0f, -30.0f),
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), true);*/
			
	// Pushing back the renderables
	//dynamic_renderables.push_back(player_renderable);
	//dynamic_renderables.push_back(snake_renderable);
}

#endif
