#pragma once

#include <Reflection.hpp>

#include <unordered_map>
#include "Texture.hpp"

class Field
{
private:
	void* m_defaultValue;
public:
	Field(const std::string& name, size_t offset, Type* type) : m_defaultValue(nullptr), Name(name), Offset(offset), Type(type) {}

	Field(const Field& other) : m_defaultValue(nullptr), Name(other.Name), Offset(other.Offset), Type(other.Type)
	{
		SetDefaultValue((const void*)other.m_defaultValue);
	}

	~Field()
	{
		Type->Destructor(m_defaultValue);
		free(m_defaultValue);
	}

	Field& operator=(const Field&) = delete;

	const std::string Name;
	const size_t      Offset;
	Type* const       Type;

	void SetDefaultValue(const void* value)
	{
		if(m_defaultValue)
		{
			Type->Destructor(m_defaultValue);
			free(m_defaultValue);
		}

		m_defaultValue = malloc(Type->Size);
		Type->CopyConstructor(value, m_defaultValue);
	}

	template<std::copy_constructible T>
	void SetDefaultValue(const T& value)
	{
		if(m_defaultValue)
		{
			Type->Destructor(m_defaultValue);
			free(m_defaultValue);
		}

		DEBUG_ASSERT(Type == Type::Get<T>(), "Type mismatch."); 
		m_defaultValue = malloc(sizeof(T)); 
		new(m_defaultValue) T(value);
	}

	const void* GetDefaultValue() const { return m_defaultValue; }
};

class Shader
{
private:
	std::unordered_map<std::string, size_t> m_fieldMap;
	std::unordered_map<std::string, uint32_t> m_textureMap;

	size_t   m_materialTextureCount;
	uint32_t m_nextTextureSlot;

	std::vector<Field> m_materialFields;
	std::vector<Field> m_uniformFields;

	size_t m_materialSize;
	size_t m_uniformSize;

	std::vector<uint8_t> m_uniformData;

	std::unordered_map<uint32_t, TextureHandle> m_textures;
protected:
	Field& AddMaterialField(const std::string& name, Type* type)
	{
		m_fieldMap[name] = m_materialFields.size();
		Field& field = m_materialFields.emplace_back(name, m_materialSize, type);
		field.SetDefaultValue(type->DefaultValue);
		m_materialSize += type->Size;
		return field;
	}
	
	Field& AddUniformField(const std::string& name, Type* type)
	{
		m_fieldMap[name] = m_uniformFields.size();
		Field& field = m_uniformFields.emplace_back(name, m_uniformSize, type);
		field.SetDefaultValue(type->DefaultValue);
		m_uniformSize += type->Size;
		m_uniformData.resize(m_uniformSize);

		field.Type->CopyConstructor(field.GetDefaultValue(), m_uniformData.data() + field.Offset);
		return field;
	}

	void AddMaterialTexture(const std::string& name)
	{
		size_t fieldIndex = m_fieldMap[name];
		Field& field = m_materialFields[fieldIndex];
		field.SetDefaultValue<int>(m_nextTextureSlot);
		m_textureMap[name] = m_nextTextureSlot++;
		++m_materialTextureCount;
	}

	void AddUniformTexture(const std::string& name)
	{
		size_t fieldIndex = m_fieldMap[name];
		Field& field = m_uniformFields[fieldIndex];
		field.SetDefaultValue<int>(m_nextTextureSlot);
		uint32_t slot = m_nextTextureSlot++;
		m_textureMap[name] = slot;
		m_textures[slot] = nullptr;

		field.Type->CopyConstructor(field.GetDefaultValue(), m_uniformData.data() + field.Offset);
	}
public:
	Shader() : m_materialTextureCount(0), m_nextTextureSlot(0), m_materialSize(0), m_uniformSize(0) {}

	      void* UniformData()       { return m_uniformData.data(); }
	const void* UniformData() const { return m_uniformData.data(); }

	void* Get(const Field& field)
	{
		return m_uniformData.data() + field.Offset;
	}

