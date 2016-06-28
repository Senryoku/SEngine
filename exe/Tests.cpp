#include <sstream>
#include <iomanip>

#include <Query.hpp>

#include <Random.hpp>
#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <ForwardRenderer.hpp>

#include <MathTools.hpp>
#include <GUIText.hpp>
#include <GUIButton.hpp>
#include <GUICheckbox.hpp>
#include <GUIEdit.hpp>
#include <GUIGraph.hpp>
#include <GUISeparator.hpp>
#include <Axes.hpp>

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public ForwardRenderer
{
public:
	Test(int argc, char* argv[]) : 
		ForwardRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		_far = 200.0f;
		ForwardRenderer::run_init();
		
		auto& Forward = ResourcesManager::getInstance().getProgram("Forward");
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
				
		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj", Forward);
		for(auto part : TestMesh)
		{
			part->createVAO();
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))));
		}
				
		_scene.getSkybox().loadCubeMap({
			"in/Textures/skybox/posx.png",
			"in/Textures/skybox/negx.png",
			"in/Textures/skybox/posy.png",
			"in/Textures/skybox/negy.png",
			"in/Textures/skybox/posz.png",
			"in/Textures/skybox/negz.png"
		});
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("Update (ms): ", [&]() -> float { return 1000.f * _updateTiming; }, 0.0, 15.0, 7.5));
		w->add(new GUISeparator(w));
		w->add(new GUIGraph<float>("Frame Time (ms): ", [&]() -> float { return 1000.f * TimeManager::getInstance().getRealDeltaTime(); }, 0.0, 20.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", [&]() -> float { return TimeManager::getInstance().getInstantFrameRate(); }, 0.0, 450.0, 7.5));
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUISeparator(w));
		w->add(new GUIText("Stats"));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUICheckbox("Vsync", [&] {
			_vsync = !_vsync; 
			glfwSwapInterval(_vsync); 
			return _vsync;
		}));
		w2->add(new GUICheckbox("Fullscreen", [&]() -> bool {
			setFullscreen(!_fullscreen);
			return _fullscreen;
		}));
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIText("Controls"));
		
		_axes.reset(new Axes());
	}

	virtual void update() override
	{
		auto start = TimeManager::getInstance().getRuntime();
		ForwardRenderer::update();		
		_updateTiming = TimeManager::getInstance().getRuntime() - start;
	}

	virtual void render() override
	{
		ForwardRenderer::render();
		
		Context::enable(Capability::Blend);
		auto& s = ResourcesManager::getInstance().getProgram("Simple");
		s.use();
		s.setUniform("Color", glm::vec4(1.0));
		_axes->draw();
		s.setUniform("Color", glm::vec4(1.0, 1.0, 1.0, 0.25));
		_axes->drawMarks();
		s.useNone();
	}
	
protected:
	float	_updateTiming;
	Query	_lightsTiming;
	
	std::vector<glm::vec3>	_partVelocities;
	std::unique_ptr<Axes>	_axes;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
