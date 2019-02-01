#include "MouseLookAroundSystem.h"
#include "../EntityComponentSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/MouseLookAroundComponent.h"

namespace Happic { namespace ECS {

	MouseLookAroundSystem::MouseLookAroundSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(MouseLookAroundComponent::ID);
	}

	void MouseLookAroundSystem::ProcessEntity(EntityID entityID, Core::IDisplayInput * pInput, float dt)
	{
		TransformComponent* pTransformComponent = m_pECS->GetComponent<TransformComponent>(entityID);
		MouseLookAroundComponent* pMouseLookAroundComponent = m_pECS->GetComponent<MouseLookAroundComponent>(entityID);

		const Math::Vector2f CENTER_POS(1280.0f / 2.0f, 720.0f / 2.0f);

		if (pInput->IsKeyPressed(pMouseLookAroundComponent->toggleKey))
		{
			pInput->SetCursorVisible(!pInput->IsCursorVisible());
			pInput->SetCursorPosition(CENTER_POS);
		}

		if (pInput->IsCursorVisible())
			return;

		Math::Vector2f deltaPos = (pInput->GetCursorPosition() - CENTER_POS) * dt;
		bool rotX = deltaPos.GetY() != 0.0f;
		bool rotY = deltaPos.GetX() != 0.0f;

		if (rotX)
			pTransformComponent->transform.Rotate(pTransformComponent->transform.GetRot().GetRight(), deltaPos.GetY() * pMouseLookAroundComponent->sensitivityX);
		if (rotY)
			pTransformComponent->transform.Rotate(Math::Vector3f(0.0f, 1.0f, 0.0f), deltaPos.GetX() * pMouseLookAroundComponent->sensitivityY);
		if (rotX || rotY)
			pInput->SetCursorPosition(CENTER_POS);
	}


} }