#pragma once

#include <Mesh.hpp>
#include <Skeleton.hpp>
#include <Animation.hpp>

class RiggedMesh : public Mesh
{
public:
	struct VertexBoneData
	{
		unsigned int	id[4] = {0, 0, 0, 0};
		float			weight[4] = {0, 0, 0, 0};
		
		inline void add(unsigned int i, float w)
		{
			size_t idx = 0;
			while(idx < 4 && weight[idx] != 0)
				++idx;
			id[idx] = i;
			weight[idx] = w;
		}
	};
	
	RiggedMesh();

	virtual void createVAO() override;
	
	Skeleton& getSkeleton() { return _skeleton; }
	std::vector<VertexBoneData>& getVertexBoneData() { return _vertexBoneData; }

private:
	Skeleton						_skeleton;
	std::vector<Animation>		_animations;
	std::vector<VertexBoneData>	_vertexBoneData;
	
	Buffer							_vertexBoneBuffer;
	UniformBuffer					_bonesBuffer;
};
