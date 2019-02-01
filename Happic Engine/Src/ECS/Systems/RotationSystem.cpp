#include "RotationSystem.h"
#include"../EntityComponentSystem.h"
#include "../Components/RotationComponent.h"
#include "../Components/TransformComponent.h"

namespace Happic { namespace ECS {

	RotationSystem::RotationSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(RotationComponent::ID);
		m = 0.0f;
	}

	void RotationSystem::ProcessEntity(EntityID entityID, Core::IDisplayInput * pInput, float dt)
	{
		TransformComponent* pTransformComponent = m_pECS->GetComponent<TransformComponent>(entityID);
		RotationComponent* pRotationComponent = m_pECS->GetComponent<RotationComponent>(entityID);

		pTransformComponent->transform.Rotate(pRotationComponent->axis, pRotationComponent->rotationPerSecond * dt);
		pTransformComponent->transform.GetPos().Set(cosf(m) * pRotationComponent->circleRadius, 0.0f, sinf(m) * pRotationComponent->circleRadius);

		m += (pRotationComponent->revolutionPerSecond * dt * MATH_2PI);
		if (m > MATH_2PI)
			m = 0.0f;
	}

} }
