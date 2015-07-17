#include <GUIGraph.hpp>

template<typename T>
GUIGraph<T>::GUIGraph(const std::string& label, T* value, T min, T max, float time) :
	_value(value),
	_min(min),
	_max(max),
	_timeWindow(time),
	_strValue(std::to_string(*_value)),
	_textLabel(label),
	_textValue(_strValue)
{
	init();
}

template<typename T>
GUIGraph<T>::GUIGraph(const std::string& label, std::function<T()> func, T min, T max, float time) :
	_func(func),
	_min(min),
	_max(max),
	_timeWindow(time),
	_strValue(std::to_string(_func())),
	_textLabel(label),
	_textValue(_strValue)
{
	init();
}

template<typename T>
void GUIGraph<T>::updateAABB()
{
	_aabb = _textLabel.getAABB() + (_textValue.getAABB() + _textValue.Position);
	_aabb.min -= glm::vec2{4.0, 00.0};
	_aabb.max += glm::vec2{4.0, 40.0};
	_aabb.max.x = glm::max(_aabb.max.x, 250.0f);
}

template<typename T>
void GUIGraph<T>::draw(const glm::vec2& resolution, const glm::vec2& position)
{
	auto p = c2p(position);

	drawAABB(resolution, p, glm::vec4(0.0, 0.3, 0.3, 0.5));
	_line.draw(resolution, p + _aabb.min);

	_textLabel.draw(resolution, p);
	_textValue.draw(resolution, p);
}

template<typename T>
void GUIGraph<T>::update(float runtime, float deltaTime)
{
	T v;
	if(_func)
		v = _func();
	else
		v = *_value;
	addSample({v, runtime});
	_textValue.setText(std::to_string(v));
}

template<typename T>
void GUIGraph<T>::addSample(Sample p)
{
	if(!_plotSamples.empty() && p.time - _plotSamples.back().time < _minDelta)
		return;
	
	_plotSamples.push_back(p);
	while(p.time - _plotSamples.front().time > _timeWindow)
		_plotSamples.pop_front();
	
	_line.getVertices().clear();
	float max_t = _plotSamples.back().time;
	for(auto& s : _plotSamples)
	{
		float t = (s.time - (max_t - _timeWindow)) / _timeWindow;
		float v = (glm::clamp(_min, _max, s.value) - _min) / (_max - _min);
		_line.getVertices().push_back({
			t * (_aabb.max.x - _aabb.min.x),
			v * (_aabb.max.y - _aabb.min.y)
		});
	}
	
	_line.update(Buffer::Usage::DynamicDraw);
}

template<typename T>
void GUIGraph<T>::init()
{
	_textLabel.setFontSize(16.0);
	_textValue.setFontSize(16.0);
	_textValue.Position = {_textLabel.getAABB().max.x, 0.0};
	updateAABB();
	
	// Subscribe to time updates
	TimeManager::getInstance().subscribe(
		std::bind(&GUIGraph<T>::update,
					this,
					std::placeholders::_1,
					std::placeholders::_2)
	);
}
