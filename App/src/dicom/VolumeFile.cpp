#include "VolumeFile.h"
namespace med
{
    void VolumeFile::SetData(std::vector<glm::vec4> &src)
    {
        m_Data = std::move(src);
    }

    void VolumeFile::SetFileDataType(FileDataType type)
    {
        m_FileDataType = type;
    }

    void VolumeFile::SetDataSize(std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size)
    {
        m_Size = size;
    }

    void VolumeFile::PreComputeGradient()
    {
        auto[xS,yS,zS] = m_Size;
        glm::vec3 xAxis(1, 0, 0), yAxis(0, 1, 0), zAxis(0, 0, 1);

        for (int z = 0; z < zS; ++z)
        {
            for (int y = 0; y < yS; ++y)
            {
                for (int x = 0; x < xS; ++x)
                {
                    int c = GetIndexFrom3D(x, y, z);
                    int xM = GetIndexFrom3D(x - 1, y, z);
                    int xP = GetIndexFrom3D(x + 1, y, z);
                    int yM = GetIndexFrom3D(x, y - 1, z);
                    int yP = GetIndexFrom3D(x, y + 1, z);
                    int zM = GetIndexFrom3D(x, y, z - 1);
                    int zP = GetIndexFrom3D(x, y, z + 1);
                    glm::vec3 temp(0.0);
                    temp.x = m_Data[xP].a - m_Data[xM].a;
                    temp.y = m_Data[yP].a - m_Data[yM].a;
                    temp.z = m_Data[zP].a - m_Data[zM].a;
                    temp = glm::normalize(temp);
                    m_Data[c].x = temp.x;
                    m_Data[c].y = temp.y;
                    m_Data[c].z = temp.z;
                }
            }
        }
    }

    std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> VolumeFile::GetSize() const
    {
        return m_Size;
    }

    FileDataType VolumeFile::GetFileType() const
    {
        return m_FileDataType;
    }

    const void * VolumeFile::GetVoidPtr() const
    {
        return m_Data.data();
    }

    const std::vector<glm::vec4>& VolumeFile::GetVecReference() const
    {
        return m_Data;
    }

    int VolumeFile::GetIndexFrom3D(int x, int y, int z) const
    {
        auto checkBounds = [&](int val, int upperBound) -> int
        {
            if (val < 0 || val >= upperBound)
                return 0;
            return val;
        };

        const auto&[xS, yS, zS] = m_Size;

        x = checkBounds(x, xS);
        y = checkBounds(y, yS);
        z = checkBounds(z, zS);

        return x * yS *zS + y * zS + z;
    }
}
