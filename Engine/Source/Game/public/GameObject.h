#pragma once

#include <RenderObject.h>
#include <RenderThread.h>
#include <Vector.h>

namespace GameEngine
{
	class GameObject final
	{
	public:
		enum class ControllerType : uint32_t {
			Keyboard = 0,
			Physics = 1,
			Slide = 2,
			Invalid = ~uint32_t(0)
		};

	public:
		GameObject() = default;
		explicit GameObject(ControllerType controller_type) : m_ControllerType(controller_type) {
			switch (m_ControllerType)
			{
			case GameEngine::GameObject::ControllerType::Invalid:
				return;
			case GameEngine::GameObject::ControllerType::Keyboard:
				m_Speed = Math::Vector3f(5, 0, 0);
				break;
			case GameEngine::GameObject::ControllerType::Physics:
				m_Speed = Math::Vector3f(0, 15, 0);
				break;
			case GameEngine::GameObject::ControllerType::Slide:
				m_Speed = Math::Vector3f(0, 0, 4);
				break;
			}

			m_CurrentSpeed = m_Speed;
		}

	public:
		Render::RenderObject** GetRenderObjectRef() { return &m_RenderObject; }

		void SetPosition(Math::Vector3f position, size_t frame)
		{
			m_Position = position;

			if (m_RenderObject) [[likely]]
			{
				m_RenderObject->SetPosition(position, frame);
			}
		}

		Math::Vector3f GetPosition()
		{
			return m_Position;
		}

		ControllerType getControllerType() const { return m_ControllerType; }

		void Move(float dt, Math::Vector3f keyDirection, size_t frame) {
			switch (m_ControllerType) {
			case GameObject::ControllerType::Invalid:
				return;
			case GameObject::ControllerType::Keyboard:
				m_CurrentSpeed = Math::Vector3f(m_Speed.x * keyDirection.x, m_Speed.y * keyDirection.y, m_Speed.z * keyDirection.z);

				break;
			case GameObject::ControllerType::Physics:
				if (m_Position.y < -1.0f) {
					m_CurrentSpeed.y = 15.0f;
				}
				m_CurrentSpeed.y -= gravity * dt;

				break;
			case GameObject::ControllerType::Slide:
				if (!m_SlideSwitched) {
					m_CurrentSpeed.z = 4.0f;
					if (m_Position.z > 20.0f) {
						m_SlideSwitched = true;
					}
				}
				else {
					m_CurrentSpeed.z = -4.0f;
					if (m_Position.z < -20.0f) {
						m_SlideSwitched = false;
					}
				}

				break;
			}

			m_Position = m_Position + m_CurrentSpeed * dt;

			SetPosition(m_Position, frame);
		}

	protected:
		Render::RenderObject* m_RenderObject = nullptr;

		Math::Vector3f m_Position = Math::Vector3f::Zero();

		Math::Vector3f m_Direction = Math::Vector3f::Zero();
		Math::Vector3f m_Speed = Math::Vector3f::Zero();
		Math::Vector3f m_CurrentSpeed = Math::Vector3f::Zero();

		bool m_SlideSwitched = false;
		float gravity = 10.0f;

		ControllerType m_ControllerType = ControllerType::Invalid;
	};
}