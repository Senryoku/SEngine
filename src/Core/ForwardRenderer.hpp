#pragma once

#include <Application.hpp>

class ForwardRenderer : public Application
{
public:
	ForwardRenderer(int argc, char* argv[]) : 
		Application(argc, argv)
	{
	}
	
	virtual void run_init() override;

protected:
	virtual void render() override;
};
