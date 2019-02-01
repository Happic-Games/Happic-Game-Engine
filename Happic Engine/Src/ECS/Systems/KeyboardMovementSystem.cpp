#include "KeyboardMovementSystem.h"

#include "../EntityComponentSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/KeyboardMovementComponent.h"

namespace Happic { namespace ECS {

	Happic::ECS::KeyboardMovementSystem::KeyboardMovementSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(KeyboardMovementComponent::ID);
	}

	void KeyboardMovementSystem::ProcessEntity(EntityID entityID, Core::IDisplayInput * pInput, float dt)
	{
		TransformComponent* pTransformComponent = m_pECS->GetComponent<TransformComponent>(entityID);
		KeyboardMovementComponent* pKeyboardMovementComponent = m_pECS->GetComponent<KeyboardMovementComponent>(entityID);

		float speed = pKeyboardMovementComponent->speed * dt;
		if (pInput->IsKeyDown(pKeyboardMovementComponent->sprintKey))
			speed *= pKeyboardMovementComponent->sprintMultiplier;

		if (pInput->IsKeyDown(pKeyboardMovementComponent->moveForward))
			pTransformComponent->transform.Translate(pTransformComponent->transform.GetRot().GetForward(), speed);
		if(pInput->IsKeyDown(pKeyboardMovementComponent->moveBack))
			pTransformComponent->transform.Translate(pTransformComponent->transform.GetRot().GetBack(), speed);
		if(pInput->IsKeyDown(pKeyboardMovementComponent->moveLeft))
			pTransformComponent->transform.Translate(pTransformComponent->transform.GetRot().GetLeft(), speed);
		if(pInput->IsKeyDown(pKeyboardMovementComponent->moveRight))
			pTransformComponent->transform.Translate(pTransformComponent->transform.GetRot().GetRight(), speed);
	}

} }
