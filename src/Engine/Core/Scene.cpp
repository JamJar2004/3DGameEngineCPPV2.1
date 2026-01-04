#include "Scene.hpp"

#include "Application.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <string>
#include <fstream>
#include <stb_image.h>

#include "../Rendering/RenderContext2D.hpp"
#include "../Json/Json.hpp"
#include <Engine/Json/ValueBase.hpp>
#include <utility>

void Scene::Start(Application* app, std::stack<SceneHandle>* scenes)
{
	m_app    = app;
	m_scenes = scenes;

	PrimaryCamera = OnStart(app->GetKeyboard(), app->GetMouse());
}

void Scene::Update(float delta, KeyboardDevice& keyboard, MouseDevice& mouse)
{
	for(const auto& system : m_updaterSystems)
	{
		if(system->IsEnabled)
			system->OnUpdate(*this, delta, keyboard, mouse);
	}
	OnUpdate(delta, keyboard, mouse);
}

void Scene::Render(RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target)
{
	target.Clear(ColorBuffer | DepthBuffer);
	
	for(const auto& system : m_rendererSystems)
	{
		if(system->IsEnabled)
		{
			system->OnRender(*this, renderDevice, renderContext2D, target);
		}
	}
	OnRender(renderDevice, renderContext2D, target);
}

void Scene::UpdateAndRenderUI(UIContext& context)
{
	OnUIRender(context);
}

//GraphicsMode Scene::GraphicsMode() const { return m_app->GetGraphicsMode(); }

ScreenGraphicsMode Scene::SelectedGraphicsMode() const { return m_app->GetGraphicsMode(); }

void Scene::Close()
{
    m_scenes->pop();
}

AudioSourceHandle Scene::CreateAudioSource()
{
	return m_app->GetAudioDevice().CreateAudioSource();
}

MeshHandle Scene::CreateMesh(const Model& model) const
{
    return m_app->GetRenderDevice().CreateMesh(model);
}

TextureAtlasHandle Scene::CreateTextureAtlas(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) const
{
    return m_app->GetRenderDevice().CreateTextureAtlas(image, minFilter, magFilter, wrappingMode);
}

TextureAtlasHandle Scene::CreateCubeMap(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) const
{
	return m_app->GetRenderDevice().CreateCubeMap(image, minFilter, magFilter, wrappingMode);
}

RenderTargetHandle Scene::CreateRenderTarget(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) const
{
	return m_app->GetRenderDevice().CreateRenderTarget(width, height, colorAttachments, depthAttachment);
}

ShaderHandle Scene::CreateShader(std::string_view sourceCode) const
{
	return m_app->GetRenderDevice().CreateShader(sourceCode);
}

SoundHandle Scene::CreateSound(ConstBufferSlice<float> samples) const
{
	return m_app->GetAudioDevice().CreateSound(samples);
}

FontHandle Scene::CreateFont(const BitmapBase& bitmap, std::istream& jsonStream) const
{
	TextureAtlasHandle textureAtlas = CreateTextureAtlas(bitmap);
	const std::shared_ptr<Json::Tree> tree = Json::Parse(jsonStream);
	std::vector<CharacterAttributes> characters;
	const auto* rootObject = tree->Root->As<Json::Object>();
	const auto* characterObjects = rootObject->Get("characters")->As<Json::Object>();
	const auto* sizeValue = rootObject->Get("size")->As<Json::Value<int64_t>>();
	for(const auto& pair : *characterObjects)
	{
		char c = pair.first[0];
		const Json::Object* attributesObject = pair.second->As<Json::Object>();

		characters.emplace_back(c,
		glm::vec2
		{
			attributesObject->Get("x")->As<Json::Value<int64_t>>()->LiteralValue,
			attributesObject->Get("y")->As<Json::Value<int64_t>>()->LiteralValue,
		},
		glm::vec2
		{
			attributesObject->Get("width")->As<Json::Value<int64_t>>()->LiteralValue,
			attributesObject->Get("height")->As<Json::Value<int64_t>>()->LiteralValue,
		},
		glm::vec2
		{
			attributesObject->Get("originX")->As<Json::Value<int64_t>>()->LiteralValue,
			attributesObject->Get("originY")->As<Json::Value<int64_t>>()->LiteralValue,
		},
		attributesObject->Get("advance")->As<Json::Value<int64_t>>()->LiteralValue);
	}

	return std::make_shared<Font>(textureAtlas, sizeValue->LiteralValue, characters);
}

