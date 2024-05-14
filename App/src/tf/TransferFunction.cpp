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

	glm::dvec2 TransferFunction::RemapCP(glm::dvec2 cp, int dataRange, int tfResolution)
	{
		tfResolution -= 1; // indexed from 0
		int currentDataRange = GetDataRange();

		if (currentDataRange == 0)
		{
			LOG_ERROR("Control point cannot be remapped to a new file data range.");
			return cp;
		}

		// Basically to [0, 1]
		double textureCoordinate = cp.x / tfResolution;
		
		// Density value that corresponded to the control point in dataset where it was calibrated
		double oldDensityValue = textureCoordinate * dataRange;

		if (oldDensityValue > currentDataRange) // && oldDensityValue > tfResolution
		{
			// even though tf resolution still might be able to cover this, there's no point to have this cp for this data range
			LOG_INFO("The TF was defined on dataset with higher range, points outside of this data range will be clipped");
			return { -1.0 , -1.0 };
		}

		// Now find where on x is oldDensityValue when data range is currentDataRange
		// (to [0,1]) * to tf range
		int newX = static_cast<int>((oldDensityValue / currentDataRange) * tfResolution); // * m_TextureResolution

		return { newX, cp.y };
	}

	std::vector<glm::dvec2> TransferFunction::RemapCPVector(std::vector<glm::dvec2> cps, int dataRange, int tfResolution)
	{
		// If control points are merged into a one CP beacuse of data range and poor TF resolution
		// then y coordinate of the cp is taken, there are many ways to handle this situation (average, median, maximal, minimal value)
		std::vector<glm::dvec2> result{};

		auto doesCpExist = [&result](double x) {
			return std::find_if(result.begin(), result.end(), [x](const glm::dvec2& point) 
				{ return point.x == x;}) != result.end();
			};

		for (auto cp : cps)
		{
			auto newCp = RemapCP(cp, dataRange, tfResolution);

			// should be clipped
			if (newCp.x == -1 && newCp.y == -1)
			{
				continue;
			}

			if (doesCpExist(newCp.x))
			{
				continue;
			}

			result.push_back(newCp);
		}

		if (!doesCpExist(tfResolution - 1))
		{
			result.emplace_back(tfResolution - 1, 1.0);
		}

		return result;
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