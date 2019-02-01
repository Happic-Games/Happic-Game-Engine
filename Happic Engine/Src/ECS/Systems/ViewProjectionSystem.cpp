#include "ViewProjectionSystem.h"
#include "../EntityComponentSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/CameraComponent.h"

namespace Happic { namespace ECS {

	ViewProjectionSystem::ViewProjectionSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(CameraComponent::ID);
	}

	void ViewProjectionSystem::ProcessEntity(EntityID entityID, Core::IDisplayInput * pInput, float dt)
	{
		CameraComponent* pCameraComponent = m_pECS->GetComponent<CameraComponent>(entityID);
		TransformComponent* pTransformComponent = m_pECS->GetComponent<TransformComponent>(entityID);

		Math::Matrix4f perspective;
		perspective.InitPerspective(pCameraComponent->fov, 1280, 720, pCameraComponent->znear, pCameraComponent->zfar);

		if(Rendering::IRenderContext::GetRenderAPI() == Rendering::RENDER_API_VULKAN)
			perspective = perspective * Math::Matrix4f().InitScale(Math::Vector3f(1.0f, -1.0f, 1.0f));

		Math::Quaternion rot = pTransformComponent->transform.GetRot().GetConjugate();
		
		Math::Matrix4f viewRotation = rot.CreateRotationMatrix();
		Math::Matrix4f viewTranslation;

		viewTranslation.InitTranslation(pTransformComponent->transform.GetPos() * -1.0f);
		Math::Matrix4f view = viewRotation * viewTranslation;

		Math::Matrix4f viewProjection = perspective * view;

		m_subEngines.pRenderingEngine->SetViewProjection(viewProjection);
	}

} }
