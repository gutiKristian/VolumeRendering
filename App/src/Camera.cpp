#include "Camera.h"
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

namespace med
{
	Camera::Camera(float left, float right, float bottom, float top, float near, float far) :
		m_Left(left), m_Right(right), m_Bottom(bottom), m_Top(top) , m_Near(near), m_Far(far)
	{
		m_Type = CameraType::Orthographic;
		RecalculateViewMatrix();
		RecalculateProjectionMatrix();
	}

	Camera::Camera(float fov, float aspect, float near, float far)
		: m_Fov(fov), m_Aspect(aspect), m_Near(near), m_Far(far)
	{
		m_Type = CameraType::Perspective;  
		RecalculateViewMatrix();
		RecalculateProjectionMatrix();
	}

	Camera Camera::CreatePerspective(float fov, float aspect, float near, float far)
	{
		return {fov, aspect, near, far};
	}

	Camera Camera::CreateOrthographic(float left, float right, float bottom, float top, float near, float far)
	{
		return { left, right, bottom, top, near, far };
	}

	void Camera::SetPosition(const glm::vec3 position)
	{
		m_Position = position;
		RecalculateViewMatrix();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_Aspect = aspectRatio;
		RecalculateProjectionMatrix();
	}

	void Camera::SetFov(float fov)
	{
		m_Fov = fov;
		RecalculateProjectionMatrix();
	}

	void Camera::SetZoomDistance(float delta)
	{
		if (m_Type == CameraType::Perspective)
		{
			m_Distance = glm::clamp(m_Distance + delta * m_Speed, 0.1f, 10.0f);
			RecalculateViewMatrix();
		}
		else
		{
			const float sign = (delta < 0.0f) ? -1.0f : 1.0f;
			const float aspect = m_Right / m_Top;
			m_Left -= (m_Speed * sign) * aspect;
			m_Right += (m_Speed * sign) * aspect;
			m_Bottom -= m_Speed * sign;
			m_Top += m_Speed * sign;
			RecalculateProjectionMatrix();
		}
			
	}

	const glm::mat4& Camera::GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	const glm::mat4& Camera::GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	const glm::mat4& Camera::GetInverseProjectionMatrix() const
	{
		return m_InverseProjectionMatrix;
	}

	const glm::mat4& Camera::GetInverseViewMatrix() const
	{
		return m_InverseViewMatrix;
	}

	glm::quat Camera::GetOrientation() const
	{
		return glm::quat(glm::vec3(m_Pitch, m_Yaw, 0));
	}

	glm::vec3 Camera::GetUp() const
	{ 
		return glm::normalize(glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	
	glm::vec3 Camera::GetForward() const
	{ 
		return glm::normalize(glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)));
	}
	
	glm::vec3 Camera::GetRight() const
	{
		return glm::normalize(glm::rotate(GetOrientation(), glm::vec3(1, 0, 0)));
	}

	glm::vec3 Camera::GetPosition() const
	{
		return m_Position - GetForward() * m_Distance;
	}

	void Camera::KeyboardEvent(int key)
	{
		switch (key)
		{
		case GLFW_KEY_UP:
			m_Position += (GetForward() * m_Speed);
			break;
		case GLFW_KEY_DOWN:
			m_Position += (-GetForward() * m_Speed);
			break;
		case GLFW_KEY_LEFT:
			glm::vec3 left = -GetRight();
			m_Position += (left * m_Speed);
			break;
		case GLFW_KEY_RIGHT:
			glm::vec3 right = GetRight();
			m_Position += (right * m_Speed);
			break;
		default:
			break;
		}
		RecalculateViewMatrix();
	}

	void Camera::Rotate(float delta_x, float delta_y)
	{
		m_Pitch += delta_y * m_RotateSens;
		m_Yaw += delta_x * m_RotateSens;

		RecalculateViewMatrix();
	}

	void Camera::RecalculateViewMatrix()
	{
		glm::quat orientation = GetOrientation();
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), -GetForward() * m_Distance) 
			* glm::translate(glm::mat4(1.0f), m_Position) 
			* glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(transform);
		m_InverseViewMatrix = transform;
	}

	void Camera::RecalculateProjectionMatrix()
	{
		if (m_Type == CameraType::Perspective) [[likely]]
		{
			m_ProjectionMatrix = glm::perspective(m_Fov, m_Aspect, m_Near, m_Far);
			m_InverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
		}
		else
		{
			m_ProjectionMatrix = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_Near, m_Far);
		}
		m_InverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);

	}
}