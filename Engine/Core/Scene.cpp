#include "Scene.hpp"

#include "Application.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <span>
#include <array>
#include <string>
#include <fstream>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Rendering/RenderContext2D.hpp"
#include "../Json/Json.hpp"
#include <Engine/Json/Value.hpp>

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

void Scene::Render(RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target)
{
	target->Bind();
	renderDevice.ClearScreen(ColorBuffer | DepthBuffer);
	
	for(const auto& system : m_rendererSystems)
	{
		if(system->IsEnabled)
			system->OnRender(*this, renderDevice, renderContext2D, target);
	}
	OnRender(renderDevice, renderContext2D, target);
}

void Scene::UpdateAndRenderUI(UIContext& context)
{
	OnUIRender(context);
}

//GraphicsMode Scene::GraphicsMode() const { return m_app->GetGraphicsMode(); }

GraphicsMode Scene::GraphicsMode() const { return m_app->GetGraphicsMode(); }

void Scene::Close()
{
    m_scenes->pop();
}

MaterialHandle Scene::CreateMaterial(ShaderHandle type)
{
	std::shared_ptr<MaterialList> list;

	auto it = m_materialLists.find(type);
	if(it != m_materialLists.end())
	{
		list = it->second;
	}
	else
	{
		list = std::make_shared<MaterialList>(type);
		m_materialLists[type] = list;
	}

	MaterialHandle result = std::make_shared<Material>(list);
	return result;
}

AudioSourceHandle Scene::CreateAudioSource()
{
	return m_app->GetAudioDevice().CreateAudioSource();
}

MeshHandle Scene::CreateMesh(const Model& model)
{
    return m_app->GetRenderDevice().CreateMesh(model);
}

TextureHandle Scene::CreateTexture(BaseBitmapHandle image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
    return m_app->GetRenderDevice().CreateTexture(*image, minFilter, magFilter, wrappingMode);
}

TextureHandle Scene::CreateCubeMap(BaseBitmapHandle image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
	return m_app->GetRenderDevice().CreateCubeMap(*image, minFilter, magFilter, wrappingMode);
}

FrameBufferHandle Scene::CreateFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment)
{
	return m_app->GetRenderDevice().CreateFrameBuffer(width, height, colorAttachments, depthAttachment);
}


static Model ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	Array<DefaultVertex> vertices;
	std::vector<uint32_t> indices;
	//std::vector<Texture> textures;

	for(size_t i = 0; i < mesh->mNumVertices; i++)
	{
		auto position = mesh->mVertices[i];

		auto texCoord = aiVector3D(0);
		
		if(mesh->mTextureCoords[0])
		{
			texCoord = mesh->mTextureCoords[0][i];
		}

		auto normal = aiVector3D(0);

		if(mesh->mNormals)
		{
			normal = mesh->mNormals[i];
		}

		auto tangent = aiVector3D(0);

		if(mesh->mTangents)
		{
			tangent = mesh->mTangents[i];
		}

		vertices.Emplace(
			glm::vec3(position.x, position.y, position.z), 
			glm::vec2(texCoord.x, texCoord.y),
			glm::vec3(normal.x, normal.y, normal.z), 
			glm::vec3(tangent.x, tangent.y, tangent.z));
	}

	for(size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(size_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}  

	return Model(vertices, indices);
}

static void ProcessNode(std::vector<Model>& models, aiNode *node, const aiScene *scene)
{
    for(size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
		models.push_back(ProcessMesh(mesh, scene));
    }

    for(size_t i = 0; i < node->mNumChildren; i++)
        ProcessNode(models, node->mChildren[i], scene);
} 

