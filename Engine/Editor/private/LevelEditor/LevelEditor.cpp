#include <LevelEditor/LevelEditor.h>
#include <LevelEditor/LevelSerializer.h>
#include <LevelEditor/ECS/ecsLevelEditor.h>

#include <Filesystem.h>
#include <ECS/ecsSystems.h>
#include <Parser/WorldParser.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <format>

namespace GameEngine
{
	namespace Editor
	{
		LevelEditor::LevelEditor(flecs::world& world)
		{
			m_Level = LevelSerializer::Deserialize(Core::g_FileSystem->GetFilePath("Levels/Main.xml").generic_string());

			m_Level->GetLevelObjects().reserve(128);

			for (World::LevelObject& levelObject : m_Level->GetLevelObjects())
			{
				flecs::entity entity = world.entity(levelObject.GetName().c_str());

				World::LevelObject::ComponentList& componentList = levelObject.GetComponents();

				World::LevelObject::ComponentList::iterator positionAttribute = std::ranges::find_if(componentList,
					[](World::LevelObject::Component& component)
					{
						return !std::strcmp(component.first.c_str(), "Position");
					}
				);

				World::LevelObject::ComponentList::iterator geometryAttribute = std::ranges::find_if(componentList,
					[](World::LevelObject::Component& component)
					{
						return !std::strcmp(component.first.c_str(), "GeometryPtr");
					}
				);

				if (positionAttribute != componentList.end() &&
					geometryAttribute != componentList.end())
				{
					assert(World::WorldParser::GetCustomComponents().contains(geometryAttribute->second));

					entity.set(EntitySystem::LevelEditorECS::PositionDesc{ &positionAttribute->second });

					// Can be set to 0 since it doesn't matter now, will be updated by the system
					entity.set(EntitySystem::EditorECS::Position{ 0.0f, 0.0f, 0.0f });
					entity.set(GeometryPtr{
						reinterpret_cast<RenderCore::Geometry*>(
							World::WorldParser::GetCustomComponents()[geometryAttribute->second]
							)
						});
				}
			}

			EntitySystem::LevelEditorECS::RegisterLevelEditorEcsSystems(world);

			flecsWorld = &world;
		}

		void LevelEditor::Draw()
		{
			ImGui::Begin(GetName());

			if (m_Level.has_value()) [[likely]]
			{
				for (World::LevelObject& levelObject : m_Level->GetLevelObjects())
				{
					if (ImGui::TreeNode(levelObject.GetName().c_str()))
					{
						for (World::LevelObject::Component& component : levelObject.GetComponents())
						{
							if (component.first == "Position") {
								float pos[3] = {};
								const char* compValue = component.second.c_str();
								char* end;
								float f = std::strtof(compValue, &end);
								pos[0] = f;
								compValue = end + 1;
								f = std::strtof(compValue, &end);
								pos[1] = f;
								compValue = end + 1;
								f = std::strtof(compValue, &end);
								pos[2] = f;

								if (ImGui::InputFloat3("Position", pos)) {
									component.second = std::format("{},{},{}", pos[0], pos[1], pos[2]);
								}
							}
							else {
								ImGui::InputText(component.first.c_str(), &component.second);
							}
						}

						ImGui::TreePop();
					}
				}
			}

			if (ImGui::Button("Save"))
			{
				m_SaveButtonMessageTimer.Reset();
				m_SaveButtonPressed = true;

				Save();
			}

			if (ImGui::Button("New Object"))
			{
				SpawnDefault();
			}

			if (m_SaveButtonPressed)
			{
				ImGui::SameLine();
				ImGui::Text("Saved!");
			}

			ImGui::End();
		}

		void LevelEditor::Update(float dt)
		{
			m_SaveButtonMessageTimer.Tick();

			if (m_SaveButtonMessageTimer.GetTotalTime() > m_TimeToShowSaveButtonMessage)
			{
				m_SaveButtonPressed = false;
			}
		}

		void LevelEditor::Save()
		{
			assert(m_Level.has_value());
			LevelSerializer::Serialize(Core::g_FileSystem->GetFilePath("Levels/Main.xml").generic_string(), m_Level.value());
		}

		void LevelEditor::SpawnDefault() {
			assert(m_Level.has_value());

			static std::size_t objects = m_Level->GetLevelObjects().size();

			World::LevelObject newObject;
			std::string newObjectName = std::format("newObject{}", objects);
			newObject.SetName(newObjectName.c_str());

			newObject.AddComponent("Position", "0.0,0.0,0.0");
			newObject.AddComponent("Velocity", "0.0,0.0,0.0");
			newObject.AddComponent("Gravity", "0.0,-9.8,0.0");
			newObject.AddComponent("BouncePlane", "0.0,1.0,0.0,5.0");
			newObject.AddComponent("Bounciness", "1.0");
			newObject.AddComponent("GeometryPtr", "Cube");

			m_Level->AddLevelObject(newObject);

			World::Level::LevelObjectList& objectList = m_Level->GetLevelObjects();

			World::LevelObject& objectIter = objectList.back();

			World::LevelObject::ComponentList& list = objectIter.GetComponents();

			World::LevelObject::ComponentList::iterator positionAttribute = std::ranges::find_if(list,
				[](World::LevelObject::Component& component)
				{
					return !std::strcmp(component.first.c_str(), "Position");
				}
			);

			assert(positionAttribute != list.end());

			flecs::entity entity = flecsWorld->entity(objectIter.GetName().c_str())
				.set(EntitySystem::LevelEditorECS::PositionDesc{ &positionAttribute->second })
				.set(EntitySystem::EditorECS::Position{ 0.0f, 0.0f, 0.0f })
				.set(GeometryPtr{
						reinterpret_cast<RenderCore::Geometry*>(
							World::WorldParser::GetCustomComponents()["Cube"]
							)
					});

			objects++;
		}
	}
}