#include <sstream>
#include <iomanip>
#include <deque>
#include <set>
#include <experimental/filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glmext.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include <fts_fuzzy_match.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

#include <Log.hpp>

#include <Entity.hpp>

#include <Meta.hpp>
#include <ComponentValidation.hpp>

static const ImVec4 LogColors[3] = {
	ImVec4{1, 1, 1, 1},
	ImVec4{1, 1, 0, 1},
	ImVec4{1, 0, 0, 1}
};
	
template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

std::experimental::filesystem::path explore(const std::experimental::filesystem::path& path, const std::vector<const char*>& extensions = {})
{
	namespace fs = std::experimental::filesystem;
	fs::path return_path;
	if(!fs::is_directory(path))
		return return_path;
	for(auto& p: fs::directory_iterator(path))
	{
		if(fs::is_directory(p))
		{
			if(ImGui::TreeNode(p.path().filename().string().c_str()))
			{
				auto tmp_r = explore(p.path(), extensions);
				if(!tmp_r.empty())
					return_path = tmp_r;
				ImGui::TreePop();
			}
		} else {
			auto ext = p.path().extension().string();
			if(extensions.empty() || std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
				if(ImGui::SmallButton(p.path().filename().string().c_str()))
					return_path = p.path();
		}
	}
	return return_path;
}

bool loadModel(const std::string& path)
{
	EntityID base_entity = create_entity(path).get_id();
	ComponentID base_transform = get_id(get_entity(base_entity).add<Transformation>());

	auto m = Mesh::load(path);
	
	for(auto& part : m)
	{
		auto t = part->resetPivot();
		part->createVAO();
		part->getMaterial().setUniform("R", 0.95f);
		part->getMaterial().setUniform("F0", 0.15f);
		
		if(m.size() == 1)
		{
			get_entity(base_entity).set_name(part->getName());
			get_entity(base_entity).add<MeshRenderer>(*part);
		} else {
			auto& entity = create_entity(part->getName());
			auto& ent_transform = entity.add<Transformation>(t);
			get_component<Transformation>(base_transform).addChild(ent_transform);
			entity.add<MeshRenderer>(*part);
		}
	}
	
	return true;
}

bool loadScene(const std::string& path)
{
	Clock scene_loading_clock;
	auto scene_loading_start = scene_loading_clock.now();
	std::ifstream f(path);
	if(!f)
	{
		Log::error("Error opening scene '", path, "'.");
		return false;
	}
	
	clear_entities();
	for_each<deletion_pass_wrapper, ComponentTypes>{}();
	Resources::clearMeshes();
	
	nlohmann::json j;
	f >> j;
	
	for(auto& e : j["models"])
	{
		auto m = Mesh::load(e);
		for(auto& part : m)
		{
			part->resetPivot();
			part->createVAO();
		}
	}
	
	// TODO: Handle transformation hierarchy!
	
	std::vector<std::tuple<ComponentID, ComponentID>> transform_relations;
	for(auto& e : j["entities"])
	{
		auto& base_entity = create_entity(e["Name"].is_string() ? e["Name"] : "UnamedEntity");
		
		auto transform = e.find("Transformation");
		if(transform != e.end())
		{
			ComponentID base_transform = get_id(base_entity.add<Transformation>(*transform));
			if((*transform).find("parent") != (*transform).end() && (*transform)["parent"] != invalid_component_idx)
				transform_relations.push_back({(*transform)["parent"], base_transform});
		
			auto meshrenderer = e.find("MeshRenderer");
			if(meshrenderer != e.end())
				base_entity.add<MeshRenderer>(*meshrenderer);
				
			auto spotlight = e.find("SpotLight");
			if(spotlight != e.end())
				base_entity.add<SpotLight>(*spotlight);
			
			auto collisionbox = e.find("CollisionBox");
			if(collisionbox != e.end())
				base_entity.add<CollisionBox>(*collisionbox);
		}
	}
	
	for(const auto& r : transform_relations)
		get_component<Transformation>(std::get<0>(r)).addChild(std::get<1>(r));
	
	auto scene_loading_end = scene_loading_clock.now();
	Log::info("Scene loading done in ", std::chrono::duration_cast<std::chrono::milliseconds>(scene_loading_end - scene_loading_start).count(), "ms.");
	
	return true;
}

bool saveScene(const std::string& path)
{
	Log::info("Saving scene to '", path, "'...");
	
	std::ofstream f(path);
	if(!f)
	{
		Log::error("Error opening '", path, "'.");
		return false;
	}
	
	nlohmann::json j;
	
	std::set<std::string> temp_mesh_paths;
	for(const auto& m : Resources::_meshes)
		temp_mesh_paths.insert(m.second->getPath());
	for(const auto& p : temp_mesh_paths)
		j["models"].push_back(p);
	
	for(const auto& e : entities)
	{
		if(e.is_valid())
		{
			nlohmann::json je;
			je["Name"] = e.get_name();
			if(e.has<Transformation>())
				je["Transformation"] = e.get<Transformation>().json();
			if(e.has<MeshRenderer>())
				je["MeshRenderer"] = e.get<MeshRenderer>().json();
			if(e.has<SpotLight>())
				je["SpotLight"] = e.get<SpotLight>().json();
			// TODO: Other Components (...)
			j["entities"].push_back(je);
		}
	}
	
	f << std::setw(4) << j;
	
	Log::info("Scene successfully saved to '", path, "'.");
	
	return true;
}

template<typename Func, typename... Args>
void file_explorer(const char* name, Func&& lambda, Args&&... exts)
{
	if(ImGui::BeginPopup(name))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0, 0.0, 0.0, 0.0});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3647, 0.3607, 0.7176, 1.0});
		namespace fs = std::experimental::filesystem;
		static char root_path[256] = ".";
		ImGui::InputText("Root", root_path, 256);
		auto p = explore(root_path, {exts...});
		if(!p.empty())
		{
			lambda(p.string());
			// @todo Not so sure about that...
			ImGui::GetIO().WantCaptureKeyboard = ImGui::GetIO().WantTextInput = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(2);
		if(ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

class Editor : public DeferredRenderer
{
public:		
	Editor(int argc, char* argv[]) : 
		DeferredRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		DeferredRenderer::run_init();
		
		auto& Simple = Resources::loadProgram("Simple",
			Resources::load<VertexShader>("src/GLSL/Deferred/deferred_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/fs.glsl")
		);
		
		_camera.speed = 15;
		Simple.bindUniformBlock("Camera", _camera.getGPUBuffer()); 
		
		loadScene(_scenePath);
		
		_volumeSamples = 16;
		
		/// TODO REMOVE?
		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});

		_shortcuts[{GLFW_KEY_S, GLFW_PRESS, GLFW_MOD_CONTROL}] = [&]()
		{
			saveScene(_scenePath);
		};
		_shortcuts[{GLFW_KEY_DELETE}] = [&]()
		{
			if(_selectedObject)
			{
				auto selectedEntityPtr = &get_entity(_selectedObject);
				destroy_entity(selectedEntityPtr->get_id());
				deselectObject();
			}
		};
		
		for(auto& it : ComponentIterator<SpotLight>{})
		{
			it.updateMatrices();
			it.drawShadowMap(ComponentIterator<MeshRenderer>{});
		}
	}
	
	virtual void renderGUI() override
	{
		bool load_scene = false;
		bool load_model = false;
		
		auto winpos = ImGui::GetMainViewport()->Pos;
		glm::vec2 glmwinpos{winpos.x, winpos.y};
		
		// Menu
		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("File"))
			{
				load_scene = ImGui::MenuItem("Load Scene");
				if(ImGui::MenuItem("Reload Scene"))
					loadScene(_scenePath);
				if(ImGui::MenuItem("Save Scene", "Ctrl+S"))
					saveScene(_scenePath);
				ImGui::Separator();
				load_model = ImGui::MenuItem("Load Model");
				ImGui::Separator();
				if(ImGui::MenuItem("Exit", "Alt+F4"))
					glfwSetWindowShouldClose(_window, GL_TRUE);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Windows"))
			{
				if(ImGui::MenuItem("Rendering Options")) win_rendering = !win_rendering;
				if(ImGui::MenuItem("Scene")) win_scene = !win_scene;
				if(ImGui::MenuItem("Entity Inspector")) win_inspect = !win_inspect;
				if(ImGui::MenuItem("Logs")) win_logs = !win_logs;
				if(ImGui::MenuItem("Statistics")) win_stats = !win_stats;
				if(ImGui::MenuItem("ImGui Statistics")) win_imgui_stats = !win_imgui_stats;
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Debug Options"))
			{
				ImGui::Checkbox("Pause", &_paused);
				ImGui::SliderFloat("Time Scale", &_timescale, 0.0f, 5.0f);
				if(ImGui::Button("Update shadow maps"))
				{
					for(auto& it : ComponentIterator<SpotLight>{})
					{
						it.updateMatrices();
						it.drawShadowMap(ComponentIterator<MeshRenderer>{});
					}
				}
				ImGui::Separator(); 
				ImGui::Checkbox("Toggle Debug", &_debug_buffers);
				const char* debugbuffer_items[] = {"Color","Position", "Normal"};
				const Attachment debugbuffer_values[] = {Attachment::Color0, Attachment::Color1, Attachment::Color2};
				static int debugbuffer_item_current = 0;
				if(ImGui::Combo("Buffer to Display", &debugbuffer_item_current, debugbuffer_items, 3))
					_framebufferToBlit = debugbuffer_values[debugbuffer_item_current];
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
			
		// Draw status bar
		if(_status_bar)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0, 2.5));
			ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.0f), ImGuiCond_Always);
			ImGui::SetNextWindowPos(ImVec2(winpos.x, winpos.y + ImGui::GetIO().DisplaySize.y - 20.f), ImGuiCond_Always);
			ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
			ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
			ImGui::SameLine(125, 0);
			ImGui::TextColored(LogColors[Log::_logs.front().type], "%s", Log::_logs.front().str().c_str());
			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		if(load_scene) ImGui::OpenPopup("Load Scene");
		file_explorer("Load Scene", loadScene, ".json");
			
		if(load_model) ImGui::OpenPopup("Load Model");
		file_explorer("Load Model", loadModel, ".obj");
	
		if(win_imgui_stats)
			ImGui::ShowMetricsWindow();
		
		update_stats();	
		if(win_stats) gui_stats();
		if(win_rendering) gui_rendering();
		if(win_scene) gui_scene();
		if(win_inspect) gui_inspect();
		if(win_logs) gui_logs();
		
		// On screen Gizmos (Position/Rotation)
		if(_selectedObject != invalid_entity)
		{
			auto selectedEntityPtr = &get_entity(_selectedObject);
			if(selectedEntityPtr->has<Transformation>())
			{
				auto& transform = selectedEntityPtr->get<Transformation>();
				
				// Dummy Window for "on field" widgets
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2{static_cast<float>(_width), static_cast<float>(_height)});
				ImGui::SetNextWindowBgAlpha(0.0);
				ImGui::Begin("SelectedObject", nullptr,
					ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
				
				ImGuiContext* g = ImGui::GetCurrentContext();
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				const auto abs_mouse = glm::vec2{ImGui::GetMousePos()};
				const auto rel_mouse = abs_mouse - glmwinpos;
				ImDrawList* drawlist = ImGui::GetWindowDrawList();
				
				// @Todo : Abstract this?
				if(selectedEntityPtr->has<MeshRenderer>())
				{
					auto& mr = selectedEntityPtr->get<MeshRenderer>();
					Context::disable(Capability::DepthTest);
					Context::enable(Capability::Blend);
					Context::blendFunc(Factor::SrcAlpha, Factor::OneMinusSrcAlpha);
					Program& blend = Resources::getProgram("Simple");
					blend.use();
					blend.setUniform("Color", _selectedObjectColor);
					blend.setUniform("ModelMatrix", transform.getGlobalMatrix());
					mr.getMesh().draw();
					
					blend.setUniform("Color", glm::vec4(1.0, 1.0, 1.0, 0.1));
					mr.draw_aabb();
					
					Program::useNone();
					Context::disable(Capability::Blend);
					Context::enable(Capability::DepthTest);
					
					auto aabb = mr.getAABB().getBounds();
					std::array<ImVec2, 8> screen_aabb;
					for(int i = 0; i < 8; ++i)
					{
						screen_aabb[i] = project(aabb[i]) + glmwinpos;
					}
					// Bounding Box Gizmo
					constexpr std::array<size_t, 24> segments{
						0, 1, 1, 3, 3, 2, 2, 0,
						4, 5, 5, 7, 7, 6, 6, 4,
						0, 4, 1, 5, 2, 6, 3, 7
					};
					for(int i = 0; i < 24; i += 2)
						drawlist->AddLine(screen_aabb[segments[i]], screen_aabb[segments[i + 1]], 
							ImGui::ColorConvertFloat4ToU32(ImVec4(0.0, 0.0, 1.0, 0.5)));
				}
				
				/////////////////////////////////////
				// Position Gizmo
				// @todo Debug it, Clean in, Package it 
				
				auto gp = transform.getGlobalPosition();
				auto gr = transform.getGlobalRotation();
				auto gs = transform.getGlobalScale();
		
				const std::array<glm::vec2, 4> gizmo_points{
					project(gp + glm::vec3{0.0, 0.0, 0.0}),
					project(gp + glm::vec3{1.0, 0.0, 0.0}),
					project(gp + glm::vec3{0.0, 1.0, 0.0}),
					project(gp + glm::vec3{0.0, 0.0, 1.0})
				};
				
				static const char* labels[3] = {"PositionGizmoX", "PositionGizmoY", "PositionGizmoZ"};
				for(int i = 0; i < 3; ++i)
				{
					const ImGuiID id = window->GetID(labels[i]);
					bool hovered = point_line_distance(rel_mouse, gizmo_points[0], gizmo_points[1 + i]) < 5.0f;
					if(hovered)
						ImGui::SetHoveredID(id);
					bool active = id == g->ActiveId;
				
					if(active && ImGui::IsMouseDragging(0))
					{
						const auto newP = rel_mouse;
						const auto oldP = newP - glm::vec2{ImGui::GetMouseDragDelta()};
						ImGui::ResetMouseDragDelta(0);
						const auto newR = getScreenRay(newP.x, newP.y);
						const auto oldR = getScreenRay(oldP.x, oldP.y);
						Plane pl{gp, -_camera.getDirection()}; // @todo Project onto something else (line)
						float d0 = std::numeric_limits<float>::max(), d1 = std::numeric_limits<float>::max();
						glm::vec3 p0, p1, n0, n1;
						trace(newR, pl, d0, p0, n0);
						trace(oldR, pl, d1, p1, n1);
						auto newPosition = transform.getPosition();
						newPosition[i] += (p0[i] - p1[i]) / (gs[i] / transform.getScale()[i]);
						transform.setPosition(newPosition);
						
						//ImGui::SetActiveID(id, window);
						ImGui::GetIO().WantCaptureMouse = true;
					} 
					if(active && ImGui::IsMouseReleased(0))
					{
						ImGui::SetActiveID(0, window);
					}
					if(ImGui::IsMouseClicked(0) && hovered)
					{
						ImGui::SetActiveID(id, window);
						ImGui::GetIO().WantCaptureMouse = true;
					}
					drawlist->AddLine(glmwinpos + gizmo_points[0], glmwinpos + gizmo_points[i + 1], 
						ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)), 2.0);
				}
				////////////////////////////////////////////////////
				
				// Rotation Gizmo
				// @todo FIX IT
				const std::array<glm::vec2, 4> rot_gizmo_points{
					project(gp + glm::vec3{gr * glm::vec4{0.0, 0.0, 0.0, 1.0f}}),
					project(gp + glm::vec3{gr * glm::vec4{1.0, 0.0, 0.0, 1.0f}}),
					project(gp + glm::vec3{gr * glm::vec4{0.0, 1.0, 0.0, 1.0f}}),
					project(gp + glm::vec3{gr * glm::vec4{0.0, 0.0, 1.0, 1.0f}})
				};
				
				static constexpr const char* rot_labels[3] = {"RotationGizmoX", "RotationGizmoY", "RotationGizmoZ"};
				const std::array<glm::vec3, 3> plane_normals = {
					glm::vec3{gr * glm::vec4{0.0, 1.0, 0.0, 1.0}},
					glm::vec3{gr * glm::vec4{0.0, 0.0, 1.0, 1.0}},
					glm::vec3{gr * glm::vec4{1.0, 0.0, 0.0, 1.0}},
				};
				for(int i = 0; i < 3; ++i)
				{
					const ImGuiID id = window->GetID(rot_labels[i]);
					bool hovered = glm::distance(rel_mouse, rot_gizmo_points[1 + i]) < 10.0f;
					if(hovered)
						ImGui::SetHoveredID(id);
					bool active = id == g->ActiveId;
					
					if(active && ImGui::IsMouseDragging(0))
					{
						const auto newP = rel_mouse;
						const auto oldP = newP - glm::vec2{ImGui::GetMouseDragDelta()};
						ImGui::ResetMouseDragDelta(0);
						const auto newR = getScreenRay(newP.x, newP.y);
						const auto oldR = getScreenRay(oldP.x, oldP.y);
						Plane pl{gp, 
							(glm::dot(plane_normals[i], _camera.getDirection()) < 0.0 ? 1.0f : -1.0f) * plane_normals[i]};
						float d0 = std::numeric_limits<float>::max(), d1 = std::numeric_limits<float>::max();
						glm::vec3 p0, p1, n0, n1;
						trace(newR, pl, d0, p0, n0);
						trace(oldR, pl, d1, p1, n1);
						
						auto v1 = p1 - gp;
						auto v2 = p0 - gp;
						v1 = glm::vec3{glm::inverse(transform.getRotation()) * glm::vec4{v1, 1.0}};
						v2 = glm::vec3{glm::inverse(transform.getRotation()) * glm::vec4{v2, 1.0}};
						auto a = glm::cross(v1, v2);
						
						glm::quat q(a);
						q.w = glm::sqrt((glm::length2(v1) * glm::length2(v2)) + glm::dot(v1, v2));
						q = glm::normalize(q);
						
						transform.setRotation(transform.getRotation() * q);
						
						//ImGui::SetActiveID(id, window);
						ImGui::GetIO().WantCaptureMouse = true;
					} 
					if(active && ImGui::IsMouseReleased(0))
					{
						ImGui::SetActiveID(0, window);
					}
					if(ImGui::IsMouseClicked(0) && hovered)
					{
						ImGui::SetActiveID(id, window);
						ImGui::GetIO().WantCaptureMouse = true;
					}
					
					const size_t circle_precision = 21;
					glm::vec3 circle[circle_precision];
					auto g = glm::cross(plane_normals[i], plane_normals[(i+2)%3]);
					for(size_t p = 0; p < circle_precision; ++p)
						circle[p] = glm::vec3{glm::vec4{
								glm::normalize(
									plane_normals[(i+2)%3] + 
									(static_cast<float>(p) / circle_precision - 0.5f) * g
								)
							, 1.0}};
					for(size_t p = 0; p < circle_precision - 1; ++p)
					{
						drawlist->AddLine(glmwinpos + project(gp + circle[p]), 
							glmwinpos + project(gp + circle[p + 1]),
							ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)));
					}

					drawlist->AddCircle(glmwinpos + rot_gizmo_points[i + 1], 10.0,
						ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)));
				}
				
				ImGui::End();
			}
		}
		
		// Entity Selection
		if(!ImGui::GetIO().WantCaptureMouse)
		{
			if(ImGui::IsMouseClicked(0))
			{
				const auto r = getMouseRay();
				float depth = std::numeric_limits<float>::max();
				deselectObject();
				for(auto& o : ComponentIterator<MeshRenderer>{})
				{
					if(trace(r, o, depth))
						selectObject(get_owner<MeshRenderer>(o));
				}
			}
		}
		
		DeferredRenderer::renderGUI();
	}
		
