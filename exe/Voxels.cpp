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

class Tests : public ForwardRenderer
{
public:
	Tests(int argc, char* argv[]) : 
		ForwardRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		ForwardRenderer::run_init();
		
		_camera.speed() = 5;
		_camera.setPosition(glm::vec3(0.0, 15, 25.0));
		_camera.lookAt(glm::vec3(0.0, 0.0, 0.0));
		
		GLubyte* data = new GLubyte[_chunkSize * _chunkSize * _chunkSize];
		for(size_t i = 0; i < _chunkSize; ++i)
			for(size_t j = 0; j < _chunkSize; ++j)
				for(size_t k = 0; k < _chunkSize; ++k)
					data[i * _chunkSize * _chunkSize + j * _chunkSize + k] = 
						(j > (0.5 * sin(8.0 * i / _chunkSize) + 1.0) * _chunkSize / 8.0 &&
						(i - 32) * (i - 32) + (j - 32) * (j - 32) + (k - 32) * (k - 32) > 25 &&					
						(i - 64) * (i - 64) + (j - 64) * (j - 64) + (k - 32) * (k - 32) > 25 &&					
						(i - 64) * (i - 64) + (j - 32) * (j - 32) + (k - 64) * (k - 64) > 50					
						) ? 0 : 255;
		
		Mesh& chunk_mesh = ResourcesManager::getInstance().getMesh("Chunk_Mesh");
		chunk_mesh.getMaterial().setShadingProgram(ResourcesManager::getInstance().getProgram("Forward"));
		for(size_t i = 0; i < _chunkSize; ++i)
			for(size_t j = 0; j < _chunkSize; ++j)
				for(size_t k = 0; k < _chunkSize; ++k)
				{
					if(data[i * _chunkSize * _chunkSize + j * _chunkSize + k])
					{
						if(i == 0 || !data[(i - 1) * _chunkSize * _chunkSize + j * _chunkSize + k])
						{
							glm::vec3 p = glm::vec3(i - 0.5f, j, k);
							glm::vec3 n = glm::vec3(-1.0f, 0.0f, 0.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, 0.5f, -0.5f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, 0.5f, 0.5f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, -0.5f, 0.5f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, -0.5f, -0.5f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
						
						if(i == _chunkSize - 1 || !data[(i + 1) * _chunkSize * _chunkSize + j * _chunkSize + k])
						{
							glm::vec3 p = glm::vec3(i + 0.5f, j, k);
							glm::vec3 n = glm::vec3(1.0f, 0.0f, 0.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, 0.5f, 0.5f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, 0.5f, -0.5f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, -0.5f, -0.5f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.0f, -0.5f, 0.5f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
						
						if(j == 0 || !data[i * _chunkSize * _chunkSize + (j - 1) * _chunkSize + k])
						{
							glm::vec3 p = glm::vec3(i, j - 0.5f, k);
							glm::vec3 n = glm::vec3(0.0f, -1.0f, 0.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.0f, -0.5f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.0f, 0.5f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.0f, 0.5f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.0f, -0.5f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
						
						if(j == _chunkSize - 1 || !data[i * _chunkSize * _chunkSize + (j + 1) * _chunkSize + k])
						{
							glm::vec3 p = glm::vec3(i, j + 0.5f, k);
							glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.0f, 0.5f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.0f, -0.5f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.0f, -0.5f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.0f, 0.5f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
						
						if(k == 0 || !data[i * _chunkSize * _chunkSize + j * _chunkSize + k - 1])
						{
							glm::vec3 p = glm::vec3(i, j, k - 0.5f);
							glm::vec3 n = glm::vec3(0.0f, 0.0f, -1.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.5f, 0.0f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.5f, 0.0f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, -0.5f, 0.0f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, -0.5f, 0.0f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
						
						if(k == _chunkSize - 1 || !data[i * _chunkSize * _chunkSize + j * _chunkSize + k + 1])
						{
							glm::vec3 p = glm::vec3(i, j, k + 0.5f);
							glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);
							size_t idx = chunk_mesh.getVertices().size();
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, 0.5f, 0.0f), n, glm::vec2(0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, 0.5f, 0.0f), n, glm::vec2(1.0f, 0.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(-0.5f, -0.5f, 0.0f), n, glm::vec2(1.0f)});
							chunk_mesh.getVertices().push_back(Mesh::Vertex{p + glm::vec3(0.5f, -0.5f, 0.0f), n, glm::vec2(0.0f, 1.0f)});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx, idx + 1, idx + 3});
							chunk_mesh.getTriangles().push_back(Mesh::Triangle{idx + 1, idx + 2, idx + 3});
						}
					}
				}
		chunk_mesh.computeBoundingBox();
		chunk_mesh.createVAO();
		static float R = 0.95f; // Will be part of voxel data
		static float F0 = 0.15f;
		chunk_mesh.getMaterial().setUniform("R", &R);
		chunk_mesh.getMaterial().setUniform("F0", &F0);
		for(size_t i = 0; i < 8; ++i)
			for(size_t j = 0; j < 8; ++j)
				for(size_t k = 0; k < 8; ++k)
					_scene.add(MeshInstance(chunk_mesh, glm::scale(glm::translate(glm::mat4(1.0), 
						glm::vec3(i * 128.0, j * 128.0, k * 128.0)), glm::vec3(1.0))));

		//_chunkTexture.create(data, _chunkSize, _chunkSize, _chunkSize, 1);
		//_data0.set(Texture::Parameter::MinFilter, GL_NEAREST);
		//_data0.set(Texture::Parameter::MinFilter, GL_NEAREST_MIPMAP_NEAREST);
		//_data0.set(Texture::Parameter::MagFilter, GL_NEAREST);
		//_data0.set(Texture::Parameter::MinFilter, GL_LINEAR);
		//_data0.set(Texture::Parameter::MagFilter, GL_LINEAR);
		
		delete[] data;
		
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->Dynamic = false;
		o->setColor(glm::vec3(2.0));
		o->setDirection(glm::normalize(glm::vec3{64.0, 0.0, 64.0} - glm::vec3{32.0, 50.0, 13.0}));
		o->_position = glm::vec3{32.0, 50.0, 13.0};
		o->updateMatrices();
		
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(_scene.getObjects());
		
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
	}
	
protected:
	size_t	_chunkSize = 128;
};

int main(int argc, char* argv[])
{
	Tests _app(argc, argv);
	_app.init();	
	_app.run();
}
