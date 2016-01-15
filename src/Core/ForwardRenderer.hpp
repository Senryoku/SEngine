#pragma once

#include <Application.hpp>

class ForwardRenderer : public Application
{
public:
	ForwardRenderer();
	ForwardRenderer(int argc, char* argv[]);
	virtual ~ForwardRenderer() =default;
	
	virtual void run_init() override;

protected:
	virtual void render() override;
};