protected:
	EntityID		_selectedObject = invalid_entity;
	glm::vec4		_selectedObjectColor = glm::vec4{0.0, 0.0, 1.0, 0.10};
	
	std::string		_scenePath = "in/Scenes/test_scene.json";

	void deselectObject()
	{
		_selectedObject = invalid_entity;
	}
	
	void selectObject(EntityID o)
	{
		if(_selectedObject != invalid_entity)
			deselectObject();
		_selectedObject = o;
	}
	
	void gui_display(const Texture& t)
	{
		ImGui::Image(reinterpret_cast<void*>(t.getName()), ImVec2{256, 256});
	}
	
	void gui_display(const Texture2D& t)
	{
		ImGui::Text("Size: %d * %d", t.getSize().x, t.getSize().y);
		ImGui::Image(reinterpret_cast<void*>(t.getName()), ImVec2{256, 256});
	}

	void gui_display(const Mesh& m)
	{
		ImGui::Text("Name: %s", m.getName().c_str());
		ImGui::Text("Path: %s", m.getPath().c_str());
		
		gui_render_mesh(m);
		// if(ImGui::TreeNode("Base Material"))
		// {
			// edit_material(mr.getMesh().getMaterial());
			// ImGui::TreePop();
		// }
	}
	
	void gui_render_mesh(const Mesh& m)
	{
		// renders the mesh into a small framebuffer (unique, so avoid calling this twice in a frame :))
		static Framebuffer<> model_render{256};
		if(!model_render)
			model_render.init();
		static auto& GUIModelRender = Resources::loadProgram("GUIModelRender",
			Resources::load<VertexShader>("src/GLSL/Forward/simple_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Forward/simple_fs.glsl")
		);
		static float gui_model_render_fov = 0.7f;
		auto aabb = m.getBoundingBox();
		//ImGui::SliderFloat("Render FoV", &gui_model_render_fov, 0.5, 1.5);
		float gui_model_render_camx = std::max(
			0.5 * (glm::distance(aabb.max, aabb.min)) / std::tan(_resolution.y / _resolution.x * gui_model_render_fov),
			0.5 * (glm::distance(aabb.max, aabb.min)) / std::tan(gui_model_render_fov)
		) * 1.25f;
		GUIModelRender.setUniform("ProjectionMatrix", 
			glm::perspective(gui_model_render_fov, 1.0f, 1.0f, 
				gui_model_render_camx + glm::distance(aabb.max, aabb.min))
		);
		GUIModelRender.setUniform("ViewMatrix", glm::lookAt(
		   glm::vec3(gui_model_render_camx, 0, 0) + 0.5f * (aabb.max + aabb.min),
		   0.5f * (aabb.max + aabb.min),
		   glm::vec3(0, -1, 0)
		));
		GUIModelRender.setUniform("ModelMatrix", glm::rotate(glm::mat4(1.0f), _time, glm::vec3{0, 1, 0}));
		Material mat = m.getMaterial(); // Copy Material... not perfect but will do for now.
		mat.setShadingProgram(GUIModelRender);
		
		model_render.bind();
		mat.use();
		Context::enable(Capability::DepthTest);
		glClearColor(0, 0, 0, 0);
		Context::clear();
		m.draw();
		mat.useNone();
		model_render.unbind();
		Context::viewport(0, 0, _width, _height);
		
		ImGui::Image(reinterpret_cast<void*>(model_render.getColor().getName()), ImVec2{256, 256});
	}
	
	bool win_imgui_stats = false,
		 win_stats = false,
		 win_rendering = false,
		 win_scene = true,
		 win_logs = false,
		 win_inspect = true;
	bool _status_bar = true;
	float last_update = 2.0;
	std::deque<float> frametimes, updatetimes, occlusiontimes, gbuffertimes, lighttimes, postprocesstimes, guitimes;
	const size_t max_samples = 100;
	
	void update_stats()
	{
		last_update += TimeManager::getRealDeltaTime();
		const float ms = TimeManager::getRealDeltaTime() * 1000;
		if(last_update > 0.05 || frametimes.empty())
		{
			if(frametimes.size() > max_samples)			frametimes.pop_front();
			if(updatetimes.size() > max_samples)		updatetimes.pop_front();
			if(occlusiontimes.size() > max_samples)		occlusiontimes.pop_front();
			if(gbuffertimes.size() > max_samples)		gbuffertimes.pop_front();
			if(lighttimes.size() > max_samples)			lighttimes.pop_front();
			if(postprocesstimes.size() > max_samples)	postprocesstimes.pop_front();
			if(guitimes.size() > max_samples)			guitimes.pop_front();
			frametimes.push_back(ms);
			updatetimes.push_back(_updateTiming.get<GLuint64>() / 1000000.0);
			occlusiontimes.push_back(_OcclusionCullingTiming.get<GLuint64>() / 1000000.0);
			gbuffertimes.push_back(_GBufferPassTiming.get<GLuint64>() / 1000000.0);
			lighttimes.push_back(_lightPassTiming.get<GLuint64>() / 1000000.0);
			postprocesstimes.push_back(_postProcessTiming.get<GLuint64>() / 1000000.0);
			guitimes.push_back(_lastGUITiming / 1000000.0);
			last_update = 0.0;
		}
	}
	
	void gui_stats()
	{
		if(ImGui::Begin("Statistics", &win_stats))
		//if(ImGui::BeginDock("Statistics", &win_stats))
		{
			ImGui::Text("%.4f ms/frame (%.1f FPS)", 
				frametimes.back(), 
				1000.0 / frametimes.back()
			);
			auto lamba_data = [](void* data, int idx) {
				return static_cast<std::deque<float>*>(data)->at(idx);
			};
			ImGui::PlotLines("FrameTime", lamba_data, &frametimes, frametimes.size(), 0, to_string(frametimes.back(), 4).c_str(), 0.0, 20.0); 
			ImGui::PlotLines("Update", lamba_data, &updatetimes, updatetimes.size(), 0, to_string(updatetimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Occlusion Culling", lamba_data, &occlusiontimes, occlusiontimes.size(), 0, to_string(occlusiontimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GBuffer", lamba_data, &gbuffertimes, gbuffertimes.size(), 0, to_string(gbuffertimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Lights", lamba_data, &lighttimes, lighttimes.size(), 0, to_string(lighttimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Post Process",lamba_data, &postprocesstimes, postprocesstimes.size(), 0, to_string(postprocesstimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GUI", lamba_data, &guitimes, guitimes.size(), 0, to_string(guitimes.back(), 4).c_str(), 0.0, 10.0);

			ImGui::Text("Scene DrawCalls: %d", _scene_draw_calls);
		}
		//ImGui::EndDock();
		ImGui::End();
	}
	
	void gui_rendering()
	{
		ImGui::Begin("Rendering Options", &win_rendering);
		//ImGui::BeginDock("Rendering Options", &win_rendering);
		{
			if(ImGui::Checkbox("Fullscreen", &_fullscreen))
				setFullscreen(_fullscreen);
			ImGui::SameLine();
			if(ImGui::Checkbox("Vsync", &_vsync))
				glfwSwapInterval(_vsync);
			ImGui::Checkbox("Frustum Culling", &_scene.UseFrustumCulling);
			ImGui::SameLine();
			ImGui::Checkbox("Occlusion Culling", &_scene.UseOcclusionCulling);
			ImGui::Text("Window resolution: %d * %d", _width, _height);
			const char* internal_resolution_items[] = {"Windows resolution", "1920 * 1080", "2715 * 1527", "3840 * 2160"};
			static int internal_resolution_item_current = 0;
			if(ImGui::Combo("Internal Resolution", &internal_resolution_item_current, internal_resolution_items, 4))
			{
				switch(internal_resolution_item_current)
				{
					case 0: setInternalResolution(0, 0); break;
					case 1: setInternalResolution(1920, 1080); break;
					case 2: setInternalResolution(2715, 1527); break;
					case 3: setInternalResolution(3840, 2160); break;
				}
			}
			float fov = _camera.getFoV();
			if(ImGui::DragFloat("FoV", &fov, 1.0, 40.0, 110.0))
				_camera.setFoV(fov);
			
			ImGui::Separator();
			
			ImGui::Checkbox("FXAA", &_fxaa);
			ImGui::Checkbox("FXAA Show Edges", &_fxaa_showEdges);
			ImGui::DragFloat("FXAA LumaThreshold", &_fxaa_lumaThreshold, 0.05, 0.0, 2.0);
			ImGui::DragFloat("FXAA MulReduce", &_fxaa_mulReduce, 1.0, 0.0, 1024.0);
			ImGui::DragFloat("FXAA MinReduce", &_fxaa_minReduce, 1.0, 0.0, 1024.0);
			
			ImGui::Separator();
			
			ImGui::Checkbox("Bloom", &_bloom);
			ImGui::DragFloat("Bloom Strength", &_bloom_strength, 0.05, 0.0, 5.0);
			
			ImGui::Separator();
			
			ImGui::DragFloat("Exposure", &_exposure, 0.05, 0.0, 5.0);
			
			ImGui::Separator();
			
			ImGui::DragFloat("MinVariance (VSM)", &_minVariance, 0.000001, 0.0, 0.00005);
			
			ImGui::Separator();
			
			ImGui::DragInt("AOSamples", &_aoSamples, 1, 0, 32);
			ImGui::DragFloat("AOThresold", &_aoThreshold, 0.05, 0.0, 5.0);
			ImGui::DragFloat("AORadius", &_aoRadius, 1.0, 0.0, 400.0);
			
			ImGui::Separator();
			
			ImGui::DragInt("VolumeSamples", &_volumeSamples, 1, 0, 64);
			ImGui::DragFloat("AtmosphericDensity", &_atmosphericDensity, 0.001, 0.0, 0.02);
		
			ImGui::Separator();

			ImGui::ColorEdit3("Ambiant Color", &_ambiant.r);
		}
		//ImGui::EndDock();
		ImGui::End();
	}
	
	void gui_inspect()
	{
		if(ImGui::Begin("Entity Inspector", &win_inspect))
		//if(ImGui::BeginDock("Entity Inspector", &win_inspect))
		{
			ImGui::ColorEdit4("Highlight color", &_selectedObjectColor.x);
			ImGui::Separator();
			if(_selectedObject != invalid_entity)
			{
				auto selectedEntityPtr = &get_entity(_selectedObject);
				char name_buffer[256];
				std::strcpy(name_buffer, selectedEntityPtr->get_name().c_str());
				if(ImGui::InputText("Name", name_buffer, 256))
					selectedEntityPtr->set_name(name_buffer);
				if(selectedEntityPtr->has<Transformation>())
				{
					if(ImGui::TreeNodeEx("Transformation", ImGuiTreeNodeFlags_DefaultOpen))
					{
						auto& transform = selectedEntityPtr->get<Transformation>();
						
						glm::vec3 p = transform.getPosition();
						if(ImGui::InputFloat3("Position", &p.x))
							transform.setPosition(p);
						ImGui::SameLine();
						if(ImGui::Button("Reset##Position"))
							transform.setPosition(glm::vec3{0.0f});
						
						glm::quat r = transform.getRotation();
						if(ImGui::InputFloat4("Rotation", &r.x))
							transform.setRotation(r);
						ImGui::SameLine();
						if(ImGui::Button("Reset##Rotation"))
							transform.setRotation(glm::quat{});
						
						glm::vec3 s = transform.getScale();
						if(ImGui::InputFloat3("Scale", &s.x))
							transform.setScale(s);
						ImGui::SameLine();
						if(ImGui::Button("Reset##Scale"))
							transform.setScale(glm::vec3{1.0f});
						
						ImGui::Text("Parent: %d", transform.getParent());
						
						ImGui::TreePop();
					}
				} else {
					if(ImGui::Button("Add Transformation component (WIP)"))
					{
						selectedEntityPtr->add<Transformation>();
					}
				}

				if(selectedEntityPtr->has<MeshRenderer>() && ImGui::TreeNodeEx("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto edit_material = [&](Material& mat)
					{
						if(auto uniform_color = mat.searchUniform<glm::vec3>("Color"))
						{
							float col[3] = {uniform_color->getValue().x, uniform_color->getValue().y, uniform_color->getValue().z};
							if(ImGui::ColorEdit3("Color", col))
								uniform_color->setValue(glm::vec3{col[0], col[1], col[2]});
						}
						if(auto uniform_r = mat.searchUniform<float>("R"))
						{
							float val = uniform_r->getValue();
							if(ImGui::SliderFloat("R", &val, 0.0, 1.0))
								uniform_r->setValue(val);
						}
						if(auto uniform_f0 = mat.searchUniform<float>("F0"))
						{
							float val = uniform_f0->getValue();
							if(ImGui::SliderFloat("F0", &val, 0.0, 1.0))
								uniform_f0->setValue(val);
						}
						auto display_texture = [&](const std::string& name) 
						{
							if(auto uniform_tex = mat.searchUniform<Texture>(name))
							{
								ImGui::Text(name.c_str());
								gui_display(uniform_tex->getValue());
							}
						};
						display_texture("Texture");
						display_texture("NormalMap");
					};
					
					auto& mr = selectedEntityPtr->get<MeshRenderer>();
					if(ImGui::TreeNode("Mesh"))
					{
						gui_display(mr.getMesh());
						ImGui::TreePop();
					}
					if(ImGui::TreeNode("Material"))
					{
						edit_material(mr.getMaterial());
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				
				if(selectedEntityPtr->has<SpotLight>())
				{
					if(ImGui::TreeNodeEx("SpotLight", ImGuiTreeNodeFlags_DefaultOpen))
					{
						auto& sl = selectedEntityPtr->get<SpotLight>();
						
						ImGui::Checkbox("Dynamic", &sl.dynamic);
						int downsampling = sl.downsampling;
						if(ImGui::SliderInt("Downsampling", &downsampling, 0, 16))
							sl.downsampling = downsampling;

						float col[3] = {sl.getColor().x, sl.getColor().y, sl.getColor().z};
						if(ImGui::ColorEdit3("Color", col))
							sl.setColor(glm::vec3{col[0], col[1], col[2]});
						
						float ran = sl.getRange();
							if(ImGui::SliderFloat("Range", &ran, 1.0, 1000.0))
								sl.setRange(ran);
							
						float ang = sl.getAngle();
						if(ImGui::SliderFloat("Angle", &ang, 0.0, 3.0))
							sl.setAngle(ang);
						
						const char* resolution_items[] = {"128", "256", "512", "1024", "2048", "4096"};
						static int resolution_item_current = log2(sl.getResolution()) - 7;
						if(ImGui::Combo("Resolution", &resolution_item_current, resolution_items, 6))
							sl.setResolution(pow(2, resolution_item_current + 7));
							
						ImGui::Text("Depth Buffer");
						gui_display(sl.getShadowMap());
						
						ImGui::TreePop();
					}
				} else {
					if(ImGui::Button("Add SpotLight component (WIP)"))
					{
						if(!selectedEntityPtr->has<Transformation>())
							selectedEntityPtr->add<Transformation>();
						auto& spotlight = selectedEntityPtr->add<SpotLight>();
						spotlight.init();
						spotlight.dynamic = true;
						spotlight.updateMatrices();
						spotlight.drawShadowMap(ComponentIterator<MeshRenderer>{});
					}
				}
				
				if(ImGui::Button("Delete Entity"))
				{
					destroy_entity(selectedEntityPtr->get_id());
					deselectObject();
				}
				
				if(ImGui::Button("Deselect Entity"))
					deselectObject();
			} else {
				ImGui::Text("No object selected.");
				
				if(ImGui::Button("Create Entity"))
				{
					auto& new_ent = create_entity("EmptyEntity");
					_selectedObject = new_ent.get_id();
				}
			}
		}
		//ImGui::EndDock();
		ImGui::End();
	}
	
	void gui_scene()
	{
		ImGui::Begin("Scene", &win_scene);
		//ImGui::BeginDock("Scene", &win_scene);
		{
			if(ImGui::TreeNode("Transformation Hierarchy"))
			{
				const static std::function<void(const Transformation&)> explore_hierarchy = [&] (const Transformation& tr)
				{
					ImGui::PushID(&tr);
					auto entityID = get_owner(tr);
					auto& entity = get_entity(entityID);
					if(!tr.getChildren().empty())
					{
						bool expand = ImGui::TreeNodeEx(entity.get_name().c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
						if(ImGui::IsItemClicked()) 
							selectObject(entityID);
						if(expand)
						{
							for(ComponentID child : tr.getChildren())
								explore_hierarchy(get_component<Transformation>(child));
							ImGui::TreePop();
						}
					} else {
						if(ImGui::SmallButton(entity.get_name().c_str()))
							selectObject(entityID);
					}
					ImGui::PopID();
				};
				
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0, 0.0, 0.0, 0.0});
				// Iterate over root (without parent) Transformations.
				for(auto& tr : ComponentIterator<Transformation>{[] (const Transformation& t) { return t.getParent() == invalid_component_idx; }})
					explore_hierarchy(tr);
				ImGui::PopStyleColor(1);
				ImGui::TreePop();
			}
			
			if(ImGui::TreeNode("Entities"))
			{
				static char filter[256] = "\0";
				ImGui::InputText("Filter", filter, 256);
				/// TODO: Use score to order results
				int score = 0;
				/// TODO: Make the matching characters stand out?
				for(auto& e : entities)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0, 0.0, 0.0, 0.0});
					if(e.is_valid() && (filter[0] == '\0' || fts::fuzzy_match(filter, e.get_name().c_str(), score)))
					{
						ImGui::PushID(&e);
						if(ImGui::SmallButton(e.get_name().c_str()))
							selectObject(e.get_id());
						ImGui::PopID();
					}
					ImGui::PopStyleColor();
				}
				ImGui::TreePop();
			}
			
			if(ImGui::TreeNode(("Point Lights (" + std::to_string(_scene.getPointLights().size()) + ")").c_str()))
			{
				for(auto& l : _scene.getPointLights())
				{
					ImGui::PushID(&l);
					ImGui::InputFloat3("Position", &l.position.x);
					ImGui::InputFloat3("Color", &l.color.r);
					ImGui::InputFloat("Range", &l.range);
					ImGui::Separator();
					ImGui::PopID();
				}
				ImGui::TreePop();
				ImGui::Separator();
				PointLight l;
				ImGui::InputFloat3("Position", &l.position.x);
				ImGui::InputFloat3("Color", &l.color.r);
				ImGui::InputFloat("Range", &l.range);
				if(ImGui::Button("Add PointLight"))
					_scene.getPointLights().push_back(l);
			}
			
			if(ImGui::TreeNode("Resources"))
			{
				if(ImGui::TreeNode("Textures"))
				{
					for(auto& p : Resources::_textures)
						if(ImGui::TreeNode(p.first.c_str()))
						{
							gui_display(*p.second);
							ImGui::TreePop();
						}
					ImGui::TreePop();
				}
				if(ImGui::TreeNode("Shaders"))
				{
					for(auto& p : Resources::_shaders)
					{
						if(ImGui::TreeNode(p.first.c_str()))
						{
							ImGui::PushID(&p);
							ImGui::Text("Path: %s", p.second->getPath().c_str());
							ImGui::Text(p.second->isValid() ? "Valid" : "Invalid!");
							ImGui::SameLine();
							if(ImGui::SmallButton("Reload"))
							{
								Log::info("Reloading ", p.first, "...");
								p.second->reload();
								Log::info(p.first, " reloaded.");
							}
							ImGui::PopID();
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
				if(ImGui::TreeNode("Programs"))
				{
					for(auto& p : Resources::_programs)
					{
						if(p.second.isValid())
							ImGui::Text("%s: Valid", p.first.c_str());
						else
							ImGui::Text("%s: Invalid!", p.first.c_str());
					}
					ImGui::TreePop();
				}
				if(ImGui::TreeNode("Meshs"))
				{
					for(auto& p : Resources::_meshes)
						if(ImGui::TreeNode(p.first.c_str()))
						{
							gui_display(*p.second);
							ImGui::TreePop();
						}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
		//ImGui::EndDock();
		ImGui::End();
	}
	
	void gui_logs()
	{
		ImGui::Begin("Logs", &win_logs);
		//ImGui::BeginDock("Logs", &win_logs);
		{
			static int log_level_current = 0;
			ImGui::Combo("Log Level", &log_level_current, Log::_log_types.data(), 3);
			std::vector<Log::LogLine*> tmp_logs;
			if(log_level_current > 0)
				for(auto& l : Log::_logs)
					if(log_level_current <= l.type)
						tmp_logs.push_back(&l);
			
			ImGui::BeginChild("Logs Lines");
			ImGuiListClipper clipper(log_level_current > 0 ? tmp_logs.size() : Log::_logs.size());
			while(clipper.Step())
				for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					if(log_level_current > 0)
						ImGui::TextColored(LogColors[tmp_logs[i]->type], "%s", tmp_logs[i]->str().c_str());
					else
						ImGui::TextColored(LogColors[Log::_logs[i].type], "%s", std::string(Log::_logs[i]).c_str());
				}
			ImGui::EndChild();
		}
		//ImGui::EndDock();
		ImGui::End();
	}
};

int main(int argc, char* argv[])
{
	Editor _app(argc, argv);
	_app.init();	
	_app.run();
}
