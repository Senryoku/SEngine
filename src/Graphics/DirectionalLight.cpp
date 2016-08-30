#include <DirectionalLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective
#include <glm/gtx/transform.hpp> // glm::translate

#include <MathTools.hpp>
#include <Blur.hpp>
#include <Resources.hpp>

///////////////////////////////////////////////////////////////////
// Static attributes

const glm::mat4 DirectionalLight::s_depthBiasMVP
(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

///////////////////////////////////////////////////////////////////

DirectionalLight::DirectionalLight(unsigned int shadowMapResolution) :
	Light<Texture2D>(shadowMapResolution)
{
}

void DirectionalLight::init()
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

void DirectionalLight::bind() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapProgram().setUniform("DepthVP", getMatrix());
	getShadowMapProgram().use();
}

void DirectionalLight::unbind() const
{
	Program::useNone();
	getShadowBuffer().unbind();
}

void DirectionalLight::drawShadowMap(const ComponentIterator<MeshRenderer>& objects) const
{
	getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	bind();
	getShadowMap().bind();
	Context::disable(Capability::CullFace);
	
	for(auto& b : objects)
		if(b.isVisible(getProjectionMatrix(), getViewMatrix()))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getTransformation().getModelMatrix());
			b.getMesh().draw();
		}
		
	unbind();
	
	getShadowMap().generateMipmaps();
	/// @todo Add some way to configure the blur
	blur(getShadowMap(), getResolution(), getResolution(), downsampling);
	getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
	getShadowMap().generateMipmaps();
}
	
void DirectionalLight::initPrograms()
{
	if(s_depthProgram == nullptr)
	{
		s_depthProgram = &Resources::loadProgram("Light_Depth",
			Resources::load<VertexShader>("src/GLSL/depth_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/depth_fs.glsl")
		);
	}
}
