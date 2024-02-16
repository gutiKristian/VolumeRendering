#include "VolumeFile.h"
namespace med
{
    void VolumeFile::SetData(std::vector<float> &src)
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

    const std::vector<float>& VolumeFile::GetVecReference() const
    {
        return m_Data;
    }

}
