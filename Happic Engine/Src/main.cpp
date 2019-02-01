#include "Core\CoreEngine.h"
#include "ECS\Components\Components.h"
#include "ECS\Systems\Systems.h"

using namespace Happic;
using namespace Rendering;
using namespace Math;
using namespace Core;
using namespace ECS;

class TestGame : public Application
{
public:
	void Init() override;
	void FreeResources() override;
private:
	Mesh				m_quad1;
	Mesh				m_quad2;
	Mesh				m_monkey;
	ITexture2D*			m_pBricks;
	ITexture2D*			m_pBricks2;
	ITextureSampler*	m_pTextureSampler;
};

void TestGame::Init()
{
	m_ecs.CreateSystem<MeshSubmitionSystem>(SYSTEM_PROCESS_ON_RENDER);
	m_ecs.CreateSystem<ViewProjectionSystem>(SYSTEM_PROCESS_ON_UPDATE);
	m_ecs.CreateSystem<KeyboardMovementSystem>(SYSTEM_PROCESS_ON_UPDATE);
	m_ecs.CreateSystem<MouseLookAroundSystem>(SYSTEM_PROCESS_ON_UPDATE);
	m_ecs.CreateSystem<RotationSystem>(SYSTEM_PROCESS_ON_UPDATE);
	
	Vertex verticies[] = {	Vertex(Vector3f(-1.0f, -1.0f, 0.0f), Vector2f(0.0f, 1.0f)),
							Vertex(Vector3f(-1.0f, 1.0f, 0.0f), Vector2f(0.0f, 0.0f)), 
							Vertex(Vector3f(1.0f, 1.0f, 0.0f), Vector2f(1.0f, 0.0f)), 
							Vertex(Vector3f(1.0f, -1.0f, 0.0f), Vector2f(1.0f, 1.0f))  };

	uint32 indices[] = { 0, 1, 2, 0, 2, 3 };

	m_pTextureSampler = ITextureSampler::CreateTextureSampler(FILTER_LINEAR, ADDRESS_MODE_REPEAT, ANISOTROPY_16);
	m_pBricks = ITexture2D::CreateTexture("Res/Textures/Bricks.jpg");
	m_pBricks2 = ITexture2D::CreateTexture("Res/Textures/Bricks2.jpg");

	Material bricks(m_pTextureSampler, m_pBricks);
	Material bricks2(m_pTextureSampler, m_pBricks2);

	m_quad1 = Mesh(verticies, 4, indices, 6, bricks);
	m_quad2 = Mesh(verticies, 4, indices, 6, bricks2);

	EntityID quadEntity1 = m_ecs.CreateEntity();
	m_ecs.CreateComponent<MeshComponent>(quadEntity1, m_quad1);
	m_ecs.CreateComponent<TransformComponent>(quadEntity1, Transform(Vector3f(-1.5f, 0.0f, 0.0f)));
	m_ecs.GetComponent<TransformComponent>(quadEntity1)->transform.Scale(0.5f);

	EntityID quadEntity2 = m_ecs.CreateEntity();
	m_ecs.CreateComponent<MeshComponent>(quadEntity2, m_quad2);
	m_ecs.CreateComponent<TransformComponent>(quadEntity2, Transform(Vector3f(1.5f, 0.0f, 0.0f)));

	m_monkey = m_modelImporter.LoadModel("Res/Models/Monkey.happmdl", &bricks);

	EntityID monkeyEntity = m_ecs.CreateEntity();
	m_ecs.CreateComponent<TransformComponent>(monkeyEntity, Transform(Vector3f(-0.5f, 0.5f, -2.0f)));
	m_ecs.CreateComponent<MeshComponent>(monkeyEntity, m_monkey);
	m_ecs.CreateComponent<RotationComponent>(monkeyEntity, Vector3f(-0.6f, 0.8f, 0.2f), 180.0f, 6.0f, 0.25f);
	

	EntityID camera = m_ecs.CreateEntity();
	m_ecs.CreateComponent<CameraComponent>(camera, 70.0f);
	m_ecs.CreateComponent<TransformComponent>(camera);
	m_ecs.CreateComponent<KeyboardMovementComponent>(camera, INPUT_KEY_W, INPUT_KEY_S, INPUT_KEY_A, INPUT_KEY_D, INPUT_KEY_LEFT_SHIFT, 2.0f, 2.0f);
	m_ecs.CreateComponent<MouseLookAroundComponent>(camera, INPUT_KEY_ESC, 10.0f, 5.0f);
}

void TestGame::FreeResources()
{
	delete m_quad1.pVertexBuffer;
	delete m_quad1.pIndexBuffer;
	delete m_quad2.pVertexBuffer;
	delete m_quad2.pIndexBuffer;
	delete m_monkey.pVertexBuffer;
	delete m_monkey.pIndexBuffer;
	delete m_pTextureSampler;
	delete m_pBricks;
	delete m_pBricks2;
}

int main()
{
	CoreEngine* pCoreEngine = new CoreEngine(new TestGame(), RENDER_API_VULKAN, "Happic Game Engine", 1280, 720);
	pCoreEngine->Start();
	delete pCoreEngine;
}