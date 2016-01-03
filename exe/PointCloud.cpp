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

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

float falloff(float r, float R)
{
	float x = glm::clamp(r / R, 0.0f, 1.0f);
	float y = (1.0f - x * x);
	return y * y * y;
}

float point(glm::vec3 p, glm::vec3 c, float e, float R)
{
	return e * falloff(glm::length(p - c), R);
}

float object(glm::vec3 p)
{
	float r = -5.0;
	r += point(p, glm::vec3(0.25), 6.0, 0.25);
	r += point(p, glm::vec3(0.5), 6.0, 0.25);
	r += point(p, glm::vec3(0.75), 6.0, 0.25);
	
	r += 6.0 * falloff(p.y, 0.25);
	return r;
}

glm::vec3 object_normal(glm::vec3 p)
{
	float eps = 0.001;
	//float eps = 1.0 / Tex3DRes;
	glm::vec3 n;
	auto v = object(p);
	glm::vec3 ex = glm::vec3(eps, 0.0, 0.0);
	glm::vec3 ey = glm::vec3(0.0, eps, 0.0);
	glm::vec3 ez = glm::vec3(0.0, 0.0, eps);
	n.x = object(p + ex) - object(p - ex);
	n.y = object(p + ey) - object(p - ey);
	n.z = object(p + ez) - object(p - ez);
	return -glm::normalize(n);
}

class PointCloud : public Application
{
public:
	PointCloud(int argc, char* argv[]) : 
		Application(argc, argv),
		_data0(Texture::PixelType::Float),
		_data0_n(Texture::PixelType::Float)
	{
	}
	
	virtual void run_init() override
	{
		Application::run_init();
		
		_pointTrace = &loadProgram("PointCloud",
			load<VertexShader>("src/GLSL/PointCloud/vs.glsl"),
			load<FragmentShader>("src/GLSL/PointCloud/fs.glsl")
		);
		if(!*_pointTrace)
			exit(EXIT_FAILURE);
		
		_camera.speed() = 5;
		_camera.setPosition(glm::vec3(0.0, 0.5, 1.0));
		_camera.lookAt(glm::vec3(0.0, 0.0, 0.0));
		
		const size_t Tex3DRes = 256;
		float* data = new float[Tex3DRes * Tex3DRes * Tex3DRes];
		#pragma omp parallel for collapse(3)
		for(size_t i = 0; i < Tex3DRes; ++i)
			for(size_t j = 0; j < Tex3DRes; ++j)
				for(size_t k = 0; k < Tex3DRes; ++k)
					data[i * Tex3DRes * Tex3DRes + j * Tex3DRes + k] = object((1.0f/Tex3DRes) * glm::vec3(i, j, k));
		_data0.create(data, Tex3DRes, Tex3DRes, Tex3DRes, 1);
		_data0.set(Texture::Parameter::MinFilter, GL_LINEAR);
		_data0.set(Texture::Parameter::MagFilter, GL_LINEAR);

		delete[] data;
		
		float* data2 = new float[Tex3DRes * Tex3DRes * Tex3DRes * 3];
		#pragma omp parallel for collapse(3)
		for(size_t i = 0; i < Tex3DRes; ++i)
			for(size_t j = 0; j < Tex3DRes; ++j)
				for(size_t k = 0; k < Tex3DRes; ++k)
				{
					glm::vec3 n = object_normal((1.0f/Tex3DRes) * glm::vec3(i, j, k));
					data2[i * Tex3DRes * Tex3DRes * 3 + j * Tex3DRes * 3 + 3 * k] = n.x;
					data2[i * Tex3DRes * Tex3DRes * 3 + j * Tex3DRes * 3 + 3 * k + 1] = n.y;
					data2[i * Tex3DRes * Tex3DRes * 3 + j * Tex3DRes * 3 + 3 * k + 2] = n.z;
				}
		_data0_n.create(data2, Tex3DRes, Tex3DRes, Tex3DRes, 3);
		/*
		_data0_n.set(Texture::Parameter::MinFilter, GL_NEAREST);
		_data0_n.set(Texture::Parameter::MagFilter, GL_NEAREST);
		*/
		_data0_n.set(Texture::Parameter::MinFilter, GL_LINEAR);
		_data0_n.set(Texture::Parameter::MagFilter, GL_LINEAR);

		delete[] data2;
		
		// GUI
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("Frame Time (ms): ", 
			[&]() -> float { 
				return 1000.f * TimeManager::getInstance().getRealDeltaTime(); }, 0.0, 20.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", 
			[&]() -> float { 
				return TimeManager::getInstance().getInstantFrameRate(); }, 0.0, 450.0, 7.5));
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

	virtual void update() override
	{
		Application::update();
	}

	virtual void render() override
	{
		Context::clear();
	
		_data0.bind(0);
		_pointTrace->setUniform("Data0", 0);
		_data0_n.bind(1);
		_pointTrace->setUniform("Data0_n", 1);
		_pointTrace->setUniform("Resolution", _resolution);
		_pointTrace->setUniform("CameraPosition", _camera.getPosition());
		_pointTrace->setUniform("Forward", _camera.getDirection());
		_pointTrace->use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		_data0.unbind();
		
		renderGUI();
	}
	
protected:
	Program*	_pointTrace = nullptr;
	
	Texture3D	_data0;
	Texture3D	_data0_n;
};

int main(int argc, char* argv[])
{
	PointCloud _app(argc, argv);
	_app.init();	
	_app.run();
}
