#include "gui.h"
#include "Tool.h"

int main(void) 
{
	Tools::Local_Tool tool = Tools::Local_Tool("test tool", { "a1", "a2" });
	auto ptr = std::make_shared<Tools::Local_Tool>(tool);
	Tools::Tool_Registry::get_instance().insert("test", ptr);
	std::vector<Tools::Tool_Chain_Entry> ents;
	Tools::Tool_Chain_Entry tce(ptr);
	tce.launch_args["help"] = "yeet";
	ents.push_back(tce);

	Tools::Local_Tool tool1 = Tools::Local_Tool("test tool", { "a11", "a21" });
	auto ptr1 = std::make_shared<Tools::Local_Tool>(tool1);
	Tools::Tool_Registry::get_instance().insert("test1", ptr1);
	Tools::Tool_Chain_Entry tce1(ptr1);
	tce1.launch_args["help1"] = "yeet1";
	ents.push_back(tce1);
	ents.push_back(tce1);

	Tools::Tool_Chain_Registry::get_instance().insert_tool_chain(
		Tools::Tool_Chain
		{
			.entries = ents,
			.description = "test1",
		});
	GUI::start_gui();
	return 0;
}