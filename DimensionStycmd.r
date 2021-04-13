/*--------------------------------------------------------------------------------------+
|
|     $Source: MstnExamples/Elements/exampletool/exampletoolcmd.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Mstn\MdlApi\rscdefs.r.h>
#include <Mstn\MdlApi\cmdclass.r.h>
#define CT_NONE         0
#define CT_MAIN         1
#define CT_DEMO         2 

#pragma suppressREQCmds
CommandTable CT_MAIN =
    { 
    { 1, CT_DEMO,     MANIPULATION,   REQ,            "DEMO" },
    };

CommandTable CT_DEMO =
    {
        { 1, CT_NONE, INHERIT, DEF, "DIMENSIONSTY" },
    };
#define DLLAPP_PRIMARY 1

DllMdlApp DLLAPP_PRIMARY =
    {
      L"DIMENSIONSTY", L"DimensionSty"
    }