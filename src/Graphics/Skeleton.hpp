#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

class Skeleton 
{
public:
	struct Node
	{
		Node*									parent = nullptr;
		std::vector<std::unique_ptr<Node>>	children;
		
		int						id = -1; ///< Bone ID, -1 if no bone is attached to this node.
		std::string			name;
		glm::mat4				offset;
	};
	
	std::unique_ptr<Node>	root;
	std::vector<Node*>	bones;
private:
};
