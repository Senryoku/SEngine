#include <sstream>
#include <iomanip>
#include <deque>

#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <tiny_gltf_loader.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

#include <Log.hpp>

#include <Component.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public DeferredRenderer
{
public:
	Test(int argc, char* argv[]) : 
		DeferredRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		DeferredRenderer::run_init();
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		tinygltf::Scene scene; 
		tinygltf::TinyGLTFLoader loader;
		std::string err;

		bool ret = loader.LoadASCIIFromFile(&scene, &err, "in/box.gltf");
		if (!err.empty())
			Log::error(err);

		if (!ret)
			Log::error("Failed to parse glTF");

		//float R = 0.95f;
		//float F0 = 0.15f;
		for(const auto& m : scene.meshes)
		{
			Log::info("Loading ", m.second.name);
			for(const auto& p : m.second.primitives)
			{
				auto& acc = scene.accessors[p.indices];
				Log::info("Accessor: ", p.indices, " ; ", acc.name);
				auto& mat = scene.materials[p.material];
				Log::info("Material: ", p.material, " ; ", mat.name);
			}
			/*
			auto m = Mesh::load(Paths.begin()[i]);
			for(auto& part : m)
			{
				part->createVAO();
				part->getMaterial().setUniform("R", R);
				part->getMaterial().setUniform("F0", F0);
				_scene.add(MeshInstance(*part, Matrices.begin()[i]));
			}
			*/
		}
		
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(ComponentIterator<MeshRenderer>{});
		
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			_scene.getOmniLights()[i].drawShadowMap(ComponentIterator<MeshRenderer>{});
	}
		
	virtual void renderGUI() override
	{	
		// Plots
		static float last_update = 2.0;
		last_update += TimeManager::getInstance().getRealDeltaTime();
		static std::deque<float> frametimes;
		static std::deque<float> updatetimes;
		static std::deque<float> gbuffertimes;
		static std::deque<float> lighttimes;
		static std::deque<float> postprocesstimes;
		static std::deque<float> guitimes;
		const size_t max_samples = 100;
		float ms = TimeManager::getInstance().getRealDeltaTime() * 1000;
		if(last_update > 0.05 || frametimes.empty())
		{
			if(frametimes.size() > max_samples) frametimes.pop_front();
			frametimes.push_back(ms);
			if(updatetimes.size() > max_samples) updatetimes.pop_front();
			updatetimes.push_back(_updateTiming.get<GLuint64>() / 1000000.0);
			if(gbuffertimes.size() > max_samples) gbuffertimes.pop_front();
			gbuffertimes.push_back(_GBufferPassTiming.get<GLuint64>() / 1000000.0);
			if(lighttimes.size() > max_samples) lighttimes.pop_front();
			lighttimes.push_back(_lightPassTiming.get<GLuint64>() / 1000000.0);
			if(postprocesstimes.size() > max_samples) postprocesstimes.pop_front();
			postprocesstimes.push_back(_postProcessTiming.get<GLuint64>() / 1000000.0);
			if(guitimes.size() > max_samples) guitimes.pop_front();
			guitimes.push_back(_lastGUITiming / 1000000.0);
			last_update = 0.0;
		}
		
		ImGui::Begin("Statistics");
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
		
		ImGui::Begin("Debug");
		{
			ImGui::Checkbox("Pause", &_paused);
			ImGui::SliderFloat("Time Scale", &_timescale, 0.0f, 5.0f);
			ImGui::Separator(); 
			ImGui::Checkbox("Toggle Debug", &_debug_buffers);
			const char* debugbuffer_items[] = {"Color","Position", "Normal"};
			const Attachment debugbuffer_values[] = {Attachment::Color0, Attachment::Color1, Attachment::Color2};
			static int debugbuffer_item_current = 0;
			if(ImGui::Combo("Buffer to Display", &debugbuffer_item_current, debugbuffer_items, 3))
				_framebufferToBlit = debugbuffer_values[debugbuffer_item_current];
		}
		ImGui::End();
		
		ImGui::Begin("Rendering Options");
		{
			if(ImGui::Checkbox("Fullscreen", &_fullscreen))
				setFullscreen(_fullscreen);
			ImGui::SameLine();
			if(ImGui::Checkbox("Vsync", &_vsync))
				glfwSwapInterval(_vsync);
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
		
		ImGui::Begin("Scene");
		{
			if(ImGui::TreeNode("MeshRenderers"))
			{
				for(auto& o : ComponentIterator<MeshRenderer>{})
				{
					ImGui::PushID(&o);
					ImGui::Text("Object");
					ImGui::PopID();
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
		
		ImGui::Begin("Logs");
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
		
		DeferredRenderer::renderGUI();
	}
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
