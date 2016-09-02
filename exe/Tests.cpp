#include <sstream>
#include <iomanip>
#include <deque>

#include <glm/gtx/transform.hpp>
#include <glmext.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

#include <Log.hpp>

#include <Entity.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
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
		
		auto& LightDraw = Resources::loadProgram("LightDraw",
			Resources::load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			Resources::load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		Simple.bindUniformBlock("Camera", _camera_buffer); 
		LightDraw.bindUniformBlock("Camera", _camera_buffer);
		
		float R = 0.95f;
		float F0 = 0.15f;
		const auto Paths = {
			"in/3DModels/sponza/sponza.obj"
			//,"in/3DModels/sibenik/sibenik.obj"
		};
		const auto Matrices = {
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))
			//,glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(200.0, 0.0, 0.0)), glm::vec3(3.0))
		};
		for(size_t i = 0; i < Paths.size(); ++i)
		{
			auto& base_entity = create_entity();
			base_entity.set_name(Paths.begin()[i]);
			auto base_transform = get_id(base_entity.add<Transformation>());
			auto m = Mesh::load(Paths.begin()[i]);
			for(auto& part : m)
			{
				auto t = part->resetPivot();
				part->createVAO();
				part->getMaterial().setUniform("R", R);
				part->getMaterial().setUniform("F0", F0);
				auto& entity = create_entity();
				entity.set_name(part->getName());
				auto& ent_transform = entity.add<Transformation>(Matrices.begin()[i] * t.getModelMatrix());
				get_component<Transformation>(base_transform).addChild(ent_transform);
				entity.add<MeshRenderer>(*part);
			}
		}
		
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		_volumeSamples = 16;
		// Shadow casting lights ---------------------------------------------------
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->dynamic = false;
		o->setColor(glm::vec3(2.0));
		o->setDirection(glm::normalize(glm::vec3{58.8467 - 63.273, 161.167 - 173.158, -34.2005 - -37.1856}));
		o->_position = glm::vec3{63.273, 173.158, -37.1856};
		o->updateMatrices();
		
		OrthographicLight* o2 = _scene.add(new OrthographicLight());
		o2->init();
		o2->dynamic = false;
		o2->setColor(glm::vec3(2.0));         
		o2->setDirection(glm::normalize(glm::vec3{220.472 - 63.273, -34.6538 - 0.0, 0.789395 - 0.0}));
		o2->_position = glm::vec3{127.27, 0.0, 0.0};
		o2->updateMatrices();
		
		SpotLight* s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(45.0, 12.0, -18.0));
		s->lookAt(glm::vec3(45.0, 0.0, -18.0));
		s->setRange(20.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		
		s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(0.0, 20.0, 00.0));
		s->lookAt(glm::vec3(0.0, 0.0, 0.0));
		s->setRange(50.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		/*
		_scene.getOmniLights().resize(2);
		_scene.getOmniLights()[0].setResolution(2048);
		_scene.getOmniLights()[0].dynamic = true;
		_scene.getOmniLights()[0].init();
		_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0, 25.0, -2.0));
		_scene.getOmniLights()[0].setColor(glm::vec3(1.5));
		_scene.getOmniLights()[0].setRange(40.0f);
		_scene.getOmniLights()[0].updateMatrices();
		
		_scene.getOmniLights()[1].setResolution(2048);
		_scene.getOmniLights()[1].dynamic = true;
		_scene.getOmniLights()[1].init();
		_scene.getOmniLights()[1].setPosition(glm::vec3(200.0, -28.0, 0.0));
		_scene.getOmniLights()[1].setColor(glm::vec3(0.5));
		_scene.getOmniLights()[1].setRange(150.0f);
		_scene.getOmniLights()[1].updateMatrices();
		*/
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(ComponentIterator<MeshRenderer>{});
		
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			_scene.getOmniLights()[i].drawShadowMap(ComponentIterator<MeshRenderer>{});

		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});
	}
	
	virtual void renderGUI() override
	{	
		static bool	win_imgui_stats = false,
					win_stats = false,
					win_rendering = false,
					win_scene = true,
					win_logs = false,
					win_inspect = true;
		
		// Menu
		if(_menu)
			if(ImGui::BeginMainMenuBar())
			{
				if(ImGui::BeginMenu("File"))
				{
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
						for(auto l : _scene.getLights())
						{
							l->updateMatrices();
							l->drawShadowMap(ComponentIterator<MeshRenderer>{});
						}
						
						for(auto& l : _scene.getOmniLights())
						{
							l.updateMatrices();
							l.drawShadowMap(ComponentIterator<MeshRenderer>{});
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
	
		// Plots
		static float last_update = 2.0;
		last_update += TimeManager::getRealDeltaTime();
		static std::deque<float> frametimes, updatetimes, gbuffertimes, lighttimes, postprocesstimes, guitimes;
		constexpr size_t max_samples = 100;
		const float ms = TimeManager::getRealDeltaTime() * 1000;
		if(last_update > 0.05 || frametimes.empty())
		{
			if(frametimes.size() > max_samples)			frametimes.pop_front();
			if(updatetimes.size() > max_samples)		updatetimes.pop_front();
			if(gbuffertimes.size() > max_samples)		gbuffertimes.pop_front();
			if(lighttimes.size() > max_samples)			lighttimes.pop_front();
			if(postprocesstimes.size() > max_samples)	postprocesstimes.pop_front();
			if(guitimes.size() > max_samples)			guitimes.pop_front();
			frametimes.push_back(ms);
			updatetimes.push_back(_updateTiming.get<GLuint64>() / 1000000.0);
			gbuffertimes.push_back(_GBufferPassTiming.get<GLuint64>() / 1000000.0);
			lighttimes.push_back(_lightPassTiming.get<GLuint64>() / 1000000.0);
			postprocesstimes.push_back(_postProcessTiming.get<GLuint64>() / 1000000.0);
			guitimes.push_back(_lastGUITiming / 1000000.0);
			last_update = 0.0;
		}
		
		if(win_imgui_stats)
			ImGui::ShowMetricsWindow();
		
		if(win_stats)
		{
			if(ImGui::Begin("Statistics", &win_stats))
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
				ImGui::PlotLines("GBuffer", lamba_data, &gbuffertimes, gbuffertimes.size(), 0, to_string(gbuffertimes.back(), 4).c_str(), 0.0, 10.0);    
				ImGui::PlotLines("Lights", lamba_data, &lighttimes, lighttimes.size(), 0, to_string(lighttimes.back(), 4).c_str(), 0.0, 10.0);    
				ImGui::PlotLines("Post Process",lamba_data, &postprocesstimes, postprocesstimes.size(), 0, to_string(postprocesstimes.back(), 4).c_str(), 0.0, 10.0);    
				ImGui::PlotLines("GUI", lamba_data, &guitimes, guitimes.size(), 0, to_string(guitimes.back(), 4).c_str(), 0.0, 10.0);         
			}
			ImGui::End();
		}
		
		if(win_rendering)
		{
			ImGui::Begin("Rendering Options", &win_rendering);
			{
				if(ImGui::Checkbox("Fullscreen", &_fullscreen))
					setFullscreen(_fullscreen);
				ImGui::SameLine();
				if(ImGui::Checkbox("Vsync", &_vsync))
					glfwSwapInterval(_vsync);
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
				if(ImGui::DragFloat("FoV", &_fov, 1.0, 40.0, 110.0))
					setFoV(_fov);
				
				ImGui::Separator();
				
				static bool bloom_toggle = _bloom > 0.0;
				if(ImGui::Checkbox("Toggle Bloom", &bloom_toggle))
					_bloom = -_bloom;
				ImGui::DragFloat("Bloom", &_bloom, 0.05, 0.0, 5.0);
				ImGui::DragFloat("Exposure", &_exposure, 0.05, 0.0, 5.0);
				ImGui::DragFloat("MinVariance (VSM)", &_minVariance, 0.000001, 0.0, 0.00005);
				ImGui::DragInt("AOSamples", &_aoSamples, 1, 0, 32);
				ImGui::DragFloat("AOThresold", &_aoThreshold, 0.05, 0.0, 5.0);
				ImGui::DragFloat("AORadius", &_aoRadius, 1.0, 0.0, 400.0);
				ImGui::DragInt("VolumeSamples", &_volumeSamples, 1, 0, 64);
				ImGui::DragFloat("AtmosphericDensity", &_atmosphericDensity, 0.001, 0.0, 0.02);
			
				ImGui::Separator();

				ImGui::ColorEdit3("Ambiant Color", &_ambiant.r);
			}
			ImGui::End();
		}
		
		if(win_scene)
		{
			ImGui::Begin("Scene", &win_scene);
			{
				if(ImGui::TreeNode("Transformation Hierarchy"))
				{
					const static std::function<void(const Transformation&)> explore_hierarchy = [&] (const Transformation& tr)
					{
						ImGui::PushID(&tr);
						auto& entity = get_entity(get_owner(tr));
						bool expand = ImGui::TreeNodeEx(entity.get_name().c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
						if(ImGui::IsItemClicked()) 
							selectObject(&entity);
						if(expand)
						{
							for(ComponentID child : tr.getChildren())
								explore_hierarchy(get_component<Transformation>(child));
							ImGui::TreePop();
						}
						ImGui::PopID();
					};
					
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0, 0.0, 0.0, 0.0});
					for(auto& tr : ComponentIterator<Transformation>{[] (const Transformation& t) { return t.getParent() == invalid_component_idx; }})
						explore_hierarchy(tr);
					ImGui::PopStyleColor();
					ImGui::TreePop();
				}
				
				if(ImGui::TreeNode("Entities"))
				{
					for(auto& e : entities)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0, 0.0, 0.0, 0.0});
						if(e.is_valid())
						{
							ImGui::PushID(&e);
							if(ImGui::SmallButton(e.get_name().c_str()))
								selectObject(&e);
							ImGui::PopID();
						}
						ImGui::PopStyleColor();
					}
					ImGui::TreePop();
				}
				
				if(ImGui::TreeNode(("Directional Lights (" + std::to_string(_scene.getLights().size()) + ")").c_str()))
				{
					for(auto& l : _scene.getLights())
					{
						ImGui::PushID(&l);
						ImGui::PushItemWidth(150);
						float c[3] = {l->getColor().r, l->getColor().g, l->getColor().b};
						if(ImGui::InputFloat3("Color", c))
							l->setColor(glm::vec3{c[0], c[1], c[2]});
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
				
				if(ImGui::TreeNode(("Point Lights (" + std::to_string(_scene.getPointLights().size()) + ")").c_str()))
				{
					for(auto& l : _scene.getPointLights())
					{
						ImGui::PushID(&l);
						ImGui::PushItemWidth(150);
						ImGui::InputFloat3("Position", &l.position.x);
						ImGui::SameLine();
						ImGui::InputFloat3("Color", &l.color.r);
						ImGui::SameLine();
						ImGui::PushItemWidth(50);
						ImGui::InputFloat("Range", &l.range);
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
			ImGui::End();
		}
		
		if(win_logs)
		{
			ImGui::Begin("Logs", &win_logs);
			{
				const ImVec4 LogColors[3] = {
					ImVec4{1, 1, 1, 1},
					ImVec4{1, 1, 0, 1},
					ImVec4{1, 0, 0, 1}
				};
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
			ImGui::End();
		}
		
		if(_selectedObject != nullptr)
		{
			if(_selectedObject->has<Transformation>())
			{
				auto& transform = _selectedObject->get<Transformation>();
				
				// Dummy Window for "on field" widgets
				ImGui::SetNextWindowSize(ImVec2{static_cast<float>(_width), static_cast<float>(_height)});
				ImGui::Begin("SelectedObject", nullptr, ImVec2{static_cast<float>(_width), static_cast<float>(_height)}, 0.0,
					ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
				
				ImGuiContext* g = ImGui::GetCurrentContext();
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				const auto mouse = glm::vec2{ImGui::GetMousePos()};
				ImDrawList* drawlist = ImGui::GetWindowDrawList();
				
				if(_selectedObject->has<MeshRenderer>())
				{
					auto& mr = _selectedObject->get<MeshRenderer>();
					Context::disable(Capability::DepthTest);
					Context::enable(Capability::Blend);
					Context::blendFunc(Factor::SrcAlpha, Factor::OneMinusSrcAlpha);
					Program& blend = Resources::getProgram("Simple");
					blend.use();
					blend.setUniform("Color", _selectedObjectColor);
					blend.setUniform("ModelMatrix", transform.getModelMatrix());
					mr.getMesh().draw();
					Program::useNone();
					Context::disable(Capability::Blend);
				
					auto aabb = mr.getAABB().getBounds();
					std::array<ImVec2, 8> screen_aabb;
					for(int i = 0; i < 8; ++i)
					{
						screen_aabb[i] = project(aabb[i]);
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
		
				const std::array<glm::vec2, 4> gizmo_points{
					project(transform.getPosition() + glm::vec3{0.0, 0.0, 0.0}),
					project(transform.getPosition() + glm::vec3{1.0, 0.0, 0.0}),
					project(transform.getPosition() + glm::vec3{0.0, 1.0, 0.0}),
					project(transform.getPosition() + glm::vec3{0.0, 0.0, 1.0})
				};
				
				static const char* labels[3] = {"PositionGizmoX", "PositionGizmoY", "PositionGizmoZ"};
				for(int i = 0; i < 3; ++i)
				{
					const ImGuiID id = window->GetID(labels[i]);
					bool hovered = point_line_distance(mouse, gizmo_points[0], gizmo_points[1 + i]) < 5.0f;
					if(hovered)
						ImGui::SetHoveredID(id);
					bool active = id == g->ActiveId;
				
					if(active && ImGui::IsMouseDragging(0))
					{
						const auto newP = mouse;
						const auto oldP = newP - glm::vec2{ImGui::GetMouseDragDelta()};
						ImGui::ResetMouseDragDelta(0);
						const auto newR = getScreenRay(newP.x, newP.y);
						const auto oldR = getScreenRay(oldP.x, oldP.y);
						Plane pl{transform.getPosition(), -_camera.getDirection()}; // @todo Project onto something else (line)
						float d0 = std::numeric_limits<float>::max(), d1 = std::numeric_limits<float>::max();
						glm::vec3 p0, p1, n0, n1;
						trace(newR, pl, d0, p0, n0);
						trace(oldR, pl, d1, p1, n1);
						auto newPosition = transform.getPosition();
						newPosition[i] += p0[i] - p1[i];
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
					drawlist->AddLine(gizmo_points[0], gizmo_points[i + 1], 
						ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)), 2.0);
				}
				////////////////////////////////////////////////////
				
				// Rotation Gizmo
				const std::array<glm::vec2, 4> rot_gizmo_points{
					project(transform.getPosition() + glm::vec3{transform.getRotation() * glm::vec4{0.0, 0.0, 0.0, 1.0f}}),
					project(transform.getPosition() + glm::vec3{transform.getRotation() * glm::vec4{1.0, 0.0, 0.0, 1.0f}}),
					project(transform.getPosition() + glm::vec3{transform.getRotation() * glm::vec4{0.0, 1.0, 0.0, 1.0f}}),
					project(transform.getPosition() + glm::vec3{transform.getRotation() * glm::vec4{0.0, 0.0, 1.0, 1.0f}})
				};
				
				static const char* rot_labels[3] = {"RotationGizmoX", "RotationGizmoY", "RotationGizmoZ"};
				const std::array<glm::vec3, 3> plane_normals = {
					glm::vec3{transform.getRotation() * glm::vec4{0.0, 1.0, 0.0, 1.0}},
					glm::vec3{transform.getRotation() * glm::vec4{0.0, 0.0, 1.0, 1.0}},
					glm::vec3{transform.getRotation() * glm::vec4{1.0, 0.0, 0.0, 1.0}},
				};
				for(int i = 0; i < 3; ++i)
				{
					const ImGuiID id = window->GetID(rot_labels[i]);
					bool hovered = glm::distance(mouse, rot_gizmo_points[1 + i]) < 10.0f;
					if(hovered)
						ImGui::SetHoveredID(id);
					bool active = id == g->ActiveId;
					
					if(active && ImGui::IsMouseDragging(0))
					{
						const auto newP = mouse;
						const auto oldP = newP - glm::vec2{ImGui::GetMouseDragDelta()};
						ImGui::ResetMouseDragDelta(0);
						const auto newR = getScreenRay(newP.x, newP.y);
						const auto oldR = getScreenRay(oldP.x, oldP.y);
						Plane pl{transform.getPosition(), 
							(glm::dot(plane_normals[i], _camera.getDirection()) < 0.0 ? 1.0f : -1.0f) * plane_normals[i]};
						float d0 = std::numeric_limits<float>::max(), d1 = std::numeric_limits<float>::max();
						glm::vec3 p0, p1, n0, n1;
						trace(newR, pl, d0, p0, n0);
						trace(oldR, pl, d1, p1, n1);
						
						auto v1 = p1 - transform.getPosition();
						auto v2 = p0 - transform.getPosition();
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
						drawlist->AddLine(project(transform.getPosition() + circle[p]), 
							project(transform.getPosition() + circle[p + 1]),
							ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)));
					}

					drawlist->AddCircle(rot_gizmo_points[i + 1], 10.0,
						ImGui::ColorConvertFloat4ToU32(ImVec4(i == 0, i == 1, i == 2, active ? 1.0 : 0.5)));
				}
				
				ImGui::End();
			}
		}
		
		if(win_inspect)
		{
			if(ImGui::Begin("Entity Inspector", &win_inspect))
			{
				ImGui::ColorEdit4("Highlight color", &_selectedObjectColor.x);
				ImGui::Separator();
				if(_selectedObject != nullptr)
				{
					ImGui::Text("Name: %s", _selectedObject->get_name().c_str());
					if(_selectedObject->has<Transformation>() && ImGui::TreeNodeEx("Transformation", ImGuiTreeNodeFlags_DefaultOpen))
					{
						auto& transform = _selectedObject->get<Transformation>();
						
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
						
						ImGui::TreePop();
					}
					
					auto edit_material = [&](Material& mat)
					{
						auto uniform_color = mat.searchUniform<glm::vec3>("Color");
						if(uniform_color != nullptr)
						{
							float col[3] = {uniform_color->getValue().x, uniform_color->getValue().y, uniform_color->getValue().z};
							if(ImGui::ColorEdit3("Color", col))
								uniform_color->setValue(glm::vec3{col[0], col[1], col[2]});
						}
						auto uniform_r = mat.searchUniform<float>("R");
						if(uniform_r != nullptr)
						{
							float val = uniform_r->getValue();
							if(ImGui::SliderFloat("R", &val, 0.0, 1.0))
								uniform_r->setValue(val);
						}
						auto uniform_f0 = mat.searchUniform<float>("F0");
						if(uniform_f0 != nullptr)
						{
							float val = uniform_f0->getValue();
							if(ImGui::SliderFloat("F0", &val, 0.0, 1.0))
								uniform_f0->setValue(val);
						}
						auto display_texture = [&](const std::string& name) 
						{
							auto uniform_tex = mat.searchUniform<Texture>(name);
							if(uniform_tex != nullptr)
							{
								ImGui::Text(name.c_str());
								ImGui::Image(reinterpret_cast<void*>(uniform_tex->getValue().getName()), ImVec2{256, 256});
							}
						};
						display_texture("Texture");
						display_texture("NormalMap");
					};
					
					if(_selectedObject->has<MeshRenderer>() && ImGui::TreeNodeEx("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
					{
						auto& mr = _selectedObject->get<MeshRenderer>();
						if(ImGui::TreeNode("Mesh"))
						{
							ImGui::Text("Name: %s", mr.getMesh().getName().c_str());
							ImGui::Text("Path: %s", mr.getMesh().getPath().c_str());
							
							// renders the mesh into a small framebuffer
							static Framebuffer<> model_render{256};
							if(!model_render)
								model_render.init();
							static auto& GUIModelRender = Resources::loadProgram("GUIModelRender",
								Resources::load<VertexShader>("src/GLSL/Forward/simple_vs.glsl"),
								Resources::load<FragmentShader>("src/GLSL/Forward/simple_fs.glsl")
							);
							static float gui_model_render_fov = 1.0f;
							static float gui_model_render_camx = 400;	/// @todo Compute it automatically
							auto aabb = mr.getMesh().getBoundingBox();
							ImGui::SliderFloat("Render FoV", &gui_model_render_fov, 0.1, 5.0);
							ImGui::SliderFloat("Position", &gui_model_render_camx, 0, 1000);
							GUIModelRender.setUniform("ProjectionMatrix", glm::perspective(gui_model_render_fov, 1.0f, 1.0f, 1500.0f));
							GUIModelRender.setUniform("ViewMatrix", glm::lookAt(
							   glm::vec3(gui_model_render_camx, 0.5f * (aabb.max.y + aabb.min.y), 0),
							   0.5f * (aabb.max + aabb.min),
							   glm::vec3(0, -1, 0)
							));
							GUIModelRender.setUniform("ModelMatrix", glm::rotate(glm::mat4(1.0f), _time, glm::vec3{0, 1, 0}));
							Material mat = mr.getMesh().getMaterial(); // Copy Material... not perfect but will do for now.
							mat.setShadingProgram(GUIModelRender);
							mat.use();
							Context::enable(Capability::DepthTest);
							model_render.bind();
							glClearColor(0, 0, 0, 0);
							Context::clear();
							mr.getMesh().draw();
							model_render.unbind();
							
							ImGui::Image(reinterpret_cast<void*>(model_render.getColor().getName()), ImVec2{256, 256});
							
							// if(ImGui::TreeNode("Base Material"))
							// {
								// edit_material(mr.getMesh().getMaterial());
								// ImGui::TreePop();
							// }
							
							ImGui::TreePop();
						}
						
						if(ImGui::TreeNode("Material"))
						{
							edit_material(mr.getMaterial());
							ImGui::TreePop();
						}
						ImGui::TreePop();
					}
					
					if(ImGui::Button("Delete Entity"))
					{
						destroy_entity(_selectedObject->get_id());
						deselectObject();
					}
				} else {
					ImGui::Text("No object selected.");
				}
			}
			ImGui::End();
		}
		
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
						selectObject(&entities[get_owner<MeshRenderer>(o)]);
				}
			}
		}
		
		DeferredRenderer::renderGUI();
	}
		
protected:
	Entity*			_selectedObject = nullptr;
	glm::vec4		_selectedObjectColor = glm::vec4{0.0, 0.0, 1.0, 0.10};

	void deselectObject()
	{
		_selectedObject = nullptr;
	}
	
	void selectObject(Entity* o)
	{
		if(_selectedObject != nullptr)
			deselectObject();
		_selectedObject = o;
	}
};

int main(int argc, char* argv[])
{
	Editor _app(argc, argv);
	_app.init();	
	_app.run();
}
