#pragma once

#include <Camera.h>
#include <Constants.h>
#include <Window/IWindow.h>
#include <Window.h>

namespace GameEngine::Core
{
	void OnMouseDown(WPARAM btnState, int x, int y, Window* window)
	{
		window->SetMousePos(x, y);

		SetCapture(GetPlatformWindowHandle(window->GetWindowHandle()));
	}

	void OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void OnMouseMove(WPARAM btnState, int x, int y, Camera* camera, Window* window)
	{
		Math::Vector2i mousePos = window->GetMousePos();
		if ((btnState & MK_LBUTTON) != 0)
		{
			float dx = 0.25 * static_cast<float>(x - mousePos.x) * Math::Constants::PI / 180.f;
			float dy = 0.25 * static_cast<float>(y - mousePos.y) * Math::Constants::PI / 180.f;

			dy = -dy; // To avoid inverse movement

			camera->Rotate(dx, dy);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			float dx = 0.05f * static_cast<float>(x - mousePos.x);
			float dy = 0.05f * static_cast<float>(y - mousePos.y);

			Math::Vector3f offset = camera->GetViewDir() * (dx - dy);

			Math::Vector3f position = camera->GetPosition();
			position = position + offset;

			camera->SetPosition(position);
		}

		window->SetMousePos(x, y);
	}

	void ProsessInput(WPARAM btnState, Camera* camera, Window* window) {
		Math::Vector3f direction = Math::Vector3f::Zero();
		float speed = 0.3f;

		std::string code = window->GetConfigReader().Get("movement", std::string{static_cast<char>(btnState)}, "");

		if (!code.empty()) [[unlikely]] {
			if (code == "move_left") {
				direction = direction - Math::Vector3f(0, 1, 0).CrossProduct(camera->GetViewDir());
			}
			if (code == "move_right") {
				direction = direction + Math::Vector3f(0, 1, 0).CrossProduct(camera->GetViewDir());
			}
			if (code == "move_forward") {
				direction = direction + camera->GetViewDir();
			}
			if (code == "move_backward") {
				direction = direction - camera->GetViewDir();
			}

			direction.Normalized();
		}

		camera->SetPosition(camera->GetPosition() + direction * speed);
	}
}