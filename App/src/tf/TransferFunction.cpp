#include "TransferFunction.h"
#include "Base/GraphicsContext.h"
#include "Base/Base.h"

#include <cassert>
#include <iterator>

namespace med
{
	void TransferFunction::ResolveResolution(int resolution)
	{
		m_TextureResolution = resolution;
		int maxTex1Dsize = GetMaxTextureResolution();

		if (maxTex1Dsize < m_TextureResolution)
		{
			LOG_WARN("Max data value is greater than max texture 1D size, clamping to max texture size");
			m_TextureResolution = maxTex1Dsize;
		}
		else if (m_TextureResolution <= 0)
		{
			LOG_INFO("Texture resolution will be set to maximum possible range.");
			std::string s = "Texture resolution set to: " + std::to_string(maxTex1Dsize);
			LOG_INFO(s.c_str());
			m_TextureResolution = maxTex1Dsize;
		}
	}

	std::shared_ptr<Texture> TransferFunction::GetTexture() const
	{
		assert(p_Texture != nullptr && "Texture is not initialized");
		return p_Texture;
	}

	int TransferFunction::GetMaxTextureResolution() const
	{
		return base::GraphicsContext::GetLimits().maxTextureDimension1D;
	}

	int TransferFunction::GetTextureResolution() const
	{
		return m_TextureResolution;
	}

	int TransferFunction::GetDataRange() const
	{
		return m_DataRange;
	}

	void TransferFunction::SetDataRange(int range)
	{
		m_DataRange = range;
	}

	int TransferFunction::AddControlPoint(double mouseX, double mouseY, bool updateOnAdd)
	{
		constexpr int CONTROL_POINT_EXISTS = -1;
		auto findPoint = [](const auto& obj, double x) { return obj.x < x; };

		// On x-axis we always work with nearest integer values -- from 0 to texture resolution
		double x = std::round(mouseX);
		double y = mouseY;

		// Retrieve the index of this point if already exist
		auto iter = std::lower_bound(m_ControlPoints.begin(), m_ControlPoints.end(), x, findPoint);

		if (iter == m_ControlPoints.end() || x == (*iter).x)
		{
			LOG_TRACE("Control point already exists, drag it to edit its value");
			return CONTROL_POINT_EXISTS;
		}

		// Create Implot handles for control points
		m_ControlPoints.emplace_back(x, y);

		// Helps us recalculate data between the neighbourhood points
		std::ranges::sort(m_ControlPoints, [](const auto& a, const auto& b) {return a.x < b.x; });

		iter = std::lower_bound(m_ControlPoints.begin(), m_ControlPoints.end(), x, findPoint);

		LOG_TRACE("Added new point");
		int cpId = std::distance(m_ControlPoints.begin(), iter);
		
		// Transfer function might have to update its inner state based before updating
		if (updateOnAdd)
		{
			// Trigger recalculation
			UpdateYAxis(cpId);
		}
		
		return cpId;
	}
}