#include <SpotLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective

#include <MathTools.hpp>
#include <Resources.hpp>
#include <Blur.hpp>

const glm::mat4 SpotLight::s_depthBiasMVP
(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

Program* SpotLight::s_depthProgram = nullptr;

SpotLight::SpotLight(unsigned int shadowMapResolution) :
	_entity(get_owner<SpotLight>(*this)),
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution)
{
	updateMatrices();
}

SpotLight::SpotLight(const nlohmann::json& json) :
	dynamic(json["dynamic"]),
	downsampling(json["downsampling"]),
	_entity(get_owner<SpotLight>(*this)),
	_color(vec3(json["color"])),
	_angle(json["angle"]),
	_range(json["range"]),
	_shadowMapResolution(json["resolution"]),
	_shadowMapFramebuffer(_shadowMapResolution)
{
	updateMatrices();
	init();
}

void SpotLight::setRange(float r)
{
	_range = r; 
	updateMatrices();
}

void SpotLight::setAngle(float a)
{
	_angle = a;
	updateMatrices();
}

void SpotLight::updateMatrices()
{
	_projection = glm::perspective(_angle, 1.0f, 0.5f, _range);
	
	glm::vec3 up{0, 1, 0};
	const auto direction = glm::vec3{getTransformation().getRotation() * glm::vec4(0, 0, 1, 1)};
	const auto& position = getTransformation().getGlobalPosition();
	if(glm::cross(direction, up) == glm::vec3(0.0))
		up = {0, 0, 1};
	_view = glm::lookAt(position, position + direction, up);
	_VPMatrix = _projection * _view;
	_biasedVPMatrix = s_depthBiasMVP * _VPMatrix;
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}

void SpotLight::init()
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

void SpotLight::bind() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapProgram().setUniform("DepthVP", getMatrix());
	getShadowMapProgram().use();
}

void SpotLight::unbind() const
{
	Program::useNone();
	getShadowBuffer().unbind();
}

void SpotLight::drawShadowMap(const ComponentIterator<MeshRenderer>& objects) const
{
	getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	bind();
	getShadowMap().bind();
	Context::disable(Capability::CullFace);
	
	for(auto& b : objects)
		if(b.isVisible(getProjectionMatrix(), getViewMatrix()))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getTransformation().getGlobalMatrix());
			b.getMesh().draw();
		}
		
	unbind();
	
	getShadowMap().generateMipmaps();
	/// @todo Add some way to configure the blur
	blur(getShadowMap(), getResolution(), getResolution(), downsampling);
	getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
	getShadowMap().generateMipmaps();
}
	
void SpotLight::initPrograms()
{
	if(s_depthProgram == nullptr)
	{
		s_depthProgram = &Resources::loadProgram("Light_Depth",
			Resources::load<VertexShader>("src/GLSL/depth_vs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/depth_fs.glsl")
		);
	}
}