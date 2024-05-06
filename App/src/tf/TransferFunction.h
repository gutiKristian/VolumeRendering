#pragma once

#include "../renderer/Texture.h"

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

namespace med
{
	class TransferFunction
	{
	public:
		virtual void Render() = 0;
		virtual void UpdateTexture() = 0;
		virtual std::string GetType() const = 0;
		virtual bool Save(const std::string& name) = 0;
		virtual void Load(const std::string& name) = 0;
	protected:
		virtual void UpdateYAxis(int cpId) = 0;
	// Implemented
	protected:
		void ResolveResolution(int resolution);
	public:
		std::shared_ptr<Texture> GetTexture() const;

		/*
		* @brief Get maximum possible resolution of 1D texture on current hardware.
		*/
		int GetMaxTextureResolution() const;

		/*
		* @brief Texture resolution, control points x-axis is bounded by this value.
		*/
		int GetTextureResolution() const;

		/*
		* @brief Get range of the data (file) on what is TF calibrated.
		*/
		int GetDataRange() const;

		/*
		* @brief Set range of the data (file) on what is TF calibrated.
		* @param range: value used for normalization of the 
		*/
		void SetDataRange(int range);
		
		/*
		* @brief Adds point to the TF
		* @param mouseX: Click position within the plot
		* @param mouseY: Click position within the plot
		* @param updateOnAdd: Whether trigger recalculation of Y axis (1D TF)
		*/
		int AddControlPoint(double mouseX, double mouseY, bool updateOnAdd = true);
	protected:
		std::shared_ptr<Texture> p_Texture = nullptr;
		std::vector<glm::dvec2> m_ControlPoints{};
		int m_TextureResolution = 0;
		int m_DataRange = 0;
		bool m_ShouldUpdate = false;
		char m_NameBuffer[30] = { 0 };
	};
}