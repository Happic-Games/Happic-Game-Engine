#pragma once

#include "Math.h"

namespace Happic { namespace Math {

	class Transform
	{
	public:
		Transform(const Vector3f& pos = Vector3f(0.0f, 0.0f, 0.0f), const Vector3f& scale = Vector3f(1.0f, 1.0f, 1.0f),
			const Quaternion& rot = Quaternion(0.0f, 0.0f, 0.0f, 1.0f)) :
			m_pos(pos),
			m_scale(scale),
			m_rot(rot)
		{}

		inline Transform& Translate(const Vector3f& translation) { m_pos += translation; return *this; }
		inline Transform& Translate(const Vector3f& direction, float speed) { m_pos += (direction * speed); return *this; }
		inline Transform& Scale(const Vector3f& scale) { m_scale *= scale; return *this; }
		inline Transform& Scale(float scale) { m_scale *= scale; return *this; }
		inline Transform& Rotate(const Quaternion& rotation) { m_rot = (rotation * m_rot).Normalized(); return *this; }
		inline Transform& Rotate(const Vector3f& axis, float angle) { return Rotate(Quaternion(axis, angle)); }

		inline Matrix4f CreateModelMatrix() const { return Matrix4f().InitTranslation(m_pos) * (m_rot.CreateRotationMatrix() * Matrix4f().InitScale(m_scale)); }

		inline Vector3f& GetPos() { return m_pos; }
		inline Vector3f& GetScale() { return m_scale; }
		inline Quaternion& GetRot() { return m_rot; }

		inline const Vector3f& GetPos() const { return m_pos; }
		inline const Vector3f& GetScale() const { return m_scale; }
		inline const Quaternion& GetRot() const { return m_rot; }
	private:
		Vector3f m_pos;
		Vector3f m_scale;
		Quaternion m_rot;
	};

}}
