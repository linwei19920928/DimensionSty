#include "DimensionSty.h"
static MdlCommandNumber s_commandNumbers[] =
{
{ (CmdHandler)DimensionsCreateTool::InstallNewInstance, CMD_DEMO_DIMENSIONSTY},
0
};
extern "C" void MdlMain(int argc, WCharCP argv[])
{
	/*  Open the resource file  */
	RscFileHandle       rfHandle;

	mdlResource_openFile(&rfHandle, nullptr, RSC_READONLY);
	/*  Register commands */
	mdlSystem_registerCommandNumbers(s_commandNumbers);
	mdlParse_loadCommandTable(nullptr);

}