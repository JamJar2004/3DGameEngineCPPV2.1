#define GLM_ENABLE_EXPERIMENTAL

#include <array>
#include <random>
#include <glm/gtx/hash.hpp>

#include <Engine/Core/Scene.hpp>
#include <Engine/Core/Game.hpp>

#include <Engine/EngineComponents/RotaterComponent.hpp>
#include <Engine/EngineComponents/AudioSourceComponent.hpp>
#include <Engine/EngineComponents/AnimationComponent.hpp>
#include <Engine/EngineComponents/FollowerComponent.hpp>
#include <Engine/EngineComponents/MouseLookComponent.hpp>

#include <Engine/EngineSystems/RotaterSystem.hpp>
#include <Engine/EngineSystems/DeferredRendererSystem.hpp>
#include <Engine/EngineSystems/WaterRendererSystem.hpp>
#include <Engine/EngineSystems/KeyboardMovementSystem.hpp>
#include <Engine/EngineSystems/MouseLookSystem.hpp>
#include <Engine/EngineSystems/AnimationSystem.hpp>
#include <Engine/EngineSystems/AudioUpdaterSystem.hpp>
#include <Engine/EngineSystems/FollowerSystem.hpp>
#include <Engine/EngineSystems/SelectEntitySystem.hpp>
#include <Engine/EngineSystems/WaterUpdaterSystem.hpp>

#include "Engine/Core/AssetFolder.hpp"
#include "Engine/Core/AssetLoaders/AssimpMeshLoader.hpp"
#include "Engine/EngineMaterials/NormalMappedMaterial.hpp"

namespace std
{
    std::mt19937 RandomGenerator(time(nullptr));

    static float randf() { return RandomGenerator() / float(std::numeric_limits<uint32_t>().max()); }

    static glm::vec2 rand2f() { return glm::vec2(randf(), randf()); }
    static glm::vec3 rand3f() { return glm::vec3(randf(), randf(), randf()); }
    static glm::vec4 rand4f() { return glm::vec4(randf(), randf(), randf(), randf()); }
}

static bool IsDoor(glm::u8vec3 pixel)
{
    return pixel != glm::u8vec3(0, 0, 0) && pixel != glm::u8vec3(255, 255, 255);
}

static bool IsKey(glm::u8vec3 pixel)
{
    return pixel != glm::u8vec3(0, 0, 0) && pixel != glm::u8vec3(255, 255, 255);
}

class MainScene : public Scene
{
private:
    std::unordered_set<glm::u8vec3> m_collectedKeys;

	static void AddQuad(std::vector<DefaultVertex>& vertices, std::vector<uint32_t>& indices, uint32_t i, uint32_t j, const glm::vec3& start, const glm::vec3& end, int axis, float direction, const glm::vec4& texCoords)
	{
        size_t offset = vertices.size();

        int axis1 = (axis + 1) % 3;
        int axis2 = (axis + 2) % 3;

        glm::vec3 pos1 = start;
        glm::vec3 pos2 = start;
        glm::vec3 pos3 = start;
        glm::vec3 pos4 = start;

        pos1[axis] = direction;
        pos2[axis] = direction;
        pos3[axis] = direction;
        pos4[axis] = direction;

        pos2[axis1] = end[axis1];
        pos3[axis2] = end[axis2];
                      
        pos4[axis1] = end[axis1];
        pos4[axis2] = end[axis2];

        glm::vec2 texCoords1;
        glm::vec2 texCoords2;
        glm::vec2 texCoords3;
        glm::vec2 texCoords4;

        switch(axis)
        { 
            case 0:
            case 1:
            {
                texCoords1 = glm::vec2(texCoords[0], texCoords[2]);
                texCoords2 = glm::vec2(texCoords[0], texCoords[3]);
                texCoords3 = glm::vec2(texCoords[1], texCoords[2]);
                texCoords4 = glm::vec2(texCoords[1], texCoords[3]);
                break;
            }
            case 2:
            {
                texCoords1 = glm::vec2(texCoords[0], texCoords[2]);
                texCoords2 = glm::vec2(texCoords[1], texCoords[2]);
                texCoords3 = glm::vec2(texCoords[0], texCoords[3]);
                texCoords4 = glm::vec2(texCoords[1], texCoords[3]);
                break;
            }
        }
        
        auto normal = glm::vec3(0);
        normal[axis] = -direction;

        vertices.emplace_back(pos1 + glm::vec3(j * 2.0f - 1.0f, 0, -(i * 2.0f - 1.0f)), texCoords1, glm::vec3(normal));
        vertices.emplace_back(pos2 + glm::vec3(j * 2.0f - 1.0f, 0, -(i * 2.0f - 1.0f)), texCoords2, glm::vec3(normal));
        vertices.emplace_back(pos3 + glm::vec3(j * 2.0f - 1.0f, 0, -(i * 2.0f - 1.0f)), texCoords3, glm::vec3(normal));
        vertices.emplace_back(pos4 + glm::vec3(j * 2.0f - 1.0f, 0, -(i * 2.0f - 1.0f)), texCoords4, glm::vec3(normal));

        if(direction == -1)
        {
            indices.push_back(offset + 2);
            indices.push_back(offset + 1);
            indices.push_back(offset    );
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
        }
        else
        {
            indices.push_back(offset    );
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
            indices.push_back(offset + 2);
            indices.push_back(offset + 1);
        }
        
	}

