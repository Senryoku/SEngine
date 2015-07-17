#include <Light.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective
#include <glm/gtx/transform.hpp> // glm::translate

#include <MathTools.hpp>
#include <Blur.hpp>
#include <ResourcesManager.hpp>

///////////////////////////////////////////////////////////////////
// Static attributes

const glm::mat4 Light::s_depthBiasMVP
(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

Program* 			Light::s_depthProgram = nullptr;
VertexShader*		Light::s_depthVS = nullptr;
FragmentShader*	Light::s_depthFS = nullptr;
Program* 			Light::s_depthInstanceProgram = nullptr;
VertexShader*		Light::s_depthInstanceVS = nullptr;

///////////////////////////////////////////////////////////////////

Light::Light(unsigned int shadowMapResolution) :
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution),
	_projection(glm::perspective(_angle, 1.0f, 0.5f, _range))
{
}

void Light::setRange(float r)
{
	_range = r; 
	_projection = glm::perspective(_angle, 1.0f, 0.5f, _range);
}

void Light::setAngle(float a)
{
	_angle = a; 
	_projection = glm::perspective(_angle, 1.0f, 0.5f, _range);
}

void Light::init()
{
	initPrograms();

	_shadowMapFramebuffer.getColor().init();
	_shadowMapFramebuffer.getColor().bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _shadowMapResolution, _shadowMapResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MinFilter, GL_LINEAR_MIPMAP_LINEAR);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MagFilter, GL_LINEAR);
	_shadowMapFramebuffer.getColor().unbind();
	_shadowMapFramebuffer.init();
	
	_gpuBuffer.init();
}

void Light::updateMatrices()
{
	glm::vec3 up{0, 1, 0};
	if(glm::cross(_direction, up) == glm::vec3(0.0))
		up = {0, 0, 1};
	_view = glm::lookAt(_position, _position + _direction, up);
	_VPMatrix = _projection * _view;
	_biasedVPMatrix = s_depthBiasMVP * _VPMatrix;
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}

void Light::bind() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapProgram().setUniform("DepthVP", getMatrix());
	getShadowMapProgram().use();
	Context::enable(Capability::CullFace);
}

void Light::bindInstanced() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapInstanceProgram().setUniform("DepthVP", getMatrix());
	getShadowMapInstanceProgram().use();
}

void Light::unbind() const
{
	Context::disable(Capability::CullFace);
	Program::useNone();
	getShadowBuffer().unbind();
}

void Light::drawShadowMap(const std::vector<MeshInstance>& objects) const
{
	getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	bind();
	getShadowMap().bind();
	
	for(auto& b : objects)
		if(b.isVisible(getProjectionMatrix(), getViewMatrix()))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getModelMatrix());
			b.getMesh().draw();
		}
		
	unbind();
	
	getShadowMap().generateMipmaps();
	/// @todo Add some way to configure the blur
	blur(getShadowMap(), getResolution(), getResolution(), downsampling);
	getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
	getShadowMap().generateMipmaps();
}
	
void Light::initPrograms()
{
	if(s_depthProgram == nullptr)
	{
		s_depthProgram = &ResourcesManager::getInstance().getProgram("Light_Depth");
		s_depthVS = &ResourcesManager::getInstance().getShader<VertexShader>("Light_DepthVS");
		s_depthFS = &ResourcesManager::getInstance().getShader<FragmentShader>("Light_DepthFS");
		
		s_depthInstanceProgram = &ResourcesManager::getInstance().getProgram("Light_DepthInstance");
		s_depthInstanceVS = &ResourcesManager::getInstance().getShader<VertexShader>("Light_DepthInstanceVS");
	}
	
	if(s_depthProgram != nullptr && !s_depthProgram->isValid())
	{
		s_depthVS->loadFromFile("src/GLSL/depth_vs.glsl");
		s_depthVS->compile();
		s_depthFS->loadFromFile("src/GLSL/depth_fs.glsl");
		s_depthFS->compile();
		s_depthProgram->attach(*s_depthVS);
		s_depthProgram->attach(*s_depthFS);
		s_depthProgram->link();
		
		s_depthInstanceVS->loadFromFile("src/GLSL/depth_instance_vs.glsl");
		s_depthInstanceVS->compile();
		s_depthInstanceProgram->attach(*s_depthInstanceVS);
		s_depthInstanceProgram->attach(*s_depthFS);
		s_depthInstanceProgram->link();
	}
}
