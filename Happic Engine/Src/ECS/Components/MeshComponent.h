#pragma once

#include "../ECSComponent.h"
#include "../../Rendering/Mesh.h"

namespace Happic { namespace ECS {

	struct MeshComponent : public Component<MeshComponent>
	{
		MeshComponent(const Rendering::Mesh& mesh) :
			mesh(mesh)
		{}

		Rendering::Mesh mesh;
	};

} }