MeshHandle Scene::LoadMesh(const std::string& fileName, bool smoothNormals, const std::string& resourceFolderName)
{
	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FlipWindingOrder;
	if(smoothNormals)
	{
		flags |= aiProcess_GenSmoothNormals;
	}
	else
	{
		flags |= aiProcess_GenNormals;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(m_resourcesPath + "/" + resourceFolderName + "/" + fileName, flags);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    //directory = path.substr(0, path.find_last_of('/'));

	std::vector<Model> models;

    ProcessNode(models, scene->mRootNode, scene);

	return CreateMesh(models[0]);
}

MeshHandle Scene::LoadTerrain(const std::string& fileName, const std::string& resourceFolderName)
{
	return CreateTerrain(LoadBitmap<uint8_t>(fileName, resourceFolderName));
}

BaseBitmapHandle Scene::LoadBitmap(std::span<std::string> facesFileNames, PixelFormat desiredFormat, const std::string& resourceFolderName)
{
	std::vector<uint8_t> resultPixels;

	int oldWidth  = 0;
	int oldHeight = 0;

	size_t offset = 0;

	for(size_t i = 0; i < facesFileNames.size(); i++)
	{
		int width;
		int height;
		int channels;

		const std::string& fileName = facesFileNames[i];
		uint8_t* pixels = stbi_load((m_resourcesPath + "/" + resourceFolderName + "/" + fileName).c_str(), &width, &height, &channels, int(desiredFormat.ChannelCount));
		if(!pixels)
		{
			return nullptr;
		}

		if(oldWidth != 0 && oldHeight != 0 && width != oldWidth && height != oldHeight)
		{
			stbi_image_free(pixels);
			return nullptr;
		}
		
		oldWidth  = width;
		oldHeight = height;
		
		size_t faceSize = size_t(width) * height * desiredFormat.Size;

		resultPixels.resize(resultPixels.size() + faceSize);

		memcpy(resultPixels.data() + offset, pixels, faceSize);

		offset = resultPixels.size();

		stbi_image_free(pixels);
	}

	BaseBitmapHandle result = desiredFormat.CreateBitmap(oldWidth, oldHeight, uint32_t(facesFileNames.size()));
	memcpy(result->GetPixels(), resultPixels.data(), result->PixelCount * desiredFormat.Size);

	return result;
}

BaseBitmapHandle Scene::LoadBitmap(const std::string& fileName, PixelFormat format, const std::string& resourceFolderName)
{
	int width;
	int height;
	uint8_t* pixels = stbi_load((m_resourcesPath + "/" + resourceFolderName + "/" + fileName).c_str(), &width, &height, nullptr, int(format.ChannelCount));

	BaseBitmapHandle result = format.CreateBitmap(width, height, 1);
	memcpy(result->GetPixels(), pixels, result->PixelCount * format.Size);
	stbi_image_free(pixels);
	return result;
}

TextureHandle Scene::LoadTexture(const std::string& fileName, PixelFormat format, const std::string& resourceFolderName, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
	auto bitmap = LoadBitmap(fileName, format, resourceFolderName);
	return CreateTexture(bitmap, minFilter, magFilter, wrappingMode);
}

TextureHandle Scene::LoadCubeMap(const std::string& left, const std::string& right, const std::string& bottom, const std::string& top, const std::string& back, const std::string& front, PixelFormat format, const std::string& resourceFolderName, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
	std::array<std::string, 6> fileNames = { left, right, bottom, top, back, front };
	return CreateCubeMap(LoadBitmap(fileNames, format, resourceFolderName), minFilter, magFilter, wrappingMode);
}

ShaderHandle Scene::LoadShader(const std::string& vertexShaderFileName, const std::string& fragmentShaderFileName, const std::string& resourceFolderName)
{
    std::ifstream vsStream;
	vsStream.open(m_resourcesPath + "/" + resourceFolderName + "/" + vertexShaderFileName);

	std::ifstream fsStream;
	fsStream.open(m_resourcesPath + "/" + resourceFolderName + "/" + fragmentShaderFileName);

	std::string line;

	std::string vertexShaderText;
	while(std::getline(vsStream, line)) { vertexShaderText += line + "\n"; }

	std::string fragmentShaderText;
	while(std::getline(fsStream, line)) { fragmentShaderText += line + "\n"; }

	vsStream.close();
	fsStream.close();

	return m_app->GetRenderDevice().CreateShader(vertexShaderText, fragmentShaderText);
}

FontHandle Scene::LoadFont(const std::string& fontName, const std::string& resourceFolderName)
{
	TextureHandle textureAtlas = LoadTexture<glm::u8vec4>(fontName + ".png", resourceFolderName);
	std::shared_ptr<Json::Tree> tree = Json::Parse(m_resourcesPath + "/" + resourceFolderName + "/" + fontName + ".json");
	std::vector<CharacterAttributes> characters;
	const Json::Object* rootObject = tree->Root->As<Json::Object>();
	const Json::Object* characterObjects = rootObject->Get("characters")->As<Json::Object>();
	const Json::TypedValue<int64_t>* sizeValue = rootObject->Get("size")->As<Json::TypedValue<int64_t>>();
	for(const auto& pair : *characterObjects)
	{
		char c = pair.first[0];
		const Json::Object* attributesObject = pair.second->As<Json::Object>();

		characters.emplace_back(c,
		glm::vec2
		{
			attributesObject->Get("x")->As<Json::TypedValue<int64_t>>()->LiteralValue,
			attributesObject->Get("y")->As<Json::TypedValue<int64_t>>()->LiteralValue,
		},
		glm::vec2
		{
			attributesObject->Get("width")->As<Json::TypedValue<int64_t>>()->LiteralValue,
			attributesObject->Get("height")->As<Json::TypedValue<int64_t>>()->LiteralValue,
		},
		glm::vec2
		{
			attributesObject->Get("originX")->As<Json::TypedValue<int64_t>>()->LiteralValue,
			attributesObject->Get("originY")->As<Json::TypedValue<int64_t>>()->LiteralValue,
		},
		attributesObject->Get("advance")->As<Json::TypedValue<int64_t>>()->LiteralValue);
	}

	return std::make_shared<Font>(textureAtlas, sizeValue->LiteralValue, characters);
}

SoundHandle Scene::LoadSound(const std::string& fileName, const std::string& resourceFolderName)
{
	return m_app->GetAudioDevice().CreateSound(m_resourcesPath + "/" + resourceFolderName + "/" + fileName);
}

MusicHandle Scene::LoadMusic(const std::string& fileName, const std::string& resourceFolderName)
{
	return m_app->GetAudioDevice().CreateMusic(m_resourcesPath + "/" + resourceFolderName + "/" + fileName);
}

void Scene::CalculateTangents(Array<DefaultVertex>& vertices, const std::vector<uint32_t>& indices)
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

static void CalcNormals(Array<DefaultVertex>& vertices, const std::vector<uint32_t>& indices)
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

	for(size_t i = 0; i < vertices.Count(); i++)
		vertices[i].Normal = glm::normalize(vertices[i].Normal);
}