static void CalculateTangents(std::vector<DefaultVertex>& vertices, const std::vector<uint32_t>& indices)
{
	for(size_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t i0 = indices[i    ];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		glm::vec3 edge1 = vertices[i1].Position - vertices[i0].Position;
		glm::vec3 edge2 = vertices[i2].Position - vertices[i0].Position;

		float deltaU1 = vertices[i1].TexCoord.x - vertices[i0].TexCoord.x;
		float deltaV1 = vertices[i1].TexCoord.y - vertices[i0].TexCoord.y;
		float deltaU2 = vertices[i2].TexCoord.x - vertices[i0].TexCoord.x;
		float deltaV2 = vertices[i2].TexCoord.y - vertices[i0].TexCoord.y;

		float dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
		//TODO: The first 0.0f may need to be changed to 1.0f here.
		float f = dividend == 0 ? 0.0f : 1.0f / dividend;

		glm::vec3 tangent(0, 0, 0);
		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		vertices[i0].Tangent = vertices[i0].Tangent + tangent;
		vertices[i1].Tangent = vertices[i1].Tangent + tangent;
		vertices[i2].Tangent = vertices[i2].Tangent + tangent;
	}
}

static void CalculateNormals(std::vector<DefaultVertex>& vertices, const std::vector<uint32_t>& indices)
{
	for(size_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t i0 = indices[i    ];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		glm::vec3 v1 = vertices[i1].Position - vertices[i0].Position;
		glm::vec3 v2 = vertices[i2].Position - vertices[i0].Position;

		glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

		vertices[i0].Normal -= normal;
		vertices[i1].Normal -= normal;
		vertices[i2].Normal -= normal;
	}

	for(auto& vertex : vertices)
	{
		vertex.Normal = glm::normalize(vertex.Normal);
	}
}


