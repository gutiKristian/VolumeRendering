#pragma once

#include "../renderer/Texture.h"

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

namespace med
{
	enum class TFLoadOption
	{
		NONE = 0,
		RESCALE_TO_NEW_RANGE = 1
	};

	class TransferFunction
	{
	public:
		virtual void Render() = 0;
		virtual void UpdateTexture() = 0;
		virtual std::string GetType() const = 0;
		virtual bool Save(const std::string& name) = 0;
		virtual void Load(const std::string& name, TFLoadOption option = TFLoadOption::NONE) = 0;
		virtual void ResetTF() = 0;
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
		* @brief Remaps CP that was defined on one dataset to this TF data range. 
		* Member function assumes you have set the data range for this TF.
		* @param cp: control point from loaded TF
		* @param dataRange:  data range on what this cp was defined on
		* @param tfResolution: resolution of loaded transfer function
		* @return new cp mapped to this datarange, if x = y = -1 point should be clipped
		*/
		glm::dvec2 RemapCP(glm::dvec2 cp, int dataRange, int tfResolution);
		

		/*
		* @brief Method remaps vector of control points, and clipps cps that cannot be 
		* remapped.
		*/
		std::vector<glm::dvec2> RemapCPVector(std::vector<glm::dvec2> cps, int dataRange, int tfResolution);

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