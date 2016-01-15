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

class CubeTrace : public Application
{
public:
	CubeTrace(int argc, char* argv[]) : 
		Application(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		Application::run_init();
		
		_cubeTrace = &loadProgram("CubeTrace",
			load<VertexShader>("src/GLSL/CubeTrace/vs.glsl"),
			load<FragmentShader>("src/GLSL/CubeTrace/fs.glsl")
		);
		if(!*_cubeTrace)
			exit(EXIT_FAILURE);
		
		_camera.speed() = 5;
		_camera.setPosition(glm::vec3(0.0, 15, 25.0));
		_camera.lookAt(glm::vec3(0.0, 0.0, 0.0));
		
		GLubyte* data = new GLubyte[_texture3DResolution * _texture3DResolution * _texture3DResolution];
		for(size_t i = 0; i < _texture3DResolution; ++i)
			for(size_t j = 0; j < _texture3DResolution; ++j)
				for(size_t k = 0; k < _texture3DResolution; ++k)
					data[i * _texture3DResolution * _texture3DResolution + j * _texture3DResolution + k] = 
						(j > (0.5 * sin(8.0 * i / _texture3DResolution) + 1.0) * _texture3DResolution / 8.0 &&
						(i - 32) * (i - 32) + (j - 32) * (j - 32) + (k - 32) * (k - 32) > 25 &&					
						(i - 64) * (i - 64) + (j - 64) * (j - 64) + (k - 32) * (k - 32) > 25 &&					
						(i - 64) * (i - 64) + (j - 32) * (j - 32) + (k - 64) * (k - 64) > 50					
						) ? 0 : 255;
						
		_data0.create(data, _texture3DResolution, _texture3DResolution, _texture3DResolution, 1);
		//_data0.set(Texture::Parameter::MinFilter, GL_NEAREST);
		//_data0.set(Texture::Parameter::MinFilter, GL_NEAREST_MIPMAP_NEAREST);
		//_data0.set(Texture::Parameter::MagFilter, GL_NEAREST);
		//_data0.set(Texture::Parameter::MinFilter, GL_LINEAR);
		//_data0.set(Texture::Parameter::MagFilter, GL_LINEAR);
		
		// Manual Mipmap
		_data0.bind();
		size_t res = _texture3DResolution / 2;
		int level = 1;
		while(res > 1)
		{
			for(size_t i = 0; i < res; ++i)
				for(size_t j = 0; j < res; ++j)
					for(size_t k = 0; k < res; ++k)
					{
						size_t x = i * 2;
						size_t y = j * 2;
						size_t z = k * 2;
						data[i * res * res + j * res + k] =
							data[x * res * res + y * res + z] ||
							data[x * res * res + y * res + (z + 1)] ||
							data[x * res * res + (y + 1) * res + z] ||
							data[x * res * res + (y + 1) * res + (z + 1)] ||
							data[(x + 1) * res * res + y * res + z] ||
							data[(x + 1) * res * res + y * res + (z + 1)] ||
							data[(x + 1) * res * res + (y + 1) * res + z] ||
							data[(x + 1) * res * res + (y + 1) * res + (z + 1)] ? 255 : 0;
					}
			glTexImage3D(GL_TEXTURE_3D, 
				 level,
				 GL_RED,
				 static_cast<GLsizei>(res),
				 static_cast<GLsizei>(res),
				 static_cast<GLsizei>(res),
				 0,
				 GL_RED,
				 GL_UNSIGNED_BYTE,
				 data
			); 
			++level;
			res = res / 2;
		}
		glGenerateMipmap(GL_TEXTURE_3D);
		_data0.unbind();

		delete[] data;
		
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
		w2->add(new GUIEdit<float>("AOThreshold ", &_aoThreshold));
		w2->add(new GUIEdit<float>("AOStrength ", &_aoStrength));
		w2->add(new GUICheckbox("Shadows", &_shadows));
		w2->add(new GUIEdit<int>("DisplayedLOD ", &_displayedLoD));
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
	
		_data0.bind();
		_cubeTrace->setUniform("Time", (float) TimeManager::getInstance().getRuntime());
		_cubeTrace->setUniform("Data0", 0);
		_cubeTrace->setUniform("Resolution", _resolution);
		_cubeTrace->setUniform("Tex3DRes", (float) _texture3DResolution);
		_cubeTrace->setUniform("DisplayedLoD", _displayedLoD);
		if(_shadows)
			_cubeTrace->setUniform("Shadows", (int) 1);
		else
			_cubeTrace->setUniform("Shadows", (int) 0);
		_cubeTrace->setUniform("AOStrength", _aoStrength);
		_cubeTrace->setUniform("AOThreshold", _aoThreshold);

		_cubeTrace->setUniform("CameraPosition", _camera.getPosition());
		_cubeTrace->setUniform("Forward", _camera.getDirection());
		_cubeTrace->use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		_data0.unbind();
		
		renderGUI();
	}
	
protected:
	Program*	_cubeTrace = nullptr;
	
	size_t		_texture3DResolution = 64;
	Texture3D	_data0;
	int			_displayedLoD = 0;
	bool		_shadows = true;
	float		_aoStrength = 0.50;
	float		_aoThreshold = 0.50;
};

int main(int argc, char* argv[])
{
	CubeTrace _app(argc, argv);
	_app.init();	
	_app.run();
}