MeshHandle Scene::CreateQuad() const
{
	std::vector<DefaultVertex> vertices =
	{
		{ glm::vec3(-1, 0,  1), glm::vec2(0, 0), glm::vec3(0, 1, 0) },
		{ glm::vec3( 1, 0,  1), glm::vec2(1, 0), glm::vec3(0, 1, 0) },
		{ glm::vec3(-1, 0, -1), glm::vec2(0, 1), glm::vec3(0, 1, 0) },
		{ glm::vec3( 1, 0, -1), glm::vec2(1, 1), glm::vec3(0, 1, 0) },
	};

	const std::vector<uint32_t> indices =
	{
		0, 2, 3,
		3, 1, 0
	};

	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCube() const
{
	glm::vec2 bl(0, 0);
	glm::vec2 br(1, 0);
	glm::vec2 tl(0, 1);
	glm::vec2 tr(1, 1);

	std::vector<DefaultVertex> vertices =
	{
		{ glm::vec3(-1, -1,  1), br, glm::vec3(-1, 0, 0) },
		{ glm::vec3(-1,  1,  1), tr, glm::vec3(-1, 0, 0) },
		{ glm::vec3(-1, -1, -1), bl, glm::vec3(-1, 0, 0) },
		{ glm::vec3(-1,  1, -1), tl, glm::vec3(-1, 0, 0) },

		{ glm::vec3(1, -1,  1), bl, glm::vec3(1, 0, 0) },
		{ glm::vec3(1, -1, -1), br, glm::vec3(1, 0, 0) },
		{ glm::vec3(1,  1,  1), tl, glm::vec3(1, 0, 0) },
		{ glm::vec3(1,  1, -1), tr, glm::vec3(1, 0, 0) },

		{ glm::vec3(-1, -1,   1), tl, glm::vec3(0, -1, 0) },
		{ glm::vec3(-1, -1,  -1), bl, glm::vec3(0, -1, 0) },
		{ glm::vec3( 1, -1,   1), tr, glm::vec3(0, -1, 0) },
		{ glm::vec3( 1, -1,  -1), br, glm::vec3(0, -1, 0) },

		{ glm::vec3(-1, 1,  1), bl, glm::vec3(0, 1, 0) },
		{ glm::vec3( 1, 1,  1), br, glm::vec3(0, 1, 0) },
		{ glm::vec3(-1, 1, -1), tl, glm::vec3(0, 1, 0) },
		{ glm::vec3( 1, 1, -1), tr, glm::vec3(0, 1, 0) },

		{ glm::vec3(-1, -1, 1), bl, glm::vec3(0, 0, 1) },
		{ glm::vec3( 1, -1, 1), br, glm::vec3(0, 0, 1) },
		{ glm::vec3(-1,  1, 1), tl, glm::vec3(0, 0, 1) },
		{ glm::vec3( 1,  1, 1), tr, glm::vec3(0, 0, 1) },

		{ glm::vec3(-1, -1, -1), br, glm::vec3(0, 0, -1) },
		{ glm::vec3(-1,  1, -1), tr, glm::vec3(0, 0, -1) },
		{ glm::vec3( 1, -1, -1), bl, glm::vec3(0, 0, -1) },
		{ glm::vec3( 1,  1, -1), tl, glm::vec3(0, 0, -1) }
	};

	size_t faceIndices[] =
	{
		0, 2, 3,
		3, 1, 0,
	};

	std::vector<uint32_t> indices;
	indices.reserve(36);

	for(size_t i = 0; i < 6; i++)
	{
		for(size_t faceIndex : faceIndices)
		{
			indices.push_back(static_cast<uint32_t>(faceIndex + i * 4));
		}
	}

	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreatePyramid() const
{
	std::vector<DefaultVertex> vertices =
	{
		{ glm::vec3(-1, -1, -1), glm::vec2(0   , 0), glm::vec3() },
		{ glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3() },
		{ glm::vec3(-1, -1,  1), glm::vec2(1   , 0), glm::vec3() },
		{ glm::vec3(-1, -1,  1), glm::vec2(0   , 0), glm::vec3() },

		{ glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()},
		{ glm::vec3( 1, -1,  1), glm::vec2(1   , 0), glm::vec3()},
		{ glm::vec3( 1, -1,  1), glm::vec2(0   , 0), glm::vec3()},
		{ glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()},

		{ glm::vec3( 1, -1, -1), glm::vec2(1   , 0), glm::vec3()},
		{ glm::vec3( 1, -1, -1), glm::vec2(0   , 0), glm::vec3()},
		{ glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()},
		{ glm::vec3(-1, -1, -1), glm::vec2(1   , 0), glm::vec3()},

		{ glm::vec3(-1, -1,  1), glm::vec2(0, 1), glm::vec3() },
		{ glm::vec3(-1, -1, -1), glm::vec2(0, 0), glm::vec3() },
		{ glm::vec3( 1, -1,  1), glm::vec2(1, 1), glm::vec3() },
		{ glm::vec3( 1, -1, -1), glm::vec2(1, 0), glm::vec3() },
	};

    std::vector<uint32_t> faceIndices =
    {
        0, 1, 2
    };

	std::vector<uint32_t> indices;
	indices.resize(18);
    for(size_t i = 0; i < 4; i++)
    {
        for(int j = 0; j < faceIndices.size(); j++)
            indices[i * 3 + j] = uint32_t(faceIndices[j] + i * 3);
    }

    indices[12] = 12;
    indices[13] = 14;
    indices[14] = 13;

    indices[15] = 14;
    indices[16] = 15;
    indices[17] = 13;

    CalculateNormals(vertices, indices);
	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCone(uint32_t detail) const
{
	std::vector<DefaultVertex> vertices;
    vertices.emplace_back(glm::vec3(0,  1, 0), glm::vec2(0.5f, -1), glm::vec3());
    vertices.emplace_back(glm::vec3(0, -1, 0), glm::vec2(0.5f, -1), glm::vec3());

    for(float a = 0; a < 360; a += 360.0f / static_cast<float>(detail))
    {
        const float x = cosf(glm::radians(a));
        const float z = sinf(glm::radians(a));

        vertices.emplace_back(glm::vec3(x, -1, -z), glm::vec2(a / 360.0f, 0), glm::vec3());
    }

    std::vector<uint32_t> indices;
	indices.resize(static_cast<size_t>(detail) * 6);

    size_t k = 0;
    for(size_t j = 2; j < vertices.size(); j++)
    {
        const auto index0 = static_cast<uint32_t>(j);
		      auto index1 = static_cast<uint32_t>(j + 1);

        if(index1 >= vertices.size())
        {
	        index1 -= static_cast<uint32_t>(vertices.size()) - 2;
        }

        indices[k    ] = 1;
        indices[k + 1] = index0;
        indices[k + 2] = index1;

        indices[k + 3] = index1;
        indices[k + 4] = index0;
        indices[k + 5] = 0;

        k += 6;
    }

	CalculateNormals(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCylinder(uint32_t detail) const
{
	std::vector<DefaultVertex> vertices;
    vertices.emplace_back(glm::vec3(0,  1, 0), glm::vec2(0.5f, -1), glm::vec3());
	vertices.emplace_back(glm::vec3(0, -1, 0), glm::vec2(0.5f, -1), glm::vec3());

    for(float a = 0; a < 360; a += 360.0f / static_cast<float>(detail))
    {
        const float x = cosf(glm::radians(a));
        const float z = sinf(glm::radians(a));

        float texX;

        if(a <= 180)
        {
	        texX = a / 180.0f;
        }
        else
    	{
    		texX = a / 180.0f - 1.0f;
    	}

        vertices.emplace_back(glm::vec3(x,  1, -z), glm::vec2(texX, 1), glm::vec3());
        vertices.emplace_back(glm::vec3(x, -1, -z), glm::vec2(texX, 0), glm::vec3());
    }

    std::vector<uint32_t> indices;
	indices.resize(static_cast<size_t>(detail) * 12);

    size_t k = 0;
    for(size_t j = 2; j < vertices.size(); j += 2)
    {
        const auto index0 = static_cast<uint32_t>(j);
              auto index1 = static_cast<uint32_t>(j + 1);
              auto index2 = static_cast<uint32_t>(j + 2);
              auto index3 = static_cast<uint32_t>(j + 3);

        if(index3 >= vertices.size())
        {
	        index3 -= static_cast<uint32_t>(vertices.size() - 2);
        }

        if(index2 >= vertices.size())
        {
	        index2 -= static_cast<uint32_t>(vertices.size() - 2);
        }

        if(index1 >= vertices.size())
        {
	        index1 -= static_cast<uint32_t>(vertices.size() - 3);
        }

        indices[k     ] = index2;
        indices[k +  1] = index0;
        indices[k +  2] = 0;

        indices[k +  3] = 1;
        indices[k +  4] = index1;
        indices[k +  5] = index3;

        indices[k +  6] = index2;
        indices[k +  7] = index1;
        indices[k +  8] = index0;

        indices[k +  9] = index2;
        indices[k + 10] = index3;
        indices[k + 11] = index1;

        k += 12;
    }

	CalculateNormals(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateSphere(uint32_t detail) const
{
	uint32_t sectorCount = detail;
    uint32_t stackCount  = detail;

    std::vector<DefaultVertex> vertices;

	const float sectorStep = 2 * glm::pi<float>() / static_cast<float>(sectorCount);
    const float stackStep = glm::pi<float>() / static_cast<float>(stackCount);

	for(size_t i = 0; i <= stackCount; ++i)
    {
        const float stackAngle = glm::pi<float>() / 2 - static_cast<float>(i) * stackStep;
        const float xy = cosf(stackAngle);
        const float zPos = sinf(stackAngle);

        for(size_t j = 0; j <= sectorCount; ++j)
        {
	        constexpr float lengthInv = 1.0f;
	        const float sectorAngle = static_cast<float>(j) * sectorStep;

            const float xPos = xy * cosf(sectorAngle);
            const float yPos = xy * sinf(sectorAngle);

            const float xNorm = xPos * lengthInv;
            const float yNorm = yPos * lengthInv;
            const float zNorm = zPos * lengthInv;

            const float xTex = static_cast<float>(j) / static_cast<float>(sectorCount);
            const float yTex = static_cast<float>(i) / static_cast<float>(stackCount);

            vertices.emplace_back(glm::vec3(xPos, yPos, -zPos), glm::vec2(xTex, yTex), glm::vec3(xNorm, yNorm, -zNorm));
        }
    }

	std::vector<uint32_t> indices;

	for(uint32_t i = 0; i < stackCount; ++i)
    {
        uint32_t k1 = i * (sectorCount + 1);
        uint32_t k2 = k1 + sectorCount + 1;

        for(uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if(i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateTerrain(const HeightMap& heightMap) const
{
	std::vector<DefaultVertex> vertices;

	const uint32_t width  = heightMap.SourceBitmap.Width;
	const uint32_t height = heightMap.SourceBitmap.Height;

	for(uint32_t z = 0; z < height; z++)
	{
		for(uint32_t x = 0; x < width; x++)
		{
			const float u = static_cast<float>(x) / static_cast<float>(width);
			const float v = static_cast<float>(z) / static_cast<float>(height);

			const float heightValue = heightMap.Get(x, z);
			vertices.emplace_back(glm::vec3(u * 2 - 1, heightValue * 2 - 1, -(v * 2 - 1)), glm::vec2(u, v), glm::vec3(0));
		}
	}

	std::vector<uint32_t> indices;

	for(size_t z = 0; z < height - 1ULL; z++)
	{
		for(size_t x = 0; x < width - 1ULL; x++)
		{
			auto index = static_cast<uint32_t>(z * width + x);

			indices.push_back(index);
			indices.push_back(index + width);
			indices.push_back(index + 1);

			indices.push_back(index + 1);
			indices.push_back(index + width);
			indices.push_back(index + width + 1);
			
		}
	}

	CalculateNormals(vertices, indices);
	CalculateTangents(vertices, indices);
	
	return CreateMesh(Model(vertices, indices));
}

void Scene::PlayMusic(MusicHandle music, bool loop) 
{
	m_isMusicPaused = false;
	m_app->GetAudioDevice().PlayMusic(std::move(music), loop);
}

void Scene::StopMusic()
{
	m_app->GetAudioDevice().StopMusic();
}

void Scene::PauseMusic(bool pause)
{
	m_isMusicPaused = pause;
	m_app->GetAudioDevice().PauseMusic(pause); 
}

void Scene::TogglePauseMusic()
{
	PauseMusic(!m_isMusicPaused);
}
