#include <sstream>
#include <iomanip>
#include <deque>
#include <set>
#include <experimental/filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glmext.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include <fts_fuzzy_match.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

#include <Log.hpp>

class ImGuiTest : public Application
{
public:
	ImGuiTest(int argc, char* argv[]) :
		Application(argc, argv)
	{
	}

	void render() override
	{
		ImGui::ShowMetricsWindow();
		renderGUI();
	}
};

int main(int argc, char* argv[])
{
	ImGuiTest _app(argc, argv);
	_app.init();	
	_app.run();
}
