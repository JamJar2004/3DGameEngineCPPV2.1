#pragma once

#include "TextureAtlas.hpp"
#include "RenderDevice.hpp"

template<typename TMaterial>
class Material
{
public:
    explicit Material(RenderDevice& renderDevice, TMaterial materialData) :
        MaterialData(materialData), m_renderDevice(renderDevice) {}

    TMaterial MaterialData;

    friend class MaterialTextureInfo;
private:
    RenderDevice& m_renderDevice;

    std::vector<TextureHandle> m_textures;
};

class MaterialTextureInfo
{
public:
    MaterialTextureInfo() : m_bindingIndex(0), m_region(Rectangle()) {}

    template<typename TMaterial>
    void Set(Material<TMaterial>& material, const TextureHandle& texture)
    {
        m_bindingIndex = material.m_renderDevice.BindTexture(texture.Atlas);
        
        material.m_textures.push_back(texture);

        m_region = texture.Region;
    }
private:
    uint32_t  m_bindingIndex;
    Rectangle m_region;
};

// class MaterialList
// {
// public:
// 	MaterialList(ShaderHandle type, size_t maxCount = 100) :
// 		m_data(new uint8_t[maxCount * type->GetMaterialSize()]), m_end(0), m_maxCount(maxCount), Type(type) {}
//
// 	const ShaderHandle Type;
//
// 	size_t Create()
// 	{
// 		uint8_t* materialData = nullptr;
//
// 		auto it = m_freeIndices.begin();
// 		if(it != m_freeIndices.end())
// 			return *it * Type->GetMaterialSize();
//
// 		DEBUG_ASSERT(m_end < m_maxCount, "Maximum Material count got exceeded.");
// 		return m_end++ * Type->GetMaterialSize();
// 	}
//
// 	void Delete(size_t index)
// 	{
// 		if(index == m_end - 1U)
// 		{
// 			--m_end;
// 			return;
// 		}
// 		m_freeIndices.insert(index);
// 	}
//
// 	uint8_t* Get(size_t index) const
// 	{
// 		return m_data + (index * Type->GetMaterialSize());
// 	}
// private:
// 	uint8_t* m_data;
// 	size_t   m_end;
//
// 	const size_t m_maxCount;
//
// 	std::unordered_set<size_t> m_freeIndices;
// };
//
// class Material
// {
// private:
// 	std::shared_ptr<MaterialList> m_list;
// 	const size_t m_index;
//
// 	std::vector<TextureHandle> m_textures;
// public:
// 	explicit Material(const std::shared_ptr<MaterialList>& list) : m_list(list), m_index(list->Create())
// 	{
// 		m_textures.resize(GetType()->GetMaterialTextureCount());
//
// 		uint8_t* data = list->Get(m_index);
// 		for(const Field& field : list->Type->GetMaterialFields())
// 		{
// 			field.Type->CopyConstructor(field.GetDefaultValue(), data + field.Offset);
// 		}
// 	}
//
// 	Material(const Material& other) : m_list(other.m_list), m_index(other.m_list->Create())
// 	{
// 		m_textures.resize(GetType()->GetMaterialTextureCount());
//
// 		uint8_t* data = m_list->Get(m_index);
// 		for(const Field& field : m_list->Type->GetMaterialFields())
// 		{
// 			field.Type->CopyConstructor(other.Get(field), data + field.Offset);
// 		}
// 	}
//
// 	~Material()
// 	{
// 		uint8_t* data = m_list->Get(m_index);
// 		for(const Field& field : m_list->Type->GetMaterialFields())
// 			field.Type->Destructor(data + field.Offset);
//
// 		m_list->Delete(m_index);
// 	}
//
// 	Material& operator=(const Material&) = delete;
//
// 	ShaderHandle GetType() const { return m_list->Type; }
//
// 	template<typename T>
// 	T& As() { return *(T*)m_list->Get(m_index); }
//
// 	template<typename T>
// 	const T& As() const { return *(const T*)m_list->Get(m_index); }
//
// 	void* Get(const Field& field)
// 	{
// 		uint8_t* data = m_list->Get(m_index);
// 		return data + field.Offset;
// 	}
//
// 	const void* Get(const Field& field) const
// 	{
// 		const uint8_t* data = m_list->Get(m_index);
// 		return data + field.Offset;
// 	}
//
// 	template<typename T>
// 	T& Get(const Field& field)
// 	{
// 		DEBUG_ASSERT(field.Type == TypeInfo::Get<T>(), "Type mismatch.");
// 		return *(T*)Get(field);
// 	}
//
// 	template<typename T>
// 	const T& Get(const Field& field) const
// 	{
// 		DEBUG_ASSERT(field.Type == TypeInfo::Get<T>(), "Type mismatch.");
// 		return *(const T*)Get(field);
// 	}
//
// 	template<typename T>
// 	T& Get(const std::string& name)
// 	{
// 		const Field& field = GetType()->GetMaterialField(name);
// 		return Get<T>(field);
// 	}
//
// 	template<typename T>
// 	const T& Get(const std::string& name) const
// 	{
// 		const Field& field = GetType()->GetMaterialField(name);
// 		return Get<T>(field);
// 	}
//
// 	void SetTexture(const std::string& name, TextureHandle texture)
// 	{
// 		size_t index = GetType()->GetTextureSlot(name);
// 		m_textures[index] = texture;
// 	}
//
// 	void Use();
//
// 	friend class RenderDevice;
// };
//
// using MaterialHandle = std::shared_ptr<Material>;