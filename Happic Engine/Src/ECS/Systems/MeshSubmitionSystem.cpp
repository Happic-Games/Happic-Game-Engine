#include "MeshSubmitionSystem.h"
#include "../Components/MeshComponent.h"
#include "../Components/TransformComponent.h"
#include "../EntityComponentSystem.h"

namespace Happic { namespace ECS {

	MeshSubmitionSystem::MeshSubmitionSystem()
	{
		AddComponentType(MeshComponent::ID);
		AddComponentType(TransformComponent::ID);
	}

	void MeshSubmitionSystem::ProcessEntity(EntityID entity, Core::IDisplayInput * pInput, float dt)
	{
		MeshComponent* pMeshComponent = m_pECS->GetComponent<MeshComponent>(entity);
		TransformComponent* pTransformComponent = m_pECS->GetComponent<TransformComponent>(entity);
		Math::Matrix4f modelMatrix = pTransformComponent->transform.CreateModelMatrix();
		m_subEngines.pRenderingEngine->Submit(pMeshComponent->mesh, modelMatrix);
	}

} }
