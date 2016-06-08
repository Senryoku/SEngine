#include <sstream>
#include <iomanip>

#include <Query.hpp>

#include <Random.hpp>
#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>
#include <GUIText.hpp>
#include <GUIButton.hpp>
#include <GUICheckbox.hpp>
#include <GUIEdit.hpp>
#include <GUIGraph.hpp>
#include <GUISeparator.hpp>
#include <Axes.hpp>
#include <Raytracing.hpp>

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public Application
{
public:
	Test(int argc, char* argv[]) : 
		Application(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		Application::run_init();
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 25.0, -25.0));
		_camera.lookAt(glm::vec3(0.0, 0.0, 0.0));
		/*
		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/ame_nebula/posx.tga",
				"in/Textures/skybox/ame_nebula/negx.tga",
				"in/Textures/skybox/ame_nebula/posy.tga",
				"in/Textures/skybox/ame_nebula/negy.tga",
				"in/Textures/skybox/ame_nebula/posz.tga",
				"in/Textures/skybox/ame_nebula/negz.tga"});
		*/
		
		_testMesh = Mesh::loadNoShading("in/3DModels/Sword/Sword OBJ/Sword OBJ.obj");
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("Tracing (ms): ", [&]() -> float { return 1000.f * _traceTiming; }, 0.0, 15.0, 7.5));
		w->add(new GUISeparator(w));
		w->add(new GUIGraph<float>("Frame Time (ms): ", [&]() -> float { return 1000.f * TimeManager::getInstance().getRealDeltaTime(); }, 0.0, 50.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", [&]() -> float { return TimeManager::getInstance().getInstantFrameRate(); }, 0.0, 60.0, 7.5));
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUISeparator(w));
		w->add(new GUIText("Stats"));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIText("Controls"));
	}

	bool scene(Ray r, glm::vec3& p, glm::vec3& n, glm::vec3& c) const
	{
		float depth = 1000.0;
		bool b = false;
		b = trace(r, Sphere{glm::vec3(0.0, 10.0, 0.0), 5.0}, depth, p, n) || b;
		b = trace(r, Sphere{glm::vec3(20.0, 10.0, 0.0), 5.0}, depth, p, n) || b;
		b = trace(r, Plane{glm::vec3(0.0, -2.5, 0.0), glm::vec3(0.0, 1.0, 0.0)}, depth, p, n) || b;
		b = trace(r, Plane{glm::vec3(-50.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0)}, depth, p, n) || b;
		b = trace(r, Plane{glm::vec3(50.0, 0.0, 0.0), glm::vec3(-1.0, 0.0, 0.0)}, depth, p, n) || b;
		b = trace(r, Plane{glm::vec3(-0.0, 0.0, -50.0), glm::vec3(0.0, 0.0, 1.0)}, depth, p, n) || b;
		b = trace(r, Plane{glm::vec3(-0.0, 0.0, 50.0), glm::vec3(0.0, 0.0, -1.0)}, depth, p, n) || b;
		/*
		for(size_t i = 0; i <_testMesh.size(); ++i)
		{
			b = trace(r, *_testMesh[i], depth, p, n) || b;
		}
		*/
		// Lights
		static const Sphere Lights[]{
			//Sphere{glm::vec3(0.0, 50.0, 0.0), 20.0},
			Sphere{glm::vec3(10.0, 25.0, 15.0), 5.0},
			Sphere{glm::vec3(10.0, 25.0, -15.0), 5.0},
			Sphere{glm::vec3(-15.0, 25.0, 0.0), 5.0}
		};
		static const glm::vec3 LightColors[]{
			//glm::vec3(2.0),
			glm::vec3(1.0, 0.0, 0.0),
			glm::vec3(0.0, 1.0, 0.0),
			glm::vec3(0.0, 0.0, 40.0)
		};
		static const glm::vec3 lightColor(1.0);
		for(int i = 0; i < 3; ++i)
			if(trace(r, Lights[i], depth, p, n))
			{
				b = true;
				c = LightColors[i];
			}
			
		if(trace(r, Plane{glm::vec3(0.0, 50.0, 0.0), glm::vec3(0.0, -1.0, 0.0)}, depth, p, n))
		{
			b = true;
			//c = glm::vec3(173/255.0, 216/255.0, 230/255.0);
			c = glm::vec3(1.0);
		}
		
		return b;
	}
	
	inline glm::vec4 tonemap(const glm::vec4& c)
	{
		constexpr float exposure = 2.0;
		return glm::vec4(1.0) - glm::exp(-c * exposure);
	}
	
	inline float tonemap(float c)
	{
		constexpr float exposure = 2.0;
		return 1.0f - std::exp(-c * exposure);
		return std::min(c, 1.0f);
	}
	
	virtual void update() override
	{
		Application::update();
		
		auto start = TimeManager::getInstance().getRuntime();
		
		const size_t samplePerFrame = 1;
		const size_t secondaryRays = 0;
		
		if(_cameraMoved)
		{
			_sampleCount = 0;
			#pragma omp parallel for collapse(2)
			for(int i = 0; i < _height; ++i)
			{
				for(int j = 0; j < _width; ++j)
				{
					auto r = getScreenRay(j, _height - i);
					glm::vec3 p, n, c;					
					glm::vec4 color;
					_hitsColors[i * _width + j] = glm::vec3(0.0);
					_hits[i * _width + j] = scene(r, _hitsPositions[i * _width + j], _hitsNormals[i * _width + j], _hitsColors[i * _width + j]);
					if(_hits[i * _width + j] && _hitsColors[i * _width + j] != glm::vec3(0.0))
						_hits[i * _width + j] = false;
				}
			}
		}
		
		_sampleCount += samplePerFrame;
		
		//const glm::vec3 lightDir = glm::normalize(glm::vec3(1.0));

		glm::vec3 randomDir[samplePerFrame];
		glm::vec3 randomDir2[secondaryRays];
		for(size_t i = 0; i < samplePerFrame; ++i)
			randomDir[i] = RandomHelper::getInstance().getSpherical();
		for(size_t i = 0; i < secondaryRays; ++i)
			randomDir2[i] = RandomHelper::getInstance().getSpherical();
		
		#pragma omp parallel for
		for(int i = 0; i < _height; ++i)
		{
			for(int j = 0; j < _width; ++j)
			{
				glm::vec3 c = _hitsColors[i * _width + j];		
				if(_hits[i * _width + j])
				{
					const glm::vec3& p = _hitsPositions[i * _width + j],
									 n = _hitsNormals[i * _width + j];
					for(size_t s = 0; s < samplePerFrame; ++s)
					{
						#if 0
						auto cos0 = glm::dot(randomDir[s], n);
						const auto dir = (cos0 > 0.0) ? randomDir[s] : -randomDir[s];
						#else
						auto rnd = RandomHelper::getInstance().getSpherical();
						auto cos0 = glm::dot(rnd, n);
						const auto dir = (cos0 > 0.0) ? rnd : -rnd;
						#endif
						cos0 = std::abs(cos0);
						glm::vec3 p2, n2, c2 = glm::vec3(0.0);
						if(scene(Ray{p + 0.00001f * n, dir}, p2, n2, c2))
						{
							if(c2 != glm::vec3(0.0)) // Hit an emissive surface
							{
								c += cos0 * c2;
							} else if(!_cameraMoved && secondaryRays > 0) {
								glm::vec3 tmp(0.0);
								for(size_t s2 = 0; s2 < secondaryRays; ++s2)
								{
									glm::vec3 p3, n3, c3;
									#if 0
									const auto dir2 = (glm::dot(randomDir2[s2], n2) > 0.0) ? randomDir2[s2] : -randomDir2[s2];
									#else
									auto rnd = RandomHelper::getInstance().getSpherical();
									const auto dir2 = (glm::dot(rnd, n2) > 0.0) ? rnd : -rnd;
									#endif
									if(scene(Ray{p2 + 0.00001f * n2, dir2}, p3, n3, c3))
										tmp += c3 * glm::dot(dir2, n2);
								}
								c += cos0 / secondaryRays * tmp;
							}
						}
					}
				}
				glm::vec4 color = glm::vec4(c, 1.0);
				
				if(_cameraMoved)
				{
					_tmpFrame[i * _width + j] = color;
				} else {
					_tmpFrame[i * _width + j] += color;
				}
				
				for(int o = 0; o < 3; ++o)
					_frame[4 * (i * _width + j) + o] = 255.0 * tonemap(_tmpFrame[i * _width + j][o] / _sampleCount);
			}
		}
		_traceTiming = TimeManager::getInstance().getRuntime() - start;

		_blitFB.getColor().update(0, 0, 0, _width, _height, GL_RGBA, &(_frame[0]));
	}

	virtual void render() override
	{
		Context::clear();
		
		//_blitFB.getColor().dump("out/dump.png");
		
		_blitFB.bind(FramebufferTarget::Read);
		glBlitFramebuffer(0, 0, _width, _height, 
							0, 0, _width, _height, 
							GL_COLOR_BUFFER_BIT, GL_LINEAR);
							
		renderGUI();
	}
	
	virtual void resize_callback(GLFWwindow* _window, int width, int height) override
	{
		Application::resize_callback(_window, width, height);
		
		_blitFB = Framebuffer<Texture2D, 1, Texture2D, false>(_width, _height);
		_blitFB.init();
		_frame.reset(new unsigned char[4 * _width * _height]);
		_tmpFrame.reset(new glm::vec4[_width * _height]);
		_hits.reset(new bool[_width * _height]);
		_hitsPositions.reset(new glm::vec3[_width * _height]);
		_hitsNormals.reset(new glm::vec3[_width * _height]);
		_hitsColors.reset(new glm::vec3[_width * _height]);
	}
	
protected:
	std::unique_ptr<unsigned char[]>				_frame;
	std::unique_ptr<glm::vec4[]>					_tmpFrame;
	std::unique_ptr<bool[]>							_hits;
	std::unique_ptr<glm::vec3[]>					_hitsPositions;
	std::unique_ptr<glm::vec3[]>					_hitsNormals;
	std::unique_ptr<glm::vec3[]>					_hitsColors;
	float											_traceTiming = 0.0;
	Framebuffer<Texture2D, 1, Texture2D, false>		_blitFB;
	size_t											_sampleCount = 0;
	
	std::vector<Mesh*>	_testMesh;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