MeshHandle Scene::CreateQuad()
{
	Array<DefaultVertex> vertices =
	{
		DefaultVertex(glm::vec3(-1, 0,  1), glm::vec2(0, 0), glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3( 1, 0,  1), glm::vec2(1, 0), glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3(-1, 0, -1), glm::vec2(0, 1), glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3( 1, 0, -1), glm::vec2(1, 1), glm::vec3(0, 1, 0)),
	};

	std::vector<uint32_t> indices =
	{
		0, 2, 3,
		3, 1, 0
	};

	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCube()
{
	glm::vec2 bl(0, 0);
	glm::vec2 br(1, 0);
	glm::vec2 tl(0, 1);
	glm::vec2 tr(1, 1);

	Array<DefaultVertex> vertices =
	{
		DefaultVertex(glm::vec3(-1, -1,  1), br, glm::vec3(-1, 0, 0)),
		DefaultVertex(glm::vec3(-1,  1,  1), tr, glm::vec3(-1, 0, 0)),
		DefaultVertex(glm::vec3(-1, -1, -1), bl, glm::vec3(-1, 0, 0)),
		DefaultVertex(glm::vec3(-1,  1, -1), tl, glm::vec3(-1, 0, 0)),

		DefaultVertex(glm::vec3(1, -1,  1), bl, glm::vec3(1, 0, 0)),
		DefaultVertex(glm::vec3(1, -1, -1), br, glm::vec3(1, 0, 0)),
		DefaultVertex(glm::vec3(1,  1,  1), tl, glm::vec3(1, 0, 0)),
		DefaultVertex(glm::vec3(1,  1, -1), tr, glm::vec3(1, 0, 0)),

		DefaultVertex(glm::vec3(-1, -1,   1), tl, glm::vec3(0, -1, 0)),
		DefaultVertex(glm::vec3(-1, -1,  -1), bl, glm::vec3(0, -1, 0)),
		DefaultVertex(glm::vec3( 1, -1,   1), tr, glm::vec3(0, -1, 0)),
		DefaultVertex(glm::vec3( 1, -1,  -1), br, glm::vec3(0, -1, 0)),

		DefaultVertex(glm::vec3(-1, 1,  1), bl, glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3( 1, 1,  1), br, glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3(-1, 1, -1), tl, glm::vec3(0, 1, 0)),
		DefaultVertex(glm::vec3( 1, 1, -1), tr, glm::vec3(0, 1, 0)),

		DefaultVertex(glm::vec3(-1, -1, 1), bl, glm::vec3(0, 0, 1)),
		DefaultVertex(glm::vec3( 1, -1, 1), br, glm::vec3(0, 0, 1)),
		DefaultVertex(glm::vec3(-1,  1, 1), tl, glm::vec3(0, 0, 1)),
		DefaultVertex(glm::vec3( 1,  1, 1), tr, glm::vec3(0, 0, 1)),

		DefaultVertex(glm::vec3(-1, -1, -1), br, glm::vec3(0, 0, -1)),
		DefaultVertex(glm::vec3(-1,  1, -1), tr, glm::vec3(0, 0, -1)),
		DefaultVertex(glm::vec3( 1, -1, -1), bl, glm::vec3(0, 0, -1)),
		DefaultVertex(glm::vec3( 1,  1, -1), tl, glm::vec3(0, 0, -1))
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
		for(size_t j = 0; j < 6; j++)
			indices.push_back(uint32_t(faceIndices[j] + i * 4));
	}

	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreatePyramid()
{
	Array<DefaultVertex> vertices =
	{
		DefaultVertex(glm::vec3(-1, -1, -1), glm::vec2(0   , 0), glm::vec3()), 
		DefaultVertex(glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()), 
		DefaultVertex(glm::vec3(-1, -1,  1), glm::vec2(1   , 0), glm::vec3()), 

		DefaultVertex(glm::vec3(-1, -1,  1), glm::vec2(0   , 0), glm::vec3()), 
		DefaultVertex(glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()), 
		DefaultVertex(glm::vec3( 1, -1,  1), glm::vec2(1   , 0), glm::vec3()), 

		DefaultVertex(glm::vec3( 1, -1,  1), glm::vec2(0   , 0), glm::vec3()),
		DefaultVertex(glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()),
		DefaultVertex(glm::vec3( 1, -1, -1), glm::vec2(1   , 0), glm::vec3()),

		DefaultVertex(glm::vec3( 1, -1, -1), glm::vec2(0   , 0), glm::vec3()),
		DefaultVertex(glm::vec3( 0,  1,  0), glm::vec2(0.5f, 1), glm::vec3()),
		DefaultVertex(glm::vec3(-1, -1, -1), glm::vec2(1   , 0), glm::vec3()),

		DefaultVertex(glm::vec3(-1, -1,  1), glm::vec2(0, 1), glm::vec3()),
		DefaultVertex(glm::vec3(-1, -1, -1), glm::vec2(0, 0), glm::vec3()),
		DefaultVertex(glm::vec3( 1, -1,  1), glm::vec2(1, 1), glm::vec3()),
		DefaultVertex(glm::vec3( 1, -1, -1), glm::vec2(1, 0), glm::vec3()),
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

    CalcNormals(vertices, indices);
	CalculateTangents(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCone(uint32_t detail)
{
	Array<DefaultVertex> vertices;
    vertices.Emplace(glm::vec3(0,  1, 0), glm::vec2(0.5f, -1), glm::vec3());
    vertices.Emplace(glm::vec3(0, -1, 0), glm::vec2(0.5f, -1), glm::vec3());

    for(float a = 0; a < 360; a += 360 / float(detail))
    {
        float x = cosf(glm::radians(a));
        float z = sinf(glm::radians(a));

        vertices.Emplace(glm::vec3(x, -1, -z), glm::vec2(a / 360.0f, 0), glm::vec3());
    }

    std::vector<uint32_t> indices;
	indices.resize(size_t(detail) * 6);

    size_t k = 0;
    for(size_t j = 2; j < vertices.Count(); j++)
    {
        uint32_t v0 = uint32_t(j);
		uint32_t v1 = uint32_t(j + 1);

        if(v1 >= vertices.Count())
            v1 -= uint32_t(vertices.Count()) - 2;

        indices[k    ] = 1;
        indices[k + 1] = v0;
        indices[k + 2] = v1;

        indices[k + 3] = v1;
        indices[k + 4] = v0;
        indices[k + 5] = 0;

        k += 6;
    }

	CalcNormals(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateCylinder(uint32_t detail)
{
	Array<DefaultVertex> vertices;
    vertices.Emplace(glm::vec3(0,  1, 0), glm::vec2(0.5f, -1), glm::vec3());
	vertices.Emplace(glm::vec3(0, -1, 0), glm::vec2(0.5f, -1), glm::vec3());

    for(float a = 0; a < 360; a += 360 / float(detail))
    {
        float x = cosf(glm::radians(a));
        float z = sinf(glm::radians(a));

        float texX;

        if(a <= 180)
            texX = a / 180.0f;
        else
            texX = (a / 180.0f) - 1.0f;

        vertices.Emplace(glm::vec3(x,  1, -z), glm::vec2(texX, 1), glm::vec3());
        vertices.Emplace(glm::vec3(x, -1, -z), glm::vec2(texX, 0), glm::vec3());
    }

    std::vector<uint32_t> indices;
	indices.resize(size_t(detail) * 12);

    size_t k = 0;
    for(size_t j = 2; j < vertices.Count(); j += 2)
    {
        uint32_t v0 = uint32_t(j);
        uint32_t v1 = uint32_t(j + 1);
        uint32_t v2 = uint32_t(j + 2);
        uint32_t v3 = uint32_t(j + 3);

        if(v3 >= vertices.Count())
            v3 -= uint32_t(vertices.Count() - 2);

        if(v2 >= vertices.Count())
            v2 -= uint32_t(vertices.Count() - 2);

        if(v1 >= vertices.Count())
            v1 -= uint32_t(vertices.Count() - 3);

        indices[k     ] = v2;
        indices[k +  1] = v0;
        indices[k +  2] = 0;

        indices[k +  3] = 1;
        indices[k +  4] = v1;
        indices[k +  5] = v3;

        indices[k +  6] = v2;
        indices[k +  7] = v1;
        indices[k +  8] = v0;

        indices[k +  9] = v2;
        indices[k + 10] = v3;
        indices[k + 11] = v1;

        k += 12;
    }

	CalcNormals(vertices, indices);
	return CreateMesh(Model(vertices, indices));
}

MeshHandle Scene::CreateSphere(uint32_t detail)
{
	uint32_t sectorCount = detail;
    uint32_t stackCount  = detail;

    Array<DefaultVertex> vertices;

    float xPos, yPos, zPos, xy;
    float xNorm, yNorm, zNorm, lengthInv = 1.0f;
    float xTex, yTex;

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for(size_t i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::pi<float>() / 2 - i * stackStep;
        xy = cosf(stackAngle);
        zPos = sinf(stackAngle);

        for(size_t j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;

            xPos = xy * cosf(sectorAngle);
            yPos = xy * sinf(sectorAngle);

            xNorm = xPos * lengthInv;
            yNorm = yPos * lengthInv;
            zNorm = zPos * lengthInv;

            xTex = float(j) / sectorCount;
            yTex = float(i) / stackCount;

            vertices.Emplace(glm::vec3(xPos, yPos, -zPos), glm::vec2(xTex, yTex), glm::vec3(xNorm, yNorm, -zNorm));
        }
    }

	std::vector<uint32_t> indices;

    uint32_t k1, k2;
    for(uint32_t i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

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

MeshHandle Scene::CreateTerrain(HeightMap heightMap)
{
	Array<DefaultVertex> vertices;

	uint32_t width  = heightMap.Bitmap.Width;
	uint32_t height = heightMap.Bitmap.Height;

	for(uint32_t z = 0; z < height; z++)
	{
		for(uint32_t x = 0; x < width; x++)
		{
			float u = x / float(width);
			float v = z / float(height);

			float heightValue = heightMap.Get(x, z);
			vertices.Emplace(glm::vec3(u * 2 - 1, heightValue * 2 - 1, -(v * 2 - 1)), glm::vec2(u, v), glm::vec3(0));
		}
	}

	std::vector<uint32_t> indices;

	for(size_t z = 0; z < height - 1ULL; z++)
	{
		for(size_t x = 0; x < width - 1ULL; x++)
		{
			uint32_t index = uint32_t(z * width + x);

			indices.push_back(index);
			indices.push_back(index + width);
			indices.push_back(index + 1);

			indices.push_back(index + 1);
			indices.push_back(index + width);
			indices.push_back(index + width + 1);
			
		}
	}

	CalcNormals(vertices, indices);
	CalculateTangents(vertices, indices);
	
	return CreateMesh(Model(vertices, indices));
}

void Scene::PlayMusic(MusicHandle music, bool loop) 
{
	m_isMusicPaused = false;
	m_app->GetAudioDevice().PlayMusic(music, loop); 
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
