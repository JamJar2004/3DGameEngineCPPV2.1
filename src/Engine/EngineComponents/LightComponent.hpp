#pragma once

#include "../Rendering/Material.hpp"

#include <memory>

struct ShadowInfo
{
	ShadowInfo(const glm::mat4& projection) : Projection(projection) {}

	glm::mat4 Projection;
};

using ShadowInfoHandle = std::shared_ptr<ShadowInfo>;

struct Light;

using LightHandle = std::shared_ptr<Light>;

struct LightComponent : public ECS::Component<LightComponent>
{
	LightComponent(LightHandle light) : Light(light) {}

	LightHandle Light;
};

struct Light
{
	Light(const glm::vec3& color, float intensity, ShadowInfoHandle shadowInfo = nullptr) : Color(color), Intensity(intensity), ShadowInfo(shadowInfo) {}

	ShadowInfoHandle ShadowInfo;

	glm::vec3 Color;
	float     Intensity;

	virtual TypeInfo* GetType() const = 0;

	virtual void UpdateShader(ShaderHandle shader) const
	{
		shader->Get<glm::vec3>("u_color") = Color;
		shader->Get<float>("u_intensity") = Intensity;
	}
};

template<std::derived_from<Light> T, typename... Args>
LightComponent CreateLight(Args&&... args) requires std::constructible_from<T, Args...>
{
	LightHandle light = std::make_shared<T>(args...);
	return LightComponent(light);
}

struct AmbientLight : public Light
{
	AmbientLight(const glm::vec3& color, float intensity) : Light(color, intensity, nullptr) {}

	TypeInfo* GetType() const override { return TypeInfo::Get<AmbientLight>(); }
};

struct DirectionalLight : public Light
{
	DirectionalLight(const glm::vec3& color, float intensity, float shadowSoftness = 3.0f) :
		Light(color, intensity, std::make_shared<struct ShadowInfo>(glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, -40.0f, 40.0f))),
		ShadowSoftness(shadowSoftness) {}

	float ShadowSoftness;

	virtual TypeInfo* GetType() const override { return TypeInfo::Get<DirectionalLight>(); }

	virtual void UpdateShader(ShaderHandle shader) const override
	{
		Light::UpdateShader(shader);
		shader->TrySet("u_shadowSoftness", ShadowSoftness);
		//shader->Get<float>("u_shadowSoftness") = ShadowSoftness;
	}
};

struct Attenuation
{
	Attenuation(float constant, float linear, float exponent) :
		Constant(constant), Linear(linear), Exponent(exponent) {}

	Attenuation(const glm::vec3& attenuation) :
		Constant(attenuation.x), Linear(attenuation.y), Exponent(attenuation.z) {}

	float Constant;
	float Linear;
	float Exponent;
};

static float CalcRange(const glm::vec3& color, float intensity, const Attenuation& attenuation)
{
	float a = attenuation.Exponent;
	float b = attenuation.Linear;
	float c = attenuation.Constant - 256 * intensity * glm::compMax(color);

	return (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);
}

struct PointLight : public Light
{
	PointLight(const glm::vec3& color, float intensity, const Attenuation& attenuation, ShadowInfoHandle shadowInfo = nullptr) :
		Light(color, intensity, shadowInfo), LightAttenuation(attenuation), Range(CalcRange(color, intensity, attenuation))
	{

	}

	Attenuation LightAttenuation;

	float Range;

	TypeInfo* GetType() const override { return TypeInfo::Get<PointLight>(); }

	void UpdateShader(ShaderHandle shader) const override
	{
		Light::UpdateShader(shader);
		shader->Get<float>("u_constant") = LightAttenuation.Constant;
		shader->Get<float>("u_linear") = LightAttenuation.Linear;
		shader->Get<float>("u_exponent") = LightAttenuation.Exponent;
		shader->Get<float>("u_range") = Range;
	}
};

struct SpotLight : public PointLight
{
	SpotLight(const glm::vec3& color, float intensity, const struct Attenuation& attenuation, float angle, float shadowSoftness = 3.0f) :
		PointLight(color, intensity, attenuation, nullptr), Cutoff(cosf(glm::radians(angle) * 0.5f)), ShadowSoftness(shadowSoftness)
	{
		ShadowInfo = std::make_shared<struct ShadowInfo>(glm::perspective(glm::radians(angle), 1.0f, 0.1f, Range));
	}

	float Cutoff;
	float ShadowSoftness;

	TypeInfo* GetType() const override { return TypeInfo::Get<SpotLight>(); }

	void UpdateShader(ShaderHandle shader) const override
	{
		PointLight::UpdateShader(shader);
		shader->Get<float>("u_cutoff") = Cutoff;
		shader->TrySet("u_shadowSoftness", ShadowSoftness);
	}
};