	const void* Get(const Field& field) const
	{
		return m_uniformData.data() + field.Offset;
	}

	template<typename T>
	T& Get(const Field& field)
	{
		DEBUG_ASSERT(field.Type == Type::Get<T>(), "Type mismatch.");
		return *(T*)Get(field);
	}

	template<typename T>
	const T& Get(const Field& field) const
	{
		DEBUG_ASSERT(field.Type == Type::Get<T>(), "Type mismatch.");
		return *(const T*)Get(field);
	}

	template<typename T>
	T& Get(const std::string& name)
	{
		const Field& field = GetUniformField(name);
		return Get<T>(field);
	}

	template<typename T>
	const T& Get(const std::string& name) const
	{
		const Field& field = GetUniformField(name);
		return Get<T>(field);
	}

	template<typename T>
	bool TrySet(const std::string& name, const T& value)
	{
		auto it = m_fieldMap.find(name);
		if(it == m_fieldMap.end())
			return false;

		size_t index = it->second;
		const Field& field = m_uniformFields[index];
		Get<T>(field) = value;
		return true;
	}

	void SetTexture(const std::string& name, TextureHandle texture);
	bool TrySetTexture(const std::string& name, TextureHandle texture);

	size_t GetTotalTextureCount()    const { return m_textureMap.size();    }
	size_t GetMaterialTextureCount() const { return m_materialTextureCount; }
	size_t GetUniformTextureCount()  const { return m_textures.size();      }

	size_t GetMaterialSize() const { return m_materialSize; }
	size_t GetUniformSize()  const { return m_uniformSize; }

	const std::vector<Field>& GetMaterialFields() const { return m_materialFields; }
	const std::vector<Field>& GetUniformFields()  const { return m_uniformFields;  }

	Field& GetMaterialField(const std::string& name) { return m_materialFields[m_fieldMap[name]]; }
	Field& GetUniformField (const std::string& name) { return m_uniformFields[m_fieldMap[name]];  }

	uint32_t GetTextureSlot(const std::string& name) { return m_textureMap[name]; }

	bool Contains(const std::string& name) const { return m_fieldMap.find(name) != m_fieldMap.end(); }

	void Use(const void* materialData);

	virtual void OnUse(const void* materialData) = 0;

	friend class Material;
};

using ShaderHandle = std::shared_ptr<Shader>;

//class UniformBuffer
//{
//private:
//	void* m_data;
//
//	ShaderHandle m_shader;
//
//	std::vector<TextureHandle> m_textures;
//public:
//	UniformBuffer(ShaderHandle shader) : m_data(malloc(shader->GetUniformSize())), m_shader(shader) {}
//
//	void* Data() { return m_data; }
//
//	void* Get(const Field& field)
//	{
//		return (uint8_t*)m_data + field.Offset;
//	}
//
//	const void* Get(const Field& field) const
//	{
//		return (const uint8_t*)m_data + field.Offset;
//	}
//
//	template<typename T>
//	T& Get(const Field& field)
//	{
//		DEBUG_ASSERT(field.Type == Type::Get<T>(), "Type mismatch.");
//		return *(T*)Get(field); 
//	}
//
//	template<typename T>
//	const T& Get(const Field& field) const
//	{ 
//		DEBUG_ASSERT(field.Type == Type::Get<T>(), "Type mismatch.");
//		return *(const T*)Get(field);
//	}
//
//	template<typename T>
//	T& Get(const std::string& name)
//	{
//		const Field& field = m_shader->GetUniformField(name);
//		return Get<T>(field);
//	}
//
//	template<typename T>
//	const T& Get(const std::string& name) const
//	{
//		const Field& field = m_shader->GetUniformField(name);
//		return Get<T>(field);
//	}
//
//	void SetTexture(const std::string& name, TextureHandle texture)
//	{
//		size_t index = m_shader->GetTextureIndex(name);
//		m_textures[index] = texture;
//	}
//};
//
//using UniformBufferHandle = std::shared_ptr<UniformBuffer>;