#include <OmnidirectionalLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective
#include <glm/gtx/transform.hpp> // glm::translate

#include <MathTools.hpp>
#include <Blur.hpp>
#include <Resources.hpp>

///////////////////////////////////////////////////////////////////
// Static attributes

Program* 		OmnidirectionalLight::s_depthProgram = nullptr;
VertexShader*	OmnidirectionalLight::s_depthVS = nullptr;
GeometryShader*	OmnidirectionalLight::s_depthGS = nullptr;
FragmentShader*	OmnidirectionalLight::s_depthFS = nullptr;

const glm::mat4 CubeFaceMatrix[6] = {
	glm::mat4(
		 0.0,  0.0, -1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		-1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), glm::mat4(
		 0.0,  0.0,  1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), glm::mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0, -1.0,  0.0,
		 0.0,  1.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), glm::mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0,  0.0,  1.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), glm::mat4(
		 1.0,  0.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0, -1.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	), glm::mat4(
		-1.0,  0.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,  0.0,
		 0.0,  0.0,  1.0,  0.0,
		 0.0,  0.0,  0.0,  1.0 
	)
};

///////////////////////////////////////////////////////////////////

OmnidirectionalLight::OmnidirectionalLight(unsigned int shadowMapResolution) :
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution)
{
	updateMatrices();
}

void OmnidirectionalLight::setRange(float r)
{
	_range = r; 
	updateMatrices();
}

void OmnidirectionalLight::init()
{
	initPrograms();

	_shadowMapFramebuffer.getColor().init();
	_shadowMapFramebuffer.getColor().bind();
	for(int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, _shadowMapResolution, _shadowMapResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapR, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MinFilter, GL_LINEAR);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MagFilter, GL_LINEAR);
	_shadowMapFramebuffer.getColor().unbind();
	
	_shadowMapFramebuffer.init();
	
	_gpuBuffer.init();
}

void OmnidirectionalLight::updateMatrices()
{
	_projection = glm::perspective(static_cast<float>(pi() * 0.5), 1.0f, 0.5f, _range);
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}

void OmnidirectionalLight::bind() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapProgram().setUniform("Position", _position);
	for(int i = 0; i < 6; ++i)
		getShadowMapProgram().setUniform("Projections[" + std::to_string(i) +"]", _projection * CubeFaceMatrix[i]);
	getShadowMapProgram().use();
	Context::enable(Capability::CullFace);
}

void OmnidirectionalLight::unbind() const
{
	Context::disable(Capability::CullFace);
	Program::useNone();
	getShadowBuffer().unbind();
}

void OmnidirectionalLight::drawShadowMap(const std::vector<MeshInstance>& objects) const
{
	//getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	BoundingSphere BoundingVolume(_position, _range);
	
	bind();
	
	for(auto& b : objects)
	{
		if(intersect(b.getAABB(), BoundingVolume))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getModelMatrix());
			b.getMesh().draw();
		}
	}
		
	unbind();
	
	//getShadowMap().generateMipmaps();
	/// @todo Good blur for Cubemaps
	//getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
}
	
void OmnidirectionalLight::initPrograms()
{
	if(s_depthProgram == nullptr)
	{
		s_depthProgram = &Resources::getProgram("OmnidirectionalLight_Depth");
		s_depthVS = &Resources::getShader<VertexShader>("OmnidirectionalLight_DepthVS");
		s_depthGS = &Resources::getShader<GeometryShader>("OmnidirectionalLight_DepthGS");
		s_depthFS = &Resources::getShader<FragmentShader>("OmnidirectionalLight_DepthFS");
	}
	
	if(s_depthProgram != nullptr && !s_depthProgram->isValid())
	{
		if(!*s_depthVS)
		{
			s_depthVS->loadFromFile("src/GLSL/cubedepth_vs.glsl");
			s_depthVS->compile();
		}
		if(!*s_depthGS)
		{
			s_depthGS->loadFromFile("src/GLSL/cubedepth_gs.glsl");
			s_depthGS->compile();
		}
		if(!*s_depthFS)
		{
			s_depthFS->loadFromFile("src/GLSL/lineardepth_fs.glsl");
			s_depthFS->compile();
		}
		s_depthProgram->attach(*s_depthVS);
		s_depthProgram->attach(*s_depthGS);
		s_depthProgram->attach(*s_depthFS);
		s_depthProgram->link();
	}
}