   /* glm::vec4 GetTexCoords(uint32_t atlasSize, uint8_t value)
    {
        uint32_t index = value / (atlasSize * atlasSize) - 1;

        uint32_t row    = index / atlasSize;
        uint32_t column = index % atlasSize;

        float texX = column / float(atlasSize);
        float texY = row    / float(atlasSize);

        return glm::vec4(texX, texX + (1.0f / atlasSize), texY + (1.0f / atlasSize), texY) * 0.9999f + 0.0001f;
    }*/

    static glm::vec4 GetTexCoords(int atlasSize, uint8_t value)
    {
        int index = value / (atlasSize * atlasSize) - 1;

        int row    = index / atlasSize;
        int column = index % atlasSize;

        float texX = column / float(atlasSize);
        float texY = row    / float(atlasSize);

        return glm::vec4(texX, texX + 1.0f / atlasSize, texY + 1.0f / atlasSize, texY) * 0.9999f + 0.0001f;
    }
public:
    MainScene() : Scene(1000) 
    {
    }

    static constexpr float PLAYER_HEIGHT = -0.4f;

	Camera OnStart(KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
        //auto& entityIdRenderer = AddSystem<EntityIdRendererSystem>();
        //auto& selectEntitySystem = AddSystem<MousePickSystem>(entityIdRenderer);

        auto deferredRendererContext = std::make_shared<DeferredRenderContext>(*this, glm::vec2(512));

		AddSystem<DeferredRendererSystem<NormalMappedMaterial>>(deferredRendererContext);
		auto& keyboardMovementSystem = AddSystem<KeyboardMovementSystem>();
	    AddSystem<MouseLookSystem>();
        AddSystem<AnimationSystem<float>>();
        AddSystem<AnimationSystem<glm::vec3>>();

        //AddSystem<WaterRendererSystem>(glm::uvec2(256));

        AddSystem<FollowerSystem>();
        AddSystem<AudioUpdaterSystem>();
        AddSystem<RotaterSystem>();
        AddSystem<WaterUpdaterSystem>();

		TextureHandle blankTexture = LoadTexture<glm::u8vec3>("blank.png");
        TextureHandle textureAtlas = LoadTexture<glm::u8vec3>("Texture Atlas.png");
        TextureHandle normalsAtlas = LoadTexture<glm::u8vec3>("Normal Map Atlas.png");

        BitmapHandle rustMap = LoadBitmap<uint8_t>("Door Rust Map.png");

        TextureHandle doorNormalMap = LoadTexture<glm::u8vec3>("Door Normal Map.png");

		ShaderHandle normalMappedShader = LoadShader("deferred/normalMapped.glsl", "deferred/normalMapped_FS.glsl");
		normalMappedShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1.0f));
		normalMappedShader->GetMaterialField("SpecularIntensity").SetDefaultValue(0.0f);
		normalMappedShader->GetMaterialField("TilingFactor").SetDefaultValue(glm::vec2(1.0f));

        ShaderHandle specularShader = LoadShader("deferred/specular.glsl", "deferred/specular_FS.glsl");
        specularShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1.0f));
        specularShader->GetMaterialField("SpecularIntensity").SetDefaultValue(0.0f);
        specularShader->GetMaterialField("TilingFactor").SetDefaultValue(glm::vec2(1.0f));

	    //auto defaultColor = *(glm::vec3*)specularShader->GetMaterialField("Color").GetDefaultValue();
        //std::cout << "Color: " << defaultColor.x << ", " << defaultColor.y << ", " << defaultColor.z << std::endl;

        MeshHandle shadowFigureMesh = CreateSphere();//LoadMesh("Shadow Figure.obj");

        SpecularMaterial blankMaterial = CreateMaterial(specularShader);
        blankMaterial.AlbedoTexture.Set(LoadTexture<glm::u8vec3>("blank.png"));

        AssetFolder folder;
        MeshHandle helloMesh = folder.Load("Hello.obj", AssimpModelLoader(true));


        MeshHandle doorMesh = CreateSphere();//LoadMesh("door.obj");
        MeshHandle  keyMesh = CreateSphere();//LoadMesh("key.obj");

        BitmapHandle<glm::u8vec3> levelBitmap = LoadBitmap<glm::u8vec3>("Levels/0/Structure.PNG");
        BitmapHandle<glm::u8vec3> doorMap = LoadBitmap<glm::u8vec3>("Levels/0/Door Map.PNG");
        BitmapHandle<glm::u8vec3> keyMap = LoadBitmap<glm::u8vec3>("Levels/0/Key Map.PNG");
		std::vector<DefaultVertex> vertices;
		std::vector<uint32_t> indices;
        
        TextureHandle silverGold = LoadTexture<glm::u8vec3>("Silver Gold.png");

        SoundHandle doorOpenSound = LoadSound("door open.wav");

        std::vector<glm::uvec2> shadowLocations;

        auto shadowEntities = std::make_shared<std::unordered_map<glm::uvec2, ECS::Entity>>();

        size_t count = 0;

        float  leftLength = 1.0f;
        float rightLength = 1.0f;
        float  backLength = 1.0f;
        float frontLength = 1.0f;
        
		for(uint32_t y = 1; y < levelBitmap->Height - 1; y++)
		{
			for(uint32_t x = 1; x < levelBitmap->Width - 1; x++)
			{
                const auto& left  = levelBitmap->GetPixel(x, y - 1);
                const auto& right = levelBitmap->GetPixel(x, y + 1);
                const auto& back  = levelBitmap->GetPixel(x - 1, y);
                const auto& front = levelBitmap->GetPixel(x + 1, y);

                const auto& center = levelBitmap->GetPixel(x, y);

				if(center != glm::u8vec3(0, 0, 0))
				{
                    size_t wallCount = 0;
                    if(left == glm::u8vec3(0, 0, 0) ^ right == glm::u8vec3(0, 0, 0)) { wallCount++; }
                    if(back == glm::u8vec3(0, 0, 0) ^ front == glm::u8vec3(0, 0, 0)) { wallCount++; }

                    count++;

                    if(wallCount == 2 && !(x == 1 && y == 1))
                    {
                        shadowLocations.emplace_back(x, y);
                    }

                    glm::vec4    wallTexCoords = GetTexCoords(4, center.r);
                    glm::vec4   floorTexCoords = GetTexCoords(4, center.g);
                    glm::vec4 ceilingTexCoords = GetTexCoords(4, center.b);

                    auto& doorPixel = doorMap->GetPixel(x, y);
                    auto& keyPixel  =  keyMap->GetPixel(x, y);

                    auto doorRotation = glm::identity<glm::quat>();
                    
                    if(left == glm::u8vec3(0, 0, 0))
                    {
                        AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 0, -1.0f, wallTexCoords);
                    }

                    if(right == glm::u8vec3(0, 0, 0))
                    {
                        if(IsDoor(doorPixel))
                        {
                            AddQuad(vertices, indices, x, y, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, -0.15f), 0, 1.0f, wallTexCoords * glm::vec4(1, 0.425f, 1, 1));
                            AddQuad(vertices, indices, x, y, glm::vec3(-1.0f, -1.0f, 0.15f), glm::vec3(1.0f, 1.0f,   1.0f), 0, 1.0f, wallTexCoords * glm::vec4(1, 0.425f, 1, 1));
                        }                                                   
                        else                                                
                        {                                                   
                            AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 0, 1.0f, wallTexCoords);
                        }
                    }

                    bool backOrFront = false;

                    if(back == glm::u8vec3(0, 0, 0))
                    {
                        if(IsDoor(doorPixel))
                        {
                            AddQuad(vertices, indices, x, y, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-0.15f, 1.0f, 1.0f), 2, 1.0f, wallTexCoords * glm::vec4(1, 0.425f, 1, 1));
                            AddQuad(vertices, indices, x, y, glm::vec3(0.15f, -1.0f, -1.0f), glm::vec3(  1.0f, 1.0f, 1.0f), 2, 1.0f, wallTexCoords * glm::vec4(1, 0.425f, 1, 1));
                        }
                        else
                        {
                            AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 2, 1.0f, wallTexCoords);
                        }
                        backOrFront = true;
                    }

                    if(front == glm::u8vec3(0, 0, 0))
                    {
                        AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 2, -1.0f, wallTexCoords);
                        backOrFront = true;
                    }
                    

                    if(!backOrFront)
                    {
                        doorRotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
                    }

                    if(IsKey(keyPixel))
                    {
                        MaterialHandle keyMaterial = CreateMaterial(specularShader);
                        keyMaterial->Get<glm::vec3>("Color") = glm::vec3(keyPixel) / 255.0f;
                        keyMaterial->Get<float>("SpecularIntensity") = 1.0f;

                        keyMaterial->SetTexture("Texture", blankTexture);

                        ECS::Entity key = CreateEntity(RenderableMesh(keyMesh, keyMaterial), ClickableComponent(keyMesh));
                        auto& keyTransformation = key.AddComponent<Transformation>(glm::vec3(y * 2.0f - 1.0f, -0.9f, -(x * 2.0f - 1.0f)));
                        keyTransformation.Rotate(glm::vec3(0, 1, 0), std::randf() * 360.0f);
                        keyTransformation.Scale = glm::vec3(0.1f);

                       /* key.SubscribeEvent(selectEntitySystem.OnEntityHover, [this, selectEntitySystem, keyPixel, &mouse, key]()
                        {
                            if(mouse.GetButtonState(MouseButton::Left) == ButtonState::Pressed)
                            {
                                m_collectedKeys.insert(keyPixel);
                                key.Delete();
                            }
                        });*/
                    }

                    //if(IsDoor(left) && !shadowLocations.contains(left))
                    //{
                    //    if(std::randf() > 0.2f)
                    //    {
                    //        shadowLocations.emplace(x, y - 1);

                    //        ECS::Entity shadowFigureEntity = CreateEntity(Transformation(glm::vec3(1.0f, -0.5f, -1.0f), glm::quat(1, 0, 0, 0), glm::vec3(0.5f)));

                    //        auto& shadowFigureMeshComponent = shadowFigureEntity.AddComponent<RenderableMesh>(shadowFigureMesh, blankMaterial);
                    //        shadowFigureMeshComponent.ShadowOnly = true;
                    //    }
                    //}

                    if(IsDoor(doorPixel))
                    {
                        MaterialHandle doorMaterial = CreateMaterial(normalMappedShader);
                        //doorMaterial->Get<glm::vec3>("Color") = glm::vec3(doorPixel) / 255.0f;
                        doorMaterial->Get<float>("SpecularIntensity") = 1.0f;

                        glm::vec3 doorColor = ((glm::vec3(doorPixel) / 255.0f) + 0.2f) * 0.5f;
                        glm::vec3 rustColor = glm::vec3(0.4f, 0.2f, 0.1f);
                        BitmapBaseHandle doorTexture = std::make_shared<Bitmap<glm::u8vec3>>(*rustMap, [=](uint8_t value)
                        {
                            return glm::u8vec3(glm::mix(doorColor, rustColor, value / 255.0f) * 255.0f);
                        });

                        doorMaterial->SetTexture("Texture", CreateTexture(doorTexture));
                        doorMaterial->SetTexture("NormalMap", doorNormalMap);

                        ECS::Entity door = CreateEntity(RenderableMesh(doorMesh, doorMaterial), ClickableComponent(doorMesh));
                        auto& doorTransformation = door.AddComponent<Transformation>(glm::vec3(y * 2.0f - 1.0f, 0, -(x * 2.0f - 1.0f)), doorRotation);
                        doorTransformation.Scale *= 0.995f;

                        float* doorOpenAxis = &doorTransformation.Position.x;
                        if(backOrFront)
                        {
                            doorOpenAxis = &doorTransformation.Position.z;
                        }

                        auto& closeDoorAnimation = door.AddComponent<AnimationComponent<float>>(*doorOpenAxis, 0.0f, false);
                        closeDoorAnimation.AddFrame(4.0f, *doorOpenAxis + 2.0f);

                        AudioSourceHandle doorAudioSource = CreateAudioSource();
                        
                        auto& doorAudioSourceComponent = door.AddComponent<AudioSourceComponent>(doorAudioSource, Attenuation(0, 0, 1));
                        doorAudioSourceComponent.Volume = 2.0f;

                        /*door.SubscribeEvent(selectEntitySystem.OnEntityHover, [this, &mouse, &closeDoorAnimation, &doorPixel, &selectEntitySystem, doorOpenSound, door, doorMap, doorAudioSource]()
                        {
                            if(m_collectedKeys.contains(doorPixel) && mouse.GetButtonState(MouseButton::Left) == ButtonState::Pressed)
                            {
                                closeDoorAnimation.Start();
                                doorAudioSource->Play(doorOpenSound);

                                door.UnsubscribeEvent(selectEntitySystem.OnEntityHover);
                            }
                        });*/

                        closeDoorAnimation.OnFinish += [door, &doorPixel]()
                        {
                            //door.Delete();
                            doorPixel = glm::u8vec3(0, 0, 0);
                        };
                    }

                    AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 1, -1.0f, floorTexCoords);
                    AddQuad(vertices, indices, x, y, glm::vec3(-1.0f), glm::vec3(1.0f), 1,  1.0f, ceilingTexCoords);
				}
			}
		}

        for(size_t i = 0; i < std::min(shadowLocations.size(), size_t(levelBitmap->Width / 4U)); i++)
        {
            auto randomIndex = int(std::randf() * shadowLocations.size());

            glm::uvec2 randomLocation = shadowLocations[randomIndex];
            shadowLocations.erase(shadowLocations.begin() + randomIndex);

            ECS::Entity shadowFigureEntity = CreateEntity();
            auto& shadowFigureTransformation = shadowFigureEntity.AddComponent<Transformation>(glm::vec3(glm::vec3(randomLocation.y * 2.0f - 1.0f, -1.25f, -(randomLocation.x * 2.0f - 1.0f))), glm::quat(1, 0, 0, 0), glm::vec3(1.25f));

            auto& shadowFigureMeshComponent = shadowFigureEntity.AddComponent<RenderableMesh>(shadowFigureMesh, blankMaterial);
            shadowFigureMeshComponent.ShadowOnly = true;

            auto& shadowFigureAnimation = shadowFigureEntity.AddComponent<AnimationComponent<glm::vec3>>(shadowFigureTransformation.Position, 0.0f, false);
            shadowFigureAnimation.AddFrame(3.0f, shadowFigureTransformation.Position);

            (*shadowEntities)[randomLocation] = shadowFigureEntity;

            shadowFigureAnimation.OnFinish += [shadowFigureEntity]()
            {
                shadowFigureEntity.Delete();
            };
        }

		MaterialHandle brickMaterial = CreateMaterial(normalMappedShader);
		brickMaterial->SetTexture("Texture", textureAtlas);
        brickMaterial->SetTexture("NormalMap", normalsAtlas);

        CalculateTangents(vertices, indices);
		MeshHandle mesh = CreateMesh(Model(vertices, indices));

		CreateEntity(Transformation(), RenderableMesh(mesh, brickMaterial));

        ShaderHandle waterShader = LoadShader("deferred/water/water_VS.glsl", "deferred/water/water_FS.glsl");
        waterShader->GetMaterialField("WaveStrength").SetDefaultValue(0.04f);
        waterShader->GetMaterialField("SpecularIntensity").SetDefaultValue(0.0f);

        MaterialHandle waterMaterial = CreateMaterial(waterShader);
        waterMaterial->Get<float>("SpecularIntensity") = 0.7f;
        waterMaterial->Get<glm::vec2>("TilingFactor") = glm::vec2(levelBitmap->Size()) * 10.0f;
        waterMaterial->SetTexture("DistortionMap", LoadTexture<glm::u8vec3>("Water DUDV Map.png"));
        waterMaterial->SetTexture("NormalMap", LoadTexture<glm::u8vec3>("Water Normal Map.png"));
        ECS::Entity waterEntity = CreateEntity(Transformation(glm::vec3(0, -1.0f, 0), glm::quat(1, 0, 0, 0), glm::vec3(levelBitmap->Height * 2.0f, 0, levelBitmap->Width * 2.0f)), WaterComponent(waterMaterial, 0.07f));

        auto& waterTransformation = waterEntity.GetComponent<Transformation>();
        auto& waterAnimation = waterEntity.AddComponent<AnimationComponent<float>>(waterTransformation.Position.y, 0.0f, true);
        waterAnimation.AddFrame(120.0f, -0.45f);
        
        //waterAnimation.Pause();

		Camera player = CreateCamera(Transformation(glm::vec3(1.0f, PLAYER_HEIGHT, -1.0f)), Projection(70.0f, SelectedGraphicsMode().GetAspectRatio(), 0.01f, 10.0f));
        //player.AddComponent(CreateLight<PointLight>(glm::vec3(1.0f, 1.0f, 0.7f), 0.8f, Attenuation(0, 0, 0.4f)));
        //player.AddComponent(CreateLight<SpotLight>(glm::vec3(1.0f, 1.0f, 0.7f), 0.8f, Attenuation(0, 0, 0.3f), 120.0f, 5.0f));

        ECS::Entity torch = CreateEntity(CreateLight<SpotLight>(glm::vec3(1.0f, 1.0f, 0.7f), 0.8f, Attenuation(0, 0, 0.3f), 120.0f, 5.0f));
        auto& torchTransformation = torch.AddComponent<Transformation>();
        torchTransformation.SetParent(player);
        torchTransformation.Position.x += 0.3f;

        AnimationComponent<float>& walkAnimation = player.AddComponent<AnimationComponent<float>>(player.GetTransformation().Position.y, 0.15f, false);
        walkAnimation.AddFrame(0.15f, -0.43f);
        walkAnimation.Loop = true;

        AudioSourceHandle  leftFootFloorAudioSource = CreateAudioSource();
        AudioSourceHandle rightFootFloorAudioSource = CreateAudioSource();

        AudioSourceHandle  leftFootWaterAudioSource = CreateAudioSource();
        AudioSourceHandle rightFootWaterAudioSource = CreateAudioSource();

        leftFootFloorAudioSource->SetPitchSpeed(2.0f);
        rightFootFloorAudioSource->SetPitchSpeed(2.0f);

        AudioSourceHandle mouthAudioSource = CreateAudioSource();

        SoundHandle footstepSound = LoadSound("Footstep.wav");
        SoundHandle   splashSound = LoadSound("splash.wav");

        SoundHandle breathInSound  = LoadSound("Breath In.wav");
        SoundHandle breathOutSound = LoadSound("Breath Out.wav");

        ECS::Entity  leftFootFloor = CreateEntity(Transformation(), AudioSourceComponent(leftFootFloorAudioSource, Attenuation(0, 0, 1.0f)));
        ECS::Entity rightFootFloor = CreateEntity(Transformation(), AudioSourceComponent(rightFootFloorAudioSource, Attenuation(0, 0, 1.0f)));
         leftFootFloor.AddComponent<FollowerComponent>(player, glm::vec3(-0.3f, PLAYER_HEIGHT, 0.0f));
        rightFootFloor.AddComponent<FollowerComponent>(player, glm::vec3( 0.3f, PLAYER_HEIGHT, 0.0f));

        ECS::Entity  leftFootWater = CreateEntity(Transformation());
        ECS::Entity rightFootWater = CreateEntity(Transformation());
         leftFootWater.AddComponent<FollowerComponent>(player, glm::vec3(-0.3f, PLAYER_HEIGHT, 0.0f));
        rightFootWater.AddComponent<FollowerComponent>(player, glm::vec3( 0.3f, PLAYER_HEIGHT, 0.0f));

         leftFootWater.AddComponent<AudioSourceComponent>( leftFootWaterAudioSource, Attenuation(0, 0, 1.0f)).Volume = 0.1f;
        rightFootWater.AddComponent<AudioSourceComponent>(rightFootWaterAudioSource, Attenuation(0, 0, 1.0f)).Volume = 0.1f;

        ECS::Entity mouth = CreateEntity(Transformation());
        mouth.AddComponent<FollowerComponent>(player, glm::vec3(0.0f, PLAYER_HEIGHT, 0.0f));
        AudioSourceComponent& mouthAudioSourceComponent = mouth.AddComponent<AudioSourceComponent>(mouthAudioSource, Attenuation(0, 0, 1));

        std::shared_ptr<bool> passed = std::make_shared<bool>(false);

        std::shared_ptr<bool> right = std::make_shared<bool>(false);
        
        KeyboardMovementComponent& controller = player.AddComponent<KeyboardMovementComponent>();
        controller.SetKeyControl(Key::W, glm::vec3(0, 0, -1), 2.5f);
        controller.SetKeyControl(Key::S, glm::vec3(0, 0,  1), 1.0f);

        walkAnimation.OnReset += [=]()
        {
            *passed = false;
        };

        walkAnimation.OnElapsed += [=, &walkAnimation, &waterAnimation, &controller](float elapsedSeconds)
        {
            float waterProgress = 1.0f - waterAnimation.GetProgress();
            controller.SetKeyControl(Key::W, glm::vec3(0, 0, -1), 2.5f * waterProgress);
            controller.SetKeyControl(Key::S, glm::vec3(0, 0,  1), 1.0f * waterProgress);

            if(elapsedSeconds >= walkAnimation[1].TimeBeforeInSeconds && !*passed)
            {
                *right = !*right;
                if(*right)
                {
                    mouthAudioSource->Play(breathInSound);
                    rightFootFloorAudioSource->Play(footstepSound);
                    rightFootWaterAudioSource->Play(splashSound);
                }
                else
                {
                    mouthAudioSource->Play(breathOutSound);
                    leftFootFloorAudioSource->Play(footstepSound);
                    leftFootWaterAudioSource->Play(splashSound);
                }

                *passed = true;
            }
        };

        MouseLookComponent& mouseLookComponent = player.AddComponent<MouseLookComponent>(0.5f);
        mouseLookComponent.CanPitch = false;

		
        
		//controller.SetKeyControl(Key::A, glm::vec3(-1, 0,  0), 2.5f);
		//controller.SetKeyControl(Key::D, glm::vec3( 1, 0,  0), 2.5f);

        float playerSize = 0.4f;

        AudioSourceHandle shadowFigureAudioSource = CreateAudioSource();

        SoundHandle shadowFigureSound = LoadSound("Shadow Figure.wav");

        player.SubscribeEvent(keyboardMovementSystem.MoveEvent, [&walkAnimation, &waterAnimation, shadowFigureAudioSource, shadowFigureSound, shadowEntities, playerSize, levelBitmap, doorMap, &mouthAudioSourceComponent](Key key, Transformation& transformation, glm::vec3 oldPosition, glm::vec3 newPosition)
        {
            float waterProgress = 1.0f - waterAnimation.GetProgress();

            if(key == Key::S)
            {
                mouthAudioSourceComponent.Volume = 0.0f;
                walkAnimation.SpeedFactor = 0.5f * waterProgress;
            }

            if(key == Key::W)
            {
                mouthAudioSourceComponent.Volume = 1.0f;
                walkAnimation.SpeedFactor = 1.0f * waterProgress;
            }

            std::array<glm::vec3, 4> oldPositions =
            {
                oldPosition + glm::vec3(-playerSize, 0.0f,  playerSize),
                oldPosition + glm::vec3( playerSize, 0.0f,  playerSize),
                oldPosition + glm::vec3(-playerSize, 0.0f, -playerSize),
                oldPosition + glm::vec3( playerSize, 0.0f, -playerSize),
            };

            std::array<glm::vec3, 4> newPositions =
            {
                newPosition + glm::vec3(-playerSize, 0.0f,  playerSize),
                newPosition + glm::vec3( playerSize, 0.0f,  playerSize),
                newPosition + glm::vec3(-playerSize, 0.0f, -playerSize),
                newPosition + glm::vec3( playerSize, 0.0f, -playerSize),
            };

            glm::vec3 movement = newPosition - oldPosition;

            for(size_t i = 0; i < 4; i++)
            {
                glm::uvec2 oldLocation = glm::uvec2(ceil(-oldPositions[i].z * 0.5f), ceil(oldPositions[i].x * 0.5f));
                glm::uvec2 newLocation = glm::uvec2(ceil(-newPositions[i].z * 0.5f), ceil(newPositions[i].x * 0.5f));

                glm::u8vec3 newXPixel = levelBitmap->GetPixel(newLocation.x, oldLocation.y);
                glm::u8vec3 newYPixel = levelBitmap->GetPixel(oldLocation.x, newLocation.y);
                glm::u8vec3 newXYPixel = levelBitmap->GetPixel(newLocation);

                glm::u8vec3 newDoorXPixel = doorMap->GetPixel(newLocation.x, oldLocation.y);
                glm::u8vec3 newDoorYPixel = doorMap->GetPixel(oldLocation.x, newLocation.y);
                glm::u8vec3 newDoorXYPixel = doorMap->GetPixel(newLocation);

                if(newXYPixel == glm::u8vec3(0, 0, 0) || IsDoor(newDoorXYPixel))
                {
                    if(newLocation.x != oldLocation.x && (newXPixel == glm::u8vec3(0, 0, 0) || IsDoor(newDoorXPixel)))
                    {
                        movement.z = 0.0f;
                    }

                    if(newLocation.y != oldLocation.y && (newYPixel == glm::u8vec3(0, 0, 0) || IsDoor(newDoorYPixel)))
                    {
                        movement.x = 0.0f;
                    }
                }
            }

            glm::vec3 frontPlayerPosition = newPosition + glm::normalize(glm::rotate(transformation.Rotation, glm::vec3(0, 0, -1))) * 2.0f;
            glm::vec3 frontRightPlayerPosition = frontPlayerPosition + glm::normalize(glm::rotate(transformation.Rotation, glm::vec3(1, 0, 0))) * float(levelBitmap->Width);

            glm::uvec2 frontPlayerLocation = glm::uvec2(ceil(-frontPlayerPosition.z * 0.5f), ceil(frontPlayerPosition.x * 0.5f));

            transformation.Position = oldPosition + movement;

            auto shadowEntityIt = shadowEntities->find(frontPlayerLocation);
            if(shadowEntityIt != shadowEntities->end())
            {
                ECS::Entity shadowEntity = shadowEntityIt->second;

                auto& shadowAnimationComponent = shadowEntity.GetComponent<AnimationComponent<glm::vec3>>();

                auto& shadowTransformation = shadowEntity.GetComponent<Transformation>();
                shadowTransformation.Rotation = glm::lookAt(shadowTransformation.Position, transformation.Position, glm::vec3(0, 1, 0));

                shadowAnimationComponent[1].Value = frontRightPlayerPosition;

                shadowFigureAudioSource->Play(shadowFigureSound);
                shadowAnimationComponent.Start();

                shadowEntities->erase(shadowEntityIt);
            }
        });
        
        std::shared_ptr<bool> isFocused = std::make_shared<bool>(true);

        mouse.OnButtonDown += [this](MouseButton button)
        {
            if(button == MouseButton::Left)
            {
                EnableSystem<MouseLookSystem>();
                EnableSystem<MousePickSystem>();
            }
        };

        keyboard.OnKeyDown += [this, &walkAnimation, player, isFocused](Key key)
        {
            if(key == Key::Escape)
            {
                DisableSystem<MouseLookSystem>();
                DisableSystem<MousePickSystem>();
            }

            if(key == Key::Space)
            {
                *isFocused = !*isFocused;

                player.GetComponent<MouseLookComponent>().CanPitch = !*isFocused;

                if(*isFocused)
                {
                    EnableSystem<KeyboardMovementSystem>();
                    
                    auto& transformation = player.GetTransformation();
                    glm::vec3 forward = glm::normalize(glm::rotate(transformation.Rotation, glm::vec3(0, 0, -1)));
                    transformation.Rotation = glm::quatLookAt(glm::normalize(forward * glm::vec3(1, 0, 1)), glm::vec3(0, 1, 0));
                }
                else
                {
                    DisableSystem<KeyboardMovementSystem>();
                }
            }

            if(*isFocused && (key == Key::W || key == Key::S))
            {
                walkAnimation.Start();
            }
        };

        keyboard.OnKeyUp += [&](Key key)
        {
            if((key == Key::W && !keyboard.IsKeyDown(Key::S)) || (key == Key::S && !keyboard.IsKeyDown(Key::W)))
            {
                walkAnimation.Stop();
            }
        };

        MusicHandle music = LoadMusic("I'll hide my scars.mp3");
        PlayMusic(music, true);

		return player;
	}

	void OnUpdate(float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
	}

	void OnRender(RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) override
	{
        
	}
};

GameSettings StartSettings()
{
	SceneHandle startScene = std::make_shared<MainScene>();
	return GameSettings(ScreenGraphicsMode(1920, 1080), "Nightmare Maze!", startScene, 5000);
}