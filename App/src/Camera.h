#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>


namespace med
{

	enum class CameraType
	{
		Undefined,
		Perspective,
		Orthographic
	};

	class Camera
	{
		Camera(float fov, float aspect, float near, float far);
		Camera(float left, float right, float bottom, float top, float near, float far);
	public:
		static Camera CreatePerspective(float fov, float aspect, float near, float far);
		static Camera CreateOrthographic(float left, float right, float bottom, float top, float near, float far);
	public:
		void KeyboardEvent(int key);
		
		void SetPosition(const glm::vec3 position);
		void SetAspectRatio(float aspectRatio);
		void SetFov(float fov);
		void SetZoomDistance(float delta);

		const glm::mat4& GetProjectionMatrix() const;
		const glm::mat4& GetViewMatrix() const;
		const glm::mat4& GetInverseProjectionMatrix() const;
		const glm::mat4& GetInverseViewMatrix() const;

		glm::quat GetOrientation() const;
		glm::vec3 GetUp() const;
		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetPosition() const;
		void Rotate(float, float);

	private:
		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();

	private:
		CameraType m_Type = CameraType::Undefined;
		float m_Far = 0.0f;
		float m_Near = 0.0f;				
		// Perspective
		float m_Fov = 0.0f;
		float m_Aspect = 0.0f;
		// Orthographic
		float m_Left = 0.0f;
		float m_Right = 0.0f;
		float m_Bottom = 0.0f;
		float m_Top = 0.0f;
		// Controls
		float m_Speed = 0.1f;
		float m_Pitch;
		float m_Yaw;
		float m_RotateSens = 0.005f;
		float m_Distance = 5.0f;

		glm::mat4 m_ProjectionMatrix{ 1.0f };

		// Position in world coordinate system
		glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };

		glm::mat4 m_ViewMatrix{ 1.0f };

		glm::mat4 m_InverseViewMatrix{};
		glm::mat4 m_InverseProjectionMatrix{};

	};
}