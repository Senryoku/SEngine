#pragma once

#include <sstream>
#include <deque>
#include <vector>
#include <functional>

#include <ResourcesManager.hpp>
#include <TimeManager.hpp>
#include <GUIElement.hpp>
#include <GUIText.hpp>
#include <GUILine.hpp>

/**
 * Draws a graph of a given value (from a pointer or a function)
 * as a function of time.
 * @param T Type of the plotted value.
**/
template<typename T = float>
class GUIGraph : public GUIElement
{
public:
	struct Sample
	{
		T	value;
		T	time;
	};
	
	GUIGraph(const std::string& label, T* value, T min, T max, float time);
	
	GUIGraph(const std::string& label, std::function<T()> func, T min, T max, float time);
	
	void updateAABB();

	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override;
	
	void update(float runtime, float deltaTime);
	
	void addSample(Sample p);
	
	/**
	 * Dictate the frequency of updates (and thus the max number of samples).
	 * @param Min time frame between two updates (seconds).
	**/
	inline void setMinDelta(float minDelta) { _minDelta = minDelta; }
	
	/**
	 * Initialize internal data and rendering components.
	**/
	void init();

private:
	T*						_value;
	std::function<T()>	_func;
	T						_min;			///< Min value for the plot
	T						_max;			///< Min value for the plot
	float					_timeWindow;	///< Time window for the plot

	float					_minDelta = 0.01667f;	///< Minimum time difference between two samples

	std::string			_strValue;

	GUIText				_textLabel;
	GUIText				_textValue;
	GUILine				_line = GUILine({0.4, 1.0, 0.2, 1.0});

	std::deque<Sample>	_plotSamples;
};

#include <GUIGraph.tcc>
