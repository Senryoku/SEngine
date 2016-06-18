#pragma once

class Animation
{
public:
	struct Bone
	{
		glm::vec4	position;
		glm::quat	rotation;
		// No scaling.
	};
	
	double								fps = 30.0;
	std::vector<std::vector<Bone>>		bones;
	
	std::vector<Bone> get(const Skeleton& s, double t) const;
	
private:
};
