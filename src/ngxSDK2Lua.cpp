// ngxSDKtoLua.cpp : Defines the exported functions for the DLL application.
//

#define NGX_VERSION   "ngxSDK2Lua 0.5 2012-03-24"
#define NGX_AUTHORS   "Darío Iriberri"

#include <sstream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <exception>
#include <strsafe.h>
#include <ctime>
#include <cmath>
#include <dirent.h>
#include <sys/types.h>
//#include <boost/thread.hpp>

extern "C" {
#include "lua-5.1.5\src\lua.h"
#include "lua-5.1.5\src\lauxlib.h"
#include "lua-5.1.5\src\lualib.h"
}

#include "SimConnect.h"
#include "PMDG_NGX_SDK.h"
#include "LuaCall.h"
#include "definitions.h"
#include "FSUIPC_User.h"

void printLog(string textToPrint) {
	bool print = true;
	int count = 1;

	while (print && count < 10) {
		try {
			LuaCall<NullT, string>(LuaGlobal, "printLog").call(textToPrint);
			print = false;

		} catch (...) {
			count ++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//										private	ipc											  //
////////////////////////////////////////////////////////////////////////////////////////////////

void writeUB(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 1, pSrce, &dwResult);
}

void writeSB(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 1, pSrce, &dwResult);
}

void writeUW(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 2, pSrce, &dwResult);
}

void writeSW(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 2, pSrce, &dwResult);
}

void writeUD(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 4, pSrce, &dwResult);
}

void writeSD(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 4, pSrce, &dwResult);
}

void writeFLT(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 4, pSrce, &dwResult);
}

void writeDBL(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 4, pSrce, &dwResult);
}

void writeDD(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	FSUIPC_Write(dwOffset, 4, pSrce, &dwResult);
}

void writeSTR(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	const char* tmp = (const char*)pSrce;
	int lenS = strlen(tmp); 

	FSUIPC_Write(dwOffset, lenS, pSrce, &dwResult);
}

void setbit(DWORD dwOffset, void *pSrce, DWORD bitPos, bool data) {
	BYTE readval;
	DWORD size;

	if (bitPos > 15) {
		size = 4;
	} else if (bitPos > 7) {
		size = 2;
	} else {
		size = 1;
	}

	FSUIPC_Read(dwOffset, size, &readval, &dwResult);
	FSUIPC_Process(&dwResult);
	BYTE res;

	DWORD mask = pow (2,(double)bitPos);

	if (data) {
		res = readval | mask;
	} else {
		res = readval & (~mask);
	}
		
	FSUIPC_Write(dwOffset, 1, &res, &dwResult);
}

void callOffsetMap(string item, void* data, bool dataB, void (*pFunc)(DWORD dwOffset, void* value, DWORD bitPos, bool data)) {
	OffsetMappings::iterator iter = offsetMappings.find(item);

	if(iter != offsetMappings.end()) {
		OffsetCall o2 = offsetMappings[item];
		
		DWORD doff = (&o2)->offset;
		DWORD bitPos = (&o2)->bitPos;

		pFunc(doff, data, bitPos, dataB);

		//FSUIPC_Process(&dwResult);
	}
}

void callUnsignedCharItem(unsigned char ucdata, unsigned char &olddata, string item, bool &implControl) {
	if (ucdata != olddata)	{

		callOffsetMap(item, &ucdata, NULL, writeUB);
		
		if (implControl)	{
			try	{
				//LuaCall<NullT, unsigned char>(LuaGlobal, "IRS_DisplaySelector").call('G');
				olddata = ucdata;
				int t = ucdata;
				LuaCall<NullT, int>(LuaGlobal, item).call(t);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);

				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callBooleanItem(bool bdata, bool &olddata, string item, bool &implControl) {
	if (bdata != olddata)	{

		callOffsetMap(item, &bdata, bdata, setbit);

		if (implControl)	{
			try	{
				olddata = bdata;
				int t = bdata;
				LuaCall<NullT, int>(LuaGlobal, item).call(t);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callCharItem(char cdata, char &olddata, string item, bool &implControl) {
	if (cdata != olddata)	{

		callOffsetMap(item, &cdata, NULL, writeSB);

		if (implControl)	{
			try	{
				olddata = cdata;
				LuaCall<NullT, char>(LuaGlobal, item).call(cdata);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callFloatItem(float fdata, float &olddata, string item, bool &implControl) {
	if (fdata != olddata)	{

		callOffsetMap(item, &fdata, NULL, writeFLT);

		if (implControl)	{
			try	{
				olddata = fdata;
				LuaCall<NullT, float>(LuaGlobal, item).call(fdata);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callShortItem(short shdata, short &olddata, string item, bool &implControl) {
	if (shdata != olddata)	{

		callOffsetMap(item, &shdata, NULL, writeSW);

		if (implControl)	{
			try	{
				olddata = shdata;
				LuaCall<NullT, short>(LuaGlobal, item).call(shdata);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callUnsignedIntegerItem(unsigned int uidata, unsigned int &olddata, string item, bool &implControl) {
	if (uidata != olddata)	{

		callOffsetMap(item, &uidata, NULL, writeUD);

		if (implControl)	{
			try	{
				olddata = uidata;
				LuaCall<NullT, int>(LuaGlobal, item).call(uidata);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

void callUnsignedShortItem(unsigned short usdata, unsigned short &olddata, string item, bool &implControl) {
	if (usdata != olddata)	{

		callOffsetMap(item, &usdata, NULL, writeUW);

		if (implControl)	{
			try	{
				olddata = usdata;
				int t = usdata;
				LuaCall<NullT, int>(LuaGlobal, item).call(t);
			} catch (std::string errMsg) {
				printLog("Item ignored (" + item + ") - " + errMsg);
				
				if (strcmp("attempt to call a nil value", errMsg.c_str()) == 0) {
					implControl = false;
				}
			}
		}
	}
}

// This function is called when NGX data changes
void ProcessNGXData (PMDG_NGX_Data *pS)
{
	// Aft overhead
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// ADIRU
	
	clock_t t1=clock();

	if (bEventCallbacks) {

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		callUnsignedCharItem(pS->IRS_DisplaySelector, pDBAK->IRS_DisplaySelector, "IRS_DisplaySelector", bIRS_DisplaySelector);
		callUnsignedCharItem(pS->IRS_ModeSelector[0], pDBAK->IRS_ModeSelector[0], "IRS_ModeSelector_0", bIRS_ModeSelector_0);
		callUnsignedCharItem(pS->IRS_ModeSelector[1], pDBAK->IRS_ModeSelector[1], "IRS_ModeSelector_1", bIRS_ModeSelector_1);
		callUnsignedCharItem(pS->LTS_DomeWhiteSw, pDBAK->LTS_DomeWhiteSw, "LTS_DomeWhiteSw", bLTS_DomeWhiteSw);
		callUnsignedCharItem(pS->OXY_Needle, pDBAK->OXY_Needle, "OXY_Needle", bOXY_Needle);
		callUnsignedCharItem(pS->FCTL_FltControl_Sw[0], pDBAK->FCTL_FltControl_Sw[0], "FCTL_FltControl_Sw_0", bFCTL_FltControl_Sw_0);
		callUnsignedCharItem(pS->FCTL_FltControl_Sw[1], pDBAK->FCTL_FltControl_Sw[1], "FCTL_FltControl_Sw_1", bFCTL_FltControl_Sw_1);
		callUnsignedCharItem(pS->FCTL_AltnFlaps_Control_Sw, pDBAK->FCTL_AltnFlaps_Control_Sw, "FCTL_AltnFlaps_Control_Sw", bFCTL_AltnFlaps_Control_Sw);
		callUnsignedCharItem(pS->NAVDIS_VHFNavSelector, pDBAK->NAVDIS_VHFNavSelector, "NAVDIS_VHFNavSelector", bNAVDIS_VHFNavSelector);
		callUnsignedCharItem(pS->NAVDIS_IRSSelector, pDBAK->NAVDIS_IRSSelector, "NAVDIS_IRSSelector", bNAVDIS_IRSSelector);
		callUnsignedCharItem(pS->NAVDIS_FMCSelector, pDBAK->NAVDIS_FMCSelector, "NAVDIS_FMCSelector", bNAVDIS_FMCSelector);
		callUnsignedCharItem(pS->NAVDIS_SourceSelector, pDBAK->NAVDIS_SourceSelector, "NAVDIS_SourceSelector", bNAVDIS_SourceSelector);
		callUnsignedCharItem(pS->NAVDIS_ControlPaneSelector, pDBAK->NAVDIS_ControlPaneSelector, "NAVDIS_ControlPaneSelector", bNAVDIS_ControlPaneSelector);
		callUnsignedCharItem(pS->ELEC_DCMeterSelector, pDBAK->ELEC_DCMeterSelector, "ELEC_DCMeterSelector", bELEC_DCMeterSelector);
		callUnsignedCharItem(pS->ELEC_ACMeterSelector, pDBAK->ELEC_ACMeterSelector, "ELEC_ACMeterSelector", bELEC_ACMeterSelector);
		callUnsignedCharItem(pS->ELEC_BatSelector, pDBAK->ELEC_BatSelector, "ELEC_BatSelector", bELEC_BatSelector);
		callUnsignedCharItem(pS->ELEC_StandbyPowerSelector, pDBAK->ELEC_StandbyPowerSelector, "ELEC_StandbyPowerSelector", bELEC_StandbyPowerSelector);
		callUnsignedCharItem(pS->OH_WiperLSelector, pDBAK->OH_WiperLSelector, "OH_WiperLSelector", bOH_WiperLSelector);
		callUnsignedCharItem(pS->OH_WiperRSelector, pDBAK->OH_WiperRSelector, "OH_WiperRSelector", bOH_WiperRSelector);
		callUnsignedCharItem(pS->LTS_CircuitBreakerKnob, pDBAK->LTS_CircuitBreakerKnob, "LTS_CircuitBreakerKnob", bLTS_CircuitBreakerKnob);
		callUnsignedCharItem(pS->LTS_OvereadPanelKnob, pDBAK->LTS_OvereadPanelKnob, "LTS_OvereadPanelKnob", bLTS_OvereadPanelKnob);
		callUnsignedCharItem(pS->LTS_EmerExitSelector, pDBAK->LTS_EmerExitSelector, "LTS_EmerExitSelector", bLTS_EmerExitSelector);
		callUnsignedCharItem(pS->COMM_NoSmokingSelector, pDBAK->COMM_NoSmokingSelector, "COMM_NoSmokingSelector", bCOMM_NoSmokingSelector);
		callUnsignedCharItem(pS->COMM_FastenBeltsSelector, pDBAK->COMM_FastenBeltsSelector, "COMM_FastenBeltsSelector", bCOMM_FastenBeltsSelector);
		callUnsignedCharItem(pS->AIR_TempSourceSelector, pDBAK->AIR_TempSourceSelector, "AIR_TempSourceSelector", bAIR_TempSourceSelector);
		callUnsignedCharItem(pS->LTS_LandingLtRetractableSw[0], pDBAK->LTS_LandingLtRetractableSw[0], "LTS_LandingLtRetractableSw_0", bLTS_LandingLtRetractableSw_0);
		callUnsignedCharItem(pS->LTS_LandingLtRetractableSw[1], pDBAK->LTS_LandingLtRetractableSw[1], "LTS_LandingLtRetractableSw_1", bLTS_LandingLtRetractableSw_1);
		callUnsignedCharItem(pS->APU_Selector, pDBAK->APU_Selector, "APU_Selector", bAPU_Selector);
		callUnsignedCharItem(pS->ENG_StartSelector[0], pDBAK->ENG_StartSelector[0], "ENG_StartSelector_0", bENG_StartSelector_1);
		callUnsignedCharItem(pS->ENG_StartSelector[1], pDBAK->ENG_StartSelector[1], "ENG_StartSelector_1", bENG_StartSelector_1);
		callUnsignedCharItem(pS->ENG_IgnitionSelector, pDBAK->ENG_IgnitionSelector, "ENG_IgnitionSelector", bENG_IgnitionSelector);
		callUnsignedCharItem(pS->LTS_PositionSw, pDBAK->LTS_PositionSw, "LTS_PositionSw", bLTS_PositionSw);
		callUnsignedCharItem(pS->EFIS_VORADFSel1[0], pDBAK->EFIS_VORADFSel1[0], "EFIS_VORADFSel1_0", bEFIS_VORADFSel1_0);
		callUnsignedCharItem(pS->EFIS_VORADFSel1[1], pDBAK->EFIS_VORADFSel1[1], "EFIS_VORADFSel1_1", bEFIS_VORADFSel1_1);
		callUnsignedCharItem(pS->EFIS_VORADFSel2[0], pDBAK->EFIS_VORADFSel2[0], "EFIS_VORADFSel2_0", bEFIS_VORADFSel2_0);
		callUnsignedCharItem(pS->EFIS_ModeSel[0], pDBAK->EFIS_ModeSel[0], "EFIS_ModeSel_0", bEFIS_ModeSel_0);
		callUnsignedCharItem(pS->EFIS_RangeSel[0], pDBAK->EFIS_RangeSel[0], "EFIS_RangeSel_0", bEFIS_RangeSel_0);
		callUnsignedCharItem(pS->EFIS_VORADFSel2[1], pDBAK->EFIS_VORADFSel2[1], "EFIS_VORADFSel2_1", bEFIS_VORADFSel2_1);
		callUnsignedCharItem(pS->EFIS_ModeSel[1], pDBAK->EFIS_ModeSel[1], "EFIS_ModeSel_1", bEFIS_ModeSel_1);
		callUnsignedCharItem(pS->EFIS_RangeSel[1], pDBAK->EFIS_RangeSel[1], "EFIS_RangeSel_1", bEFIS_RangeSel_1);
		callUnsignedCharItem(pS->MCP_BankLimitSel, pDBAK->MCP_BankLimitSel, "MCP_BankLimitSel", bMCP_BankLimitSel);
		callUnsignedCharItem(pS->MAIN_MainPanelDUSel[0], pDBAK->MAIN_MainPanelDUSel[0], "MAIN_MainPanelDUSel_0", bMAIN_MainPanelDUSel_0);
		callUnsignedCharItem(pS->MAIN_LowerDUSel[0], pDBAK->MAIN_LowerDUSel[0], "MAIN_LowerDUSel_0", bMAIN_LowerDUSel_0);
		callUnsignedCharItem(pS->MAIN_DisengageTestSelector[0], pDBAK->MAIN_DisengageTestSelector[0], "MAIN_DisengageTestSelector_0", bMAIN_DisengageTestSelector_0);
		callUnsignedCharItem(pS->MAIN_MainPanelDUSel[1], pDBAK->MAIN_MainPanelDUSel[1], "MAIN_MainPanelDUSel_1", bMAIN_MainPanelDUSel_1);
		callUnsignedCharItem(pS->MAIN_LowerDUSel[1], pDBAK->MAIN_LowerDUSel[1], "MAIN_LowerDUSel_1", bMAIN_LowerDUSel_1);
		callUnsignedCharItem(pS->MAIN_DisengageTestSelector[1], pDBAK->MAIN_DisengageTestSelector[1], "MAIN_DisengageTestSelector_1", bMAIN_DisengageTestSelector_1);
		callUnsignedCharItem(pS->MAIN_LightsSelector, pDBAK->MAIN_LightsSelector, "MAIN_LightsSelector", bMAIN_LightsSelector);
		callUnsignedCharItem(pS->MAIN_N1SetSelector, pDBAK->MAIN_N1SetSelector, "MAIN_N1SetSelector", bMAIN_N1SetSelector);
		callUnsignedCharItem(pS->MAIN_SpdRefSelector, pDBAK->MAIN_SpdRefSelector, "MAIN_SpdRefSelector", bMAIN_SpdRefSelector);
		callUnsignedCharItem(pS->MAIN_FuelFlowSelector, pDBAK->MAIN_FuelFlowSelector, "MAIN_FuelFlowSelector", bMAIN_FuelFlowSelector);
		callUnsignedCharItem(pS->MAIN_AutobrakeSelector, pDBAK->MAIN_AutobrakeSelector, "MAIN_AutobrakeSelector", bMAIN_AutobrakeSelector);
		callUnsignedCharItem(pS->MAIN_GearLever, pDBAK->MAIN_GearLever, "MAIN_GearLever", bMAIN_GearLever);
		callUnsignedCharItem(pS->LTS_MainPanelKnob[0], pDBAK->LTS_MainPanelKnob[0], "LTS_MainPanelKnob_0", bLTS_MainPanelKnob_0);
		callUnsignedCharItem(pS->LTS_MainPanelKnob[1], pDBAK->LTS_MainPanelKnob[1], "LTS_MainPanelKnob_1", bLTS_MainPanelKnob_1);
		callUnsignedCharItem(pS->LTS_BackgroundKnob, pDBAK->LTS_BackgroundKnob, "LTS_BackgroundKnob", bLTS_BackgroundKnob);
		callUnsignedCharItem(pS->LTS_AFDSFloodKnob, pDBAK->LTS_AFDSFloodKnob, "LTS_AFDSFloodKnob", bLTS_AFDSFloodKnob);
		callUnsignedCharItem(pS->LTS_OutbdDUBrtKnob[0], pDBAK->LTS_OutbdDUBrtKnob[0], "LTS_OutbdDUBrtKnob_0", bLTS_OutbdDUBrtKnob_0);
		callUnsignedCharItem(pS->LTS_InbdDUBrtKnob[0], pDBAK->LTS_InbdDUBrtKnob[0], "LTS_InbdDUBrtKnob_0", bLTS_InbdDUBrtKnob_0);
		callUnsignedCharItem(pS->LTS_InbdDUMapBrtKnob[0], pDBAK->LTS_InbdDUMapBrtKnob[0], "LTS_InbdDUMapBrtKnob_0", bLTS_InbdDUMapBrtKnob_0);
		callUnsignedCharItem(pS->LTS_OutbdDUBrtKnob[1], pDBAK->LTS_OutbdDUBrtKnob[1], "LTS_OutbdDUBrtKnob_1", bLTS_OutbdDUBrtKnob_1);
		callUnsignedCharItem(pS->LTS_InbdDUBrtKnob[1], pDBAK->LTS_InbdDUBrtKnob[1], "LTS_InbdDUBrtKnob_1", bLTS_InbdDUBrtKnob_1);
		callUnsignedCharItem(pS->LTS_InbdDUMapBrtKnob[1], pDBAK->LTS_InbdDUMapBrtKnob[1], "LTS_InbdDUMapBrtKnob_1", bLTS_InbdDUMapBrtKnob_1);
		callUnsignedCharItem(pS->LTS_UpperDUBrtKnob, pDBAK->LTS_UpperDUBrtKnob, "LTS_UpperDUBrtKnob", bLTS_UpperDUBrtKnob);
		callUnsignedCharItem(pS->LTS_LowerDUBrtKnob, pDBAK->LTS_LowerDUBrtKnob, "LTS_LowerDUBrtKnob", bLTS_LowerDUBrtKnob);
		callUnsignedCharItem(pS->LTS_LowerDUMapBrtKnob, pDBAK->LTS_LowerDUMapBrtKnob, "LTS_LowerDUMapBrtKnob", bLTS_LowerDUMapBrtKnob);
		callUnsignedCharItem(pS->CDU_BrtKnob[0], pDBAK->CDU_BrtKnob[0], "CDU_BrtKnob_0", bCDU_BrtKnob_0);
		callUnsignedCharItem(pS->FIRE_OvhtDetSw[0], pDBAK->FIRE_OvhtDetSw[0], "FIRE_OvhtDetSw_0", bFIRE_OvhtDetSw_0);
		callUnsignedCharItem(pS->CDU_BrtKnob[1], pDBAK->CDU_BrtKnob[1], "CDU_BrtKnob_1", bCDU_BrtKnob_1);
		callUnsignedCharItem(pS->FIRE_OvhtDetSw[1], pDBAK->FIRE_OvhtDetSw[1], "FIRE_OvhtDetSw_1", bFIRE_OvhtDetSw_1);
		callUnsignedCharItem(pS->FIRE_DetTestSw, pDBAK->FIRE_DetTestSw, "FIRE_DetTestSw", bFIRE_DetTestSw);
		callUnsignedCharItem(pS->FIRE_HandlePos[0], pDBAK->FIRE_HandlePos[0], "FIRE_HandlePos_0", bFIRE_HandlePos_0);
		callUnsignedCharItem(pS->FIRE_HandlePos[1], pDBAK->FIRE_HandlePos[1], "FIRE_HandlePos_1", bFIRE_HandlePos_1);
		callUnsignedCharItem(pS->FIRE_HandlePos[2], pDBAK->FIRE_HandlePos[2], "FIRE_HandlePos_2", bFIRE_HandlePos_2);
		callUnsignedCharItem(pS->FIRE_ExtinguisherTestSw, pDBAK->FIRE_ExtinguisherTestSw, "FIRE_ExtinguisherTestSw", bFIRE_ExtinguisherTestSw);
		callUnsignedCharItem(pS->CARGO_DetSelect[0], pDBAK->CARGO_DetSelect[0], "CARGO_DetSelect_0", bCARGO_DetSelect_0);
		callUnsignedCharItem(pS->CARGO_DetSelect[1], pDBAK->CARGO_DetSelect[1], "CARGO_DetSelect_1", bCARGO_DetSelect_1);
		callUnsignedCharItem(pS->XPDR_ModeSel, pDBAK->XPDR_ModeSel, "XPDR_ModeSel", bXPDR_ModeSel);
		callUnsignedCharItem(pS->LTS_PedFloodKnob, pDBAK->LTS_PedFloodKnob, "LTS_PedFloodKnob", bLTS_PedFloodKnob);
		callUnsignedCharItem(pS->LTS_PedPanelKnob, pDBAK->LTS_PedPanelKnob, "LTS_PedPanelKnob", bLTS_PedPanelKnob);
		callUnsignedCharItem(pS->PED_FltDkDoorSel, pDBAK->PED_FltDkDoorSel, "PED_FltDkDoorSel", bPED_FltDkDoorSel);
		callUnsignedCharItem(pS->FMC_TakeoffFlaps, pDBAK->FMC_TakeoffFlaps, "FMC_TakeoffFlaps", bFMC_TakeoffFlaps);
		callUnsignedCharItem(pS->FMC_V1, pDBAK->FMC_V1, "FMC_V1", bFMC_V1);
		callUnsignedCharItem(pS->FMC_VR, pDBAK->FMC_VR, "FMC_VR", bFMC_VR);
		callUnsignedCharItem(pS->FMC_V2, pDBAK->FMC_V2, "FMC_V2", bFMC_V2);
		callUnsignedCharItem(pS->FMC_LandingFlaps, pDBAK->FMC_LandingFlaps, "FMC_LandingFlaps", bFMC_LandingFlaps);
		callUnsignedCharItem(pS->FMC_LandingVREF, pDBAK->FMC_LandingVREF, "FMC_LandingVREF", bFMC_LandingVREF);


		callBooleanItem(pS->IRS_SysDisplay_R, pDBAK->IRS_SysDisplay_R, "IRS_SysDisplay_R", bIRS_SysDisplay_R);
		callBooleanItem(pS->IRS_annunGPS, pDBAK->IRS_annunGPS, "IRS_annunGPS", bIRS_annunGPS);
		callBooleanItem(pS->IRS_annunALIGN[0], pDBAK->IRS_annunALIGN[0], "IRS_annunALIGN_0", bIRS_annunALIGN_0);
		callBooleanItem(pS->IRS_annunON_DC[0], pDBAK->IRS_annunON_DC[0], "IRS_annunON_DC_0", bIRS_annunON_DC_0);
		callBooleanItem(pS->IRS_annunFAULT[0], pDBAK->IRS_annunFAULT[0], "IRS_annunFAULT_0", bIRS_annunFAULT_0);
		callBooleanItem(pS->IRS_annunDC_FAIL[0], pDBAK->IRS_annunDC_FAIL[0], "IRS_annunDC_FAIL_0", bIRS_annunDC_FAIL_0);

		callBooleanItem(pS->IRS_annunALIGN[1], pDBAK->IRS_annunALIGN[1], "IRS_annunALIGN_1", bIRS_annunALIGN_1);
		callBooleanItem(pS->IRS_annunON_DC[1], pDBAK->IRS_annunON_DC[1], "IRS_annunON_DC_1", bIRS_annunON_DC_1);
		callBooleanItem(pS->IRS_annunFAULT[1], pDBAK->IRS_annunFAULT[1], "IRS_annunFAULT_1", bIRS_annunFAULT_1);
		callBooleanItem(pS->IRS_annunDC_FAIL[1], pDBAK->IRS_annunDC_FAIL[1], "IRS_annunDC_FAIL_1", bIRS_annunDC_FAIL_1);
		callBooleanItem(pS->WARN_annunPSEU, pDBAK->WARN_annunPSEU, "WARN_annunPSEU", bWARN_annunPSEU);
		callBooleanItem(pS->COMM_ServiceInterphoneSw, pDBAK->COMM_ServiceInterphoneSw, "COMM_ServiceInterphoneSw", bCOMM_ServiceInterphoneSw);
		
		callBooleanItem(pS->ENG_EECSwitch[0], pDBAK->ENG_EECSwitch[0], "ENG_EECSwitch_0", bENG_EECSwitch_0);
		callBooleanItem(pS->ENG_annunREVERSER[0], pDBAK->ENG_annunREVERSER[0], "ENG_annunREVERSER_0", bENG_annunREVERSER_0);
		callBooleanItem(pS->ENG_annunENGINE_CONTROL[0], pDBAK->ENG_annunENGINE_CONTROL[0], "ENG_annunENGINE_CONTROL_0", bENG_annunENGINE_CONTROL_0);
		callBooleanItem(pS->ENG_annunALTN[0], pDBAK->ENG_annunALTN[0], "ENG_annunALTN_0", bENG_annunALTN_0);

		callBooleanItem(pS->ENG_EECSwitch[1], pDBAK->ENG_EECSwitch[1], "ENG_EECSwitch_1", bENG_EECSwitch_1);
		callBooleanItem(pS->ENG_annunREVERSER[1], pDBAK->ENG_annunREVERSER[1], "ENG_annunREVERSER_1", bENG_annunREVERSER_1);
		callBooleanItem(pS->ENG_annunENGINE_CONTROL[1], pDBAK->ENG_annunENGINE_CONTROL[1], "ENG_annunENGINE_CONTROL_1", bENG_annunENGINE_CONTROL_1);
		callBooleanItem(pS->ENG_annunALTN[1], pDBAK->ENG_annunALTN[1], "ENG_annunALTN_1", bENG_annunALTN_1);

		callBooleanItem(pS->OXY_SwNormal, pDBAK->OXY_SwNormal, "OXY_SwNormal", bOXY_SwNormal);
		callBooleanItem(pS->OXY_annunPASS_OXY_ON, pDBAK->OXY_annunPASS_OXY_ON, "OXY_annunPASS_OXY_ON", bOXY_annunPASS_OXY_ON);
		callBooleanItem(pS->GEAR_annunOvhdLEFT, pDBAK->GEAR_annunOvhdLEFT, "GEAR_annunOvhdLEFT", bGEAR_annunOvhdLEFT);
		callBooleanItem(pS->GEAR_annunOvhdNOSE, pDBAK->GEAR_annunOvhdNOSE, "GEAR_annunOvhdNOSE", bGEAR_annunOvhdNOSE);
		callBooleanItem(pS->GEAR_annunOvhdRIGHT, pDBAK->GEAR_annunOvhdRIGHT, "GEAR_annunOvhdRIGHT", bGEAR_annunOvhdRIGHT);
		callBooleanItem(pS->FLTREC_SwNormal, pDBAK->FLTREC_SwNormal, "FLTREC_SwNormal", bFLTREC_SwNormal);
		callBooleanItem(pS->FLTREC_annunOFF, pDBAK->FLTREC_annunOFF, "FLTREC_annunOFF", bFLTREC_annunOFF);
		callBooleanItem(pS->FCTL_Spoiler_Sw[0], pDBAK->FCTL_Spoiler_Sw[0], "FCTL_Spoiler_Sw_0", bFCTL_Spoiler_Sw_0);
		callBooleanItem(pS->FCTL_Spoiler_Sw[1], pDBAK->FCTL_Spoiler_Sw[1], "FCTL_Spoiler_Sw_1", bFCTL_Spoiler_Sw_1);
		callBooleanItem(pS->FCTL_YawDamper_Sw, pDBAK->FCTL_YawDamper_Sw, "FCTL_YawDamper_Sw", bFCTL_YawDamper_Sw);
		callBooleanItem(pS->FCTL_AltnFlaps_Sw_ARM, pDBAK->FCTL_AltnFlaps_Sw_ARM, "FCTL_AltnFlaps_Sw_ARM", bFCTL_AltnFlaps_Sw_ARM);
		callBooleanItem(pS->FCTL_annunFC_LOW_PRESSURE[0], pDBAK->FCTL_annunFC_LOW_PRESSURE[0], "FCTL_annunFC_LOW_PRESSURE_0", bFCTL_annunFC_LOW_PRESSURE_0);
		callBooleanItem(pS->FCTL_annunFC_LOW_PRESSURE[1], pDBAK->FCTL_annunFC_LOW_PRESSURE[1], "FCTL_annunFC_LOW_PRESSURE_1", bFCTL_annunFC_LOW_PRESSURE_0);
		callBooleanItem(pS->FCTL_annunYAW_DAMPER, pDBAK->FCTL_annunYAW_DAMPER, "FCTL_annunYAW_DAMPER", bFCTL_annunYAW_DAMPER);
		callBooleanItem(pS->FCTL_annunLOW_QUANTITY, pDBAK->FCTL_annunLOW_QUANTITY, "FCTL_annunLOW_QUANTITY", bFCTL_annunLOW_QUANTITY);
		callBooleanItem(pS->FCTL_annunLOW_PRESSURE, pDBAK->FCTL_annunLOW_PRESSURE, "FCTL_annunLOW_PRESSURE", bFCTL_annunLOW_PRESSURE);
		callBooleanItem(pS->FCTL_annunLOW_STBY_RUD_ON, pDBAK->FCTL_annunLOW_STBY_RUD_ON, "FCTL_annunLOW_STBY_RUD_ON", bFCTL_annunLOW_STBY_RUD_ON);
		callBooleanItem(pS->FCTL_annunFEEL_DIFF_PRESS, pDBAK->FCTL_annunFEEL_DIFF_PRESS, "FCTL_annunFEEL_DIFF_PRESS", bFCTL_annunFEEL_DIFF_PRESS);
		callBooleanItem(pS->FCTL_annunSPEED_TRIM_FAIL, pDBAK->FCTL_annunSPEED_TRIM_FAIL, "FCTL_annunSPEED_TRIM_FAIL", bFCTL_annunSPEED_TRIM_FAIL);
		callBooleanItem(pS->FCTL_annunMACH_TRIM_FAIL, pDBAK->FCTL_annunMACH_TRIM_FAIL, "FCTL_annunMACH_TRIM_FAIL", bFCTL_annunMACH_TRIM_FAIL);
		callBooleanItem(pS->FCTL_annunAUTO_SLAT_FAIL, pDBAK->FCTL_annunAUTO_SLAT_FAIL, "FCTL_annunAUTO_SLAT_FAIL", bFCTL_annunAUTO_SLAT_FAIL);
		callBooleanItem(pS->FUEL_CrossFeedSw, pDBAK->FUEL_CrossFeedSw, "FUEL_CrossFeedSw", bFUEL_CrossFeedSw);
		callBooleanItem(pS->FUEL_PumpFwdSw[0], pDBAK->FUEL_PumpFwdSw[0], "FUEL_PumpFwdSw_0", bFUEL_PumpFwdSw_0);
		callBooleanItem(pS->FUEL_PumpAftSw[0], pDBAK->FUEL_PumpAftSw[0], "FUEL_PumpAftSw_0", bFUEL_PumpAftSw_0);
		callBooleanItem(pS->FUEL_PumpCtrSw[0], pDBAK->FUEL_PumpCtrSw[0], "FUEL_PumpCtrSw_0", bFUEL_PumpCtrSw_0);

		callBooleanItem(pS->FUEL_PumpFwdSw[1], pDBAK->FUEL_PumpFwdSw[1], "FUEL_PumpFwdSw_1", bFUEL_PumpFwdSw_1);
		callBooleanItem(pS->FUEL_PumpAftSw[1], pDBAK->FUEL_PumpAftSw[1], "FUEL_PumpAftSw_1", bFUEL_PumpAftSw_1);
		callBooleanItem(pS->FUEL_PumpCtrSw[1], pDBAK->FUEL_PumpCtrSw[1], "FUEL_PumpCtrSw_1", bFUEL_PumpCtrSw_1);

		callBooleanItem(pS->FUEL_annunENG_VALVE_CLOSED[0], pDBAK->FUEL_annunENG_VALVE_CLOSED[0], "FUEL_annunENG_VALVE_CLOSED_0", bFUEL_annunENG_VALVE_CLOSED_0);
		callBooleanItem(pS->FUEL_annunSPAR_VALVE_CLOSED[0], pDBAK->FUEL_annunSPAR_VALVE_CLOSED[0], "FUEL_annunSPAR_VALVE_CLOSED_0", bFUEL_annunSPAR_VALVE_CLOSED_0);
		callBooleanItem(pS->FUEL_annunFILTER_BYPASS[0], pDBAK->FUEL_annunFILTER_BYPASS[0], "FUEL_annunFILTER_BYPASS_0", bFUEL_annunFILTER_BYPASS_0);
		
		callBooleanItem(pS->FUEL_annunENG_VALVE_CLOSED[1], pDBAK->FUEL_annunENG_VALVE_CLOSED[1], "FUEL_annunENG_VALVE_CLOSED_1", bFUEL_annunENG_VALVE_CLOSED_1);
		callBooleanItem(pS->FUEL_annunSPAR_VALVE_CLOSED[1], pDBAK->FUEL_annunSPAR_VALVE_CLOSED[1], "FUEL_annunSPAR_VALVE_CLOSED_1", bFUEL_annunSPAR_VALVE_CLOSED_1);
		callBooleanItem(pS->FUEL_annunFILTER_BYPASS[1], pDBAK->FUEL_annunFILTER_BYPASS[1], "FUEL_annunFILTER_BYPASS_1", bFUEL_annunFILTER_BYPASS_1);

		callBooleanItem(pS->FUEL_annunXFEED_VALVE_OPEN, pDBAK->FUEL_annunXFEED_VALVE_OPEN, "FUEL_annunXFEED_VALVE_OPEN", bFUEL_annunXFEED_VALVE_OPEN);
		callBooleanItem(pS->FUEL_annunLOWPRESS_Fwd[0], pDBAK->FUEL_annunLOWPRESS_Fwd[0], "FUEL_annunLOWPRESS_Fwd_0", bFUEL_annunLOWPRESS_Fwd_0);
		callBooleanItem(pS->FUEL_annunLOWPRESS_Aft[0], pDBAK->FUEL_annunLOWPRESS_Aft[0], "FUEL_annunLOWPRESS_Aft_0", bFUEL_annunLOWPRESS_Aft_0);
		callBooleanItem(pS->FUEL_annunLOWPRESS_Ctr[0], pDBAK->FUEL_annunLOWPRESS_Ctr[0], "FUEL_annunLOWPRESS_Ctr_0", bFUEL_annunLOWPRESS_Ctr_0);

		callBooleanItem(pS->FUEL_annunLOWPRESS_Fwd[1], pDBAK->FUEL_annunLOWPRESS_Fwd[1], "FUEL_annunLOWPRESS_Fwd_1", bFUEL_annunLOWPRESS_Fwd_1);
		callBooleanItem(pS->FUEL_annunLOWPRESS_Aft[1], pDBAK->FUEL_annunLOWPRESS_Aft[1], "FUEL_annunLOWPRESS_Aft_1", bFUEL_annunLOWPRESS_Aft_1);
		callBooleanItem(pS->FUEL_annunLOWPRESS_Ctr[1], pDBAK->FUEL_annunLOWPRESS_Ctr[1], "FUEL_annunLOWPRESS_Ctr_1", bFUEL_annunLOWPRESS_Ctr_1);

		callBooleanItem(pS->ELEC_annunBAT_DISCHARGE, pDBAK->ELEC_annunBAT_DISCHARGE, "ELEC_annunBAT_DISCHARGE", bELEC_annunBAT_DISCHARGE);
		callBooleanItem(pS->ELEC_annunTR_UNIT, pDBAK->ELEC_annunTR_UNIT, "ELEC_annunTR_UNIT", bELEC_annunTR_UNIT);
		callBooleanItem(pS->ELEC_annunELEC, pDBAK->ELEC_annunELEC, "ELEC_annunELEC", bELEC_annunELEC);
		callBooleanItem(pS->ELEC_CabUtilSw, pDBAK->ELEC_CabUtilSw, "ELEC_CabUtilSw", bELEC_CabUtilSw);
		callBooleanItem(pS->ELEC_IFEPassSeatSw, pDBAK->ELEC_IFEPassSeatSw, "ELEC_IFEPassSeatSw", bELEC_IFEPassSeatSw);
		callBooleanItem(pS->ELEC_annunDRIVE[0], pDBAK->ELEC_annunDRIVE[0], "ELEC_annunDRIVE_0", bELEC_annunDRIVE_0);
		callBooleanItem(pS->ELEC_annunDRIVE[1], pDBAK->ELEC_annunDRIVE[1], "ELEC_annunDRIVE_1", bELEC_annunDRIVE_1);
		callBooleanItem(pS->ELEC_annunSTANDBY_POWER_OFF, pDBAK->ELEC_annunSTANDBY_POWER_OFF, "ELEC_annunSTANDBY_POWER_OFF", bELEC_annunSTANDBY_POWER_OFF);
		callBooleanItem(pS->ELEC_IDGDisconnectSw[0], pDBAK->ELEC_IDGDisconnectSw[0], "ELEC_IDGDisconnectSw_0", bELEC_IDGDisconnectSw_0);
		callBooleanItem(pS->ELEC_IDGDisconnectSw[1], pDBAK->ELEC_IDGDisconnectSw[1], "ELEC_IDGDisconnectSw_1", bELEC_IDGDisconnectSw_1);
		callBooleanItem(pS->ELEC_annunGRD_POWER_AVAILABLE, pDBAK->ELEC_annunGRD_POWER_AVAILABLE, "ELEC_annunGRD_POWER_AVAILABLE", bELEC_annunGRD_POWER_AVAILABLE);
		callBooleanItem(pS->ELEC_GrdPwrSw, pDBAK->ELEC_GrdPwrSw, "ELEC_GrdPwrSw", bELEC_GrdPwrSw);
		callBooleanItem(pS->ELEC_BusTransSw_AUTO, pDBAK->ELEC_BusTransSw_AUTO, "ELEC_BusTransSw_AUTO", bELEC_BusTransSw_AUTO);
		
		callBooleanItem(pS->ELEC_GenSw[0], pDBAK->ELEC_GenSw[0], "ELEC_GenSw_0", bELEC_GenSw_0);
		callBooleanItem(pS->ELEC_APUGenSw[0], pDBAK->ELEC_APUGenSw[0], "ELEC_APUGenSw_0", bELEC_APUGenSw_0);
		callBooleanItem(pS->ELEC_annunTRANSFER_BUS_OFF[0], pDBAK->ELEC_annunTRANSFER_BUS_OFF[0], "ELEC_annunTRANSFER_BUS_OFF_0", bELEC_annunTRANSFER_BUS_OFF_0);
		callBooleanItem(pS->ELEC_annunSOURCE_OFF[0], pDBAK->ELEC_annunSOURCE_OFF[0], "ELEC_annunSOURCE_OFF_0", bELEC_annunSOURCE_OFF_0);
		callBooleanItem(pS->ELEC_annunGEN_BUS_OFF[0], pDBAK->ELEC_annunGEN_BUS_OFF[0], "ELEC_annunGEN_BUS_OFF_0", bELEC_annunGEN_BUS_OFF_0);

		callBooleanItem(pS->ELEC_GenSw[1], pDBAK->ELEC_GenSw[1], "ELEC_GenSw_1", bELEC_GenSw_1);
		callBooleanItem(pS->ELEC_APUGenSw[1], pDBAK->ELEC_APUGenSw[1], "ELEC_APUGenSw_1", bELEC_APUGenSw_1);
		callBooleanItem(pS->ELEC_annunTRANSFER_BUS_OFF[1], pDBAK->ELEC_annunTRANSFER_BUS_OFF[1], "ELEC_annunTRANSFER_BUS_OFF_1", bELEC_annunTRANSFER_BUS_OFF_1);
		callBooleanItem(pS->ELEC_annunSOURCE_OFF[1], pDBAK->ELEC_annunSOURCE_OFF[1], "ELEC_annunSOURCE_OFF_1", bELEC_annunSOURCE_OFF_1);
		callBooleanItem(pS->ELEC_annunGEN_BUS_OFF[1], pDBAK->ELEC_annunGEN_BUS_OFF[1], "ELEC_annunGEN_BUS_OFF_1", bELEC_annunGEN_BUS_OFF_1);

		callBooleanItem(pS->ELEC_annunAPU_GEN_OFF_BUS, pDBAK->ELEC_annunAPU_GEN_OFF_BUS, "ELEC_annunAPU_GEN_OFF_BUS", bELEC_annunAPU_GEN_OFF_BUS);
		callBooleanItem(pS->APU_annunMAINT, pDBAK->APU_annunMAINT, "APU_annunMAINT", bAPU_annunMAINT);
		callBooleanItem(pS->APU_annunLOW_OIL_PRESSURE, pDBAK->APU_annunLOW_OIL_PRESSURE, "APU_annunLOW_OIL_PRESSURE", bAPU_annunLOW_OIL_PRESSURE);
		callBooleanItem(pS->APU_annunFAULT, pDBAK->APU_annunFAULT, "APU_annunFAULT", bAPU_annunFAULT);
		callBooleanItem(pS->APU_annunOVERSPEED, pDBAK->APU_annunOVERSPEED, "APU_annunOVERSPEED", bAPU_annunOVERSPEED);
		callBooleanItem(pS->AIR_EquipCoolingSupplyNORM, pDBAK->AIR_EquipCoolingSupplyNORM, "AIR_EquipCoolingSupplyNORM", bAIR_EquipCoolingSupplyNORM);
		callBooleanItem(pS->AIR_EquipCoolingExhaustNORM, pDBAK->AIR_EquipCoolingExhaustNORM, "AIR_EquipCoolingExhaustNORM", bAIR_EquipCoolingExhaustNORM);
		callBooleanItem(pS->AIR_annunEquipCoolingSupplyOFF, pDBAK->AIR_annunEquipCoolingSupplyOFF, "AIR_annunEquipCoolingSupplyOFF", bAIR_annunEquipCoolingSupplyOFF);
		callBooleanItem(pS->AIR_annunEquipCoolingExhaustOFF, pDBAK->AIR_annunEquipCoolingExhaustOFF, "AIR_annunEquipCoolingExhaustOFF", bAIR_annunEquipCoolingExhaustOFF);
		callBooleanItem(pS->LTS_annunEmerNOT_ARMED, pDBAK->LTS_annunEmerNOT_ARMED, "LTS_annunEmerNOT_ARMED", bLTS_annunEmerNOT_ARMED);
		callBooleanItem(pS->COMM_annunCALL, pDBAK->COMM_annunCALL, "COMM_annunCALL", bCOMM_annunCALL);
		callBooleanItem(pS->COMM_annunPA_IN_USE, pDBAK->COMM_annunPA_IN_USE, "COMM_annunPA_IN_USE", bCOMM_annunPA_IN_USE);
		callBooleanItem(pS->ICE_annunOVERHEAT[0], pDBAK->ICE_annunOVERHEAT[0], "ICE_annunOVERHEAT_0", bICE_annunOVERHEAT_0);
		callBooleanItem(pS->ICE_annunOVERHEAT[1], pDBAK->ICE_annunOVERHEAT[1], "ICE_annunOVERHEAT_1", bICE_annunOVERHEAT_1);
		callBooleanItem(pS->ICE_annunOVERHEAT[2], pDBAK->ICE_annunOVERHEAT[2], "ICE_annunOVERHEAT_2", bICE_annunOVERHEAT_2);
		callBooleanItem(pS->ICE_annunOVERHEAT[3], pDBAK->ICE_annunOVERHEAT[3], "ICE_annunOVERHEAT_3", bICE_annunOVERHEAT_3);

		callBooleanItem(pS->ICE_annunON[0], pDBAK->ICE_annunON[1], "ICE_annunON_0", bICE_annunON_0);
		callBooleanItem(pS->ICE_WindowHeatSw[0], pDBAK->ICE_WindowHeatSw[1], "ICE_WindowHeatSw_0", bICE_WindowHeatSw_0);

		callBooleanItem(pS->ICE_annunON[1], pDBAK->ICE_annunON[1], "ICE_annunON_1", bICE_annunON_1);
		callBooleanItem(pS->ICE_WindowHeatSw[1], pDBAK->ICE_WindowHeatSw[1], "ICE_WindowHeatSw_1", bICE_WindowHeatSw_1);

		callBooleanItem(pS->ICE_annunON[2], pDBAK->ICE_annunON[2], "ICE_annunON_2", bICE_annunON_2);
		callBooleanItem(pS->ICE_WindowHeatSw[2], pDBAK->ICE_WindowHeatSw[2], "ICE_WindowHeatSw_2", bICE_WindowHeatSw_2);

		callBooleanItem(pS->ICE_annunON[3], pDBAK->ICE_annunON[3], "ICE_annunON_3", bICE_annunON_3);
		callBooleanItem(pS->ICE_WindowHeatSw[3], pDBAK->ICE_WindowHeatSw[3], "ICE_WindowHeatSw_3", bICE_WindowHeatSw_3);

		callBooleanItem(pS->ICE_annunCAPT_PITOT, pDBAK->ICE_annunCAPT_PITOT, "ICE_annunCAPT_PITOT", bICE_annunCAPT_PITOT);
		callBooleanItem(pS->ICE_annunL_ELEV_PITOT, pDBAK->ICE_annunL_ELEV_PITOT, "ICE_annunL_ELEV_PITOT", bICE_annunL_ELEV_PITOT);
		callBooleanItem(pS->ICE_annunL_ALPHA_VANE, pDBAK->ICE_annunL_ALPHA_VANE, "ICE_annunL_ALPHA_VANE", bICE_annunL_ALPHA_VANE);
		callBooleanItem(pS->ICE_annunL_TEMP_PROBE, pDBAK->ICE_annunL_TEMP_PROBE, "ICE_annunL_TEMP_PROBE", bICE_annunL_TEMP_PROBE);
		callBooleanItem(pS->ICE_annunFO_PITOT, pDBAK->ICE_annunFO_PITOT, "ICE_annunFO_PITOT", bICE_annunFO_PITOT);
		callBooleanItem(pS->ICE_annunR_ELEV_PITOT, pDBAK->ICE_annunR_ELEV_PITOT, "ICE_annunR_ELEV_PITOT", bICE_annunR_ELEV_PITOT);
		callBooleanItem(pS->ICE_annunR_ALPHA_VANE, pDBAK->ICE_annunR_ALPHA_VANE, "ICE_annunR_ALPHA_VANE", bICE_annunR_ALPHA_VANE);
		callBooleanItem(pS->ICE_annunAUX_PITOT, pDBAK->ICE_annunAUX_PITOT, "ICE_annunAUX_PITOT", bICE_annunAUX_PITOT);
		callBooleanItem(pS->ICE_TestProbeHeatSw[0], pDBAK->ICE_TestProbeHeatSw[0], "ICE_TestProbeHeatSw_0", bICE_TestProbeHeatSw_0);
		callBooleanItem(pS->ICE_annunVALVE_OPEN[0], pDBAK->ICE_annunVALVE_OPEN[0], "ICE_annunVALVE_OPEN_0", bICE_annunVALVE_OPEN_0);
		callBooleanItem(pS->ICE_annunCOWL_ANTI_ICE[0], pDBAK->ICE_annunCOWL_ANTI_ICE[0], "ICE_annunCOWL_ANTI_ICE_0", bICE_annunCOWL_ANTI_ICE_0);
		callBooleanItem(pS->ICE_annunCOWL_VALVE_OPEN[0], pDBAK->ICE_annunCOWL_VALVE_OPEN[0], "ICE_annunCOWL_VALVE_OPEN_0", bICE_annunCOWL_VALVE_OPEN_0);

		callBooleanItem(pS->ICE_TestProbeHeatSw[1], pDBAK->ICE_TestProbeHeatSw[1], "ICE_TestProbeHeatSw_1", bICE_TestProbeHeatSw_1);
		callBooleanItem(pS->ICE_annunVALVE_OPEN[1], pDBAK->ICE_annunVALVE_OPEN[1], "ICE_annunVALVE_OPEN_1", bICE_annunVALVE_OPEN_1);
		callBooleanItem(pS->ICE_annunCOWL_ANTI_ICE[1], pDBAK->ICE_annunCOWL_ANTI_ICE[1], "ICE_annunCOWL_ANTI_ICE_1", bICE_annunCOWL_ANTI_ICE_1);
		callBooleanItem(pS->ICE_annunCOWL_VALVE_OPEN[1], pDBAK->ICE_annunCOWL_VALVE_OPEN[1], "ICE_annunCOWL_VALVE_OPEN_1", bICE_annunCOWL_VALVE_OPEN_1);

		callBooleanItem(pS->ICE_WingAntiIceSw, pDBAK->ICE_WingAntiIceSw, "ICE_WingAntiIceSw", bICE_WingAntiIceSw);
		
		callBooleanItem(pS->ICE_EngAntiIceSw[0], pDBAK->ICE_EngAntiIceSw[0], "ICE_EngAntiIceSw_0", bICE_EngAntiIceSw_0);
		callBooleanItem(pS->HYD_annunLOW_PRESS_eng[0], pDBAK->HYD_annunLOW_PRESS_eng[0], "HYD_annunLOW_PRESS_eng_0", bHYD_annunLOW_PRESS_eng_0);
		callBooleanItem(pS->HYD_annunLOW_PRESS_elec[0], pDBAK->HYD_annunLOW_PRESS_elec[0], "HYD_annunLOW_PRESS_elec_0", bHYD_annunLOW_PRESS_elec_0);
		callBooleanItem(pS->HYD_annunOVERHEAT_elec[0], pDBAK->HYD_annunOVERHEAT_elec[0], "HYD_annunOVERHEAT_elec_0", bHYD_annunOVERHEAT_elec_0);
		callBooleanItem(pS->HYD_PumpSw_eng[0], pDBAK->HYD_PumpSw_eng[0], "HYD_PumpSw_eng_0", bHYD_PumpSw_eng_0);
		callBooleanItem(pS->HYD_PumpSw_elec[0], pDBAK->HYD_PumpSw_elec[0], "HYD_PumpSw_elec_0", bHYD_PumpSw_elec_0);
		
		callBooleanItem(pS->ICE_EngAntiIceSw[1], pDBAK->ICE_EngAntiIceSw[1], "ICE_EngAntiIceSw_1", bICE_EngAntiIceSw_1);
		callBooleanItem(pS->HYD_annunLOW_PRESS_eng[1], pDBAK->HYD_annunLOW_PRESS_eng[1], "HYD_annunLOW_PRESS_eng_1", bHYD_annunLOW_PRESS_eng_1);
		callBooleanItem(pS->HYD_annunLOW_PRESS_elec[1], pDBAK->HYD_annunLOW_PRESS_elec[1], "HYD_annunLOW_PRESS_elec_1", bHYD_annunLOW_PRESS_elec_1);
		callBooleanItem(pS->HYD_annunOVERHEAT_elec[1], pDBAK->HYD_annunOVERHEAT_elec[1], "HYD_annunOVERHEAT_elec_1", bHYD_annunOVERHEAT_elec_1);
		callBooleanItem(pS->HYD_PumpSw_eng[1], pDBAK->HYD_PumpSw_eng[1], "HYD_PumpSw_eng_1", bHYD_PumpSw_eng_1);
		callBooleanItem(pS->HYD_PumpSw_elec[1], pDBAK->HYD_PumpSw_elec[1], "HYD_PumpSw_elec_1", bHYD_PumpSw_elec_1);
		
		callBooleanItem(pS->AIR_TrimAirSwitch, pDBAK->AIR_TrimAirSwitch, "AIR_TrimAirSwitch", bAIR_TrimAirSwitch);
		callBooleanItem(pS->AIR_annunZoneTemp[0], pDBAK->AIR_annunZoneTemp[0], "AIR_annunZoneTemp_0", bAIR_annunZoneTemp_0);
		callBooleanItem(pS->AIR_annunZoneTemp[1], pDBAK->AIR_annunZoneTemp[1], "AIR_annunZoneTemp_1", bAIR_annunZoneTemp_1);
		callBooleanItem(pS->AIR_annunZoneTemp[2], pDBAK->AIR_annunZoneTemp[2], "AIR_annunZoneTemp_2", bAIR_annunZoneTemp_2);
		callBooleanItem(pS->AIR_annunDualBleed, pDBAK->AIR_annunDualBleed, "AIR_annunDualBleed", bAIR_annunDualBleed);
		callBooleanItem(pS->AIR_annunRamDoorL, pDBAK->AIR_annunRamDoorL, "AIR_annunRamDoorL", bAIR_annunRamDoorL);
		callBooleanItem(pS->AIR_annunRamDoorR, pDBAK->AIR_annunRamDoorR, "AIR_annunRamDoorR", bAIR_annunRamDoorR);
		callBooleanItem(pS->AIR_RecircFanSwitch[0], pDBAK->AIR_RecircFanSwitch[0], "AIR_RecircFanSwitch_0", bAIR_RecircFanSwitch_0);
		callBooleanItem(pS->AIR_BleedAirSwitch[0], pDBAK->AIR_BleedAirSwitch[0], "AIR_BleedAirSwitch_0", bAIR_BleedAirSwitch_0);
		callBooleanItem(pS->AIR_RecircFanSwitch[1], pDBAK->AIR_RecircFanSwitch[0], "AIR_RecircFanSwitch_1", bAIR_RecircFanSwitch_1);
		callBooleanItem(pS->AIR_BleedAirSwitch[1], pDBAK->AIR_BleedAirSwitch[0], "AIR_BleedAirSwitch_1", bAIR_BleedAirSwitch_1);
		callBooleanItem(pS->AIR_APUBleedAirSwitch, pDBAK->AIR_APUBleedAirSwitch, "AIR_APUBleedAirSwitch", bAIR_APUBleedAirSwitch);
		callBooleanItem(pS->AIR_IsolationValveSwitch, pDBAK->AIR_IsolationValveSwitch, "AIR_IsolationValveSwitch", bAIR_IsolationValveSwitch);

		callBooleanItem(pS->AIR_annunPackTripOff[0], pDBAK->AIR_annunPackTripOff[0], "AIR_annunPackTripOff_0", bAIR_annunPackTripOff_0);
		callBooleanItem(pS->AIR_annunWingBodyOverheat[0], pDBAK->AIR_annunWingBodyOverheat[0], "AIR_annunWingBodyOverheat_0", bAIR_annunWingBodyOverheat_0);
		callBooleanItem(pS->AIR_annunBleedTripOff[0], pDBAK->AIR_annunBleedTripOff[0], "AIR_annunBleedTripOff_0", bAIR_annunBleedTripOff_0);
		callBooleanItem(pS->LTS_LandingLtFixedSw[0], pDBAK->LTS_LandingLtFixedSw[0], "LTS_LandingLtFixedSw_0", bLTS_LandingLtFixedSw_0);
		callBooleanItem(pS->LTS_RunwayTurnoffSw[0], pDBAK->LTS_RunwayTurnoffSw[0], "LTS_RunwayTurnoffSw_0", bLTS_RunwayTurnoffSw_0);

		callBooleanItem(pS->AIR_annunPackTripOff[1], pDBAK->AIR_annunPackTripOff[1], "AIR_annunPackTripOff_1", bAIR_annunPackTripOff_1);
		callBooleanItem(pS->AIR_annunWingBodyOverheat[1], pDBAK->AIR_annunWingBodyOverheat[1], "AIR_annunWingBodyOverheat_1", bAIR_annunWingBodyOverheat_1);
		callBooleanItem(pS->AIR_annunBleedTripOff[1], pDBAK->AIR_annunBleedTripOff[1], "AIR_annunBleedTripOff_1", bAIR_annunBleedTripOff_1);
		callBooleanItem(pS->LTS_LandingLtFixedSw[1], pDBAK->LTS_LandingLtFixedSw[1], "LTS_LandingLtFixedSw_1", bLTS_LandingLtFixedSw_1);
		callBooleanItem(pS->LTS_RunwayTurnoffSw[1], pDBAK->LTS_RunwayTurnoffSw[1], "LTS_RunwayTurnoffSw_1", bLTS_RunwayTurnoffSw_1);

		callBooleanItem(pS->LTS_TaxiSw, pDBAK->LTS_TaxiSw, "LTS_TaxiSw", bLTS_TaxiSw);
		callBooleanItem(pS->LTS_LogoSw, pDBAK->LTS_LogoSw, "LTS_LogoSw", bLTS_LogoSw);
		callBooleanItem(pS->LTS_AntiCollisionSw, pDBAK->LTS_AntiCollisionSw, "LTS_AntiCollisionSw", bLTS_AntiCollisionSw);
		callBooleanItem(pS->LTS_WingSw, pDBAK->LTS_WingSw, "LTS_WingSw", bLTS_WingSw);
		callBooleanItem(pS->LTS_WheelWellSw, pDBAK->LTS_WheelWellSw, "LTS_WheelWellSw", bLTS_WheelWellSw);
		callBooleanItem(pS->WARN_annunFIRE_WARN[0], pDBAK->WARN_annunFIRE_WARN[0], "WARN_annunFIRE_WARN_0", bWARN_annunFIRE_WARN_0);
		callBooleanItem(pS->WARN_annunMASTER_CAUTION[0], pDBAK->WARN_annunMASTER_CAUTION[0], "WARN_annunMASTER_CAUTION_0", bWARN_annunMASTER_CAUTION_0);
		callBooleanItem(pS->WARN_annunFIRE_WARN[1], pDBAK->WARN_annunFIRE_WARN[1], "WARN_annunFIRE_WARN_1", bWARN_annunFIRE_WARN_1);
		callBooleanItem(pS->WARN_annunMASTER_CAUTION[1], pDBAK->WARN_annunMASTER_CAUTION[1], "WARN_annunMASTER_CAUTION_1", bWARN_annunMASTER_CAUTION_1);
		callBooleanItem(pS->WARN_annunFLT_CONT, pDBAK->WARN_annunFLT_CONT, "WARN_annunFLT_CONT", bWARN_annunFLT_CONT);
		callBooleanItem(pS->WARN_annunIRS, pDBAK->WARN_annunIRS, "WARN_annunIRS", bWARN_annunIRS);
		callBooleanItem(pS->WARN_annunFUEL, pDBAK->WARN_annunFUEL, "WARN_annunFUEL", bWARN_annunFUEL);
		callBooleanItem(pS->WARN_annunELEC, pDBAK->WARN_annunELEC, "WARN_annunELEC", bWARN_annunELEC);
		callBooleanItem(pS->WARN_annunAPU, pDBAK->WARN_annunAPU, "WARN_annunAPU", bWARN_annunAPU);
		callBooleanItem(pS->WARN_annunOVHT_DET, pDBAK->WARN_annunOVHT_DET, "WARN_annunOVHT_DET", bWARN_annunOVHT_DET);
		callBooleanItem(pS->WARN_annunANTI_ICE, pDBAK->WARN_annunANTI_ICE, "WARN_annunANTI_ICE", bWARN_annunANTI_ICE);
		callBooleanItem(pS->WARN_annunHYD, pDBAK->WARN_annunHYD, "WARN_annunHYD", bWARN_annunHYD);
		callBooleanItem(pS->WARN_annunDOORS, pDBAK->WARN_annunDOORS, "WARN_annunDOORS", bWARN_annunDOORS);
		callBooleanItem(pS->WARN_annunENG, pDBAK->WARN_annunENG, "WARN_annunENG", bWARN_annunENG);
		callBooleanItem(pS->WARN_annunOVERHEAD, pDBAK->WARN_annunOVERHEAD, "WARN_annunOVERHEAD", bWARN_annunOVERHEAD);
		callBooleanItem(pS->WARN_annunAIR_COND, pDBAK->WARN_annunAIR_COND, "WARN_annunAIR_COND", bWARN_annunAIR_COND);
		callBooleanItem(pS->EFIS_MinsSelBARO[0], pDBAK->EFIS_MinsSelBARO[0], "EFIS_MinsSelBARO_0", bEFIS_MinsSelBARO_0);
		callBooleanItem(pS->EFIS_BaroSelHPA[0], pDBAK->EFIS_BaroSelHPA[0], "EFIS_BaroSelHPA_0", bEFIS_BaroSelHPA_0);
		callBooleanItem(pS->EFIS_MinsSelBARO[1], pDBAK->EFIS_MinsSelBARO[1], "EFIS_MinsSelBARO_1", bEFIS_MinsSelBARO_1);
		callBooleanItem(pS->EFIS_BaroSelHPA[1], pDBAK->EFIS_BaroSelHPA[1], "EFIS_BaroSelHPA_1", bEFIS_BaroSelHPA_1);
		callBooleanItem(pS->MCP_IASBlank, pDBAK->MCP_IASBlank, "MCP_IASBlank", bMCP_IASBlank);
		callBooleanItem(pS->MCP_IASOverspeedFlash, pDBAK->MCP_IASOverspeedFlash, "MCP_IASOverspeedFlash", bMCP_IASOverspeedFlash);
		callBooleanItem(pS->MCP_IASUnderspeedFlash, pDBAK->MCP_IASUnderspeedFlash, "MCP_IASUnderspeedFlash", bMCP_IASUnderspeedFlash);
		callBooleanItem(pS->MCP_VertSpeedBlank, pDBAK->MCP_VertSpeedBlank, "MCP_VertSpeedBlank", bMCP_VertSpeedBlank);
		callBooleanItem(pS->MCP_FDSw[0], pDBAK->MCP_FDSw[0], "MCP_FDSw_0", bMCP_FDSw_0);
		callBooleanItem(pS->MCP_FDSw[1], pDBAK->MCP_FDSw[1], "MCP_FDSw_1", bMCP_FDSw_1);
		callBooleanItem(pS->MCP_ATArmSw, pDBAK->MCP_ATArmSw, "MCP_ATArmSw", bMCP_ATArmSw);
		callBooleanItem(pS->MCP_DisengageBar, pDBAK->MCP_DisengageBar, "MCP_DisengageBar", bMCP_DisengageBar);
		callBooleanItem(pS->MCP_annunFD[0], pDBAK->MCP_annunFD[0], "MCP_annunFD_0", bMCP_annunFD_0);
		callBooleanItem(pS->MCP_annunFD[1], pDBAK->MCP_annunFD[1], "MCP_annunFD_1", bMCP_annunFD_1);
		callBooleanItem(pS->MCP_annunATArm, pDBAK->MCP_annunATArm, "MCP_annunATArm", bMCP_annunATArm);
		callBooleanItem(pS->MCP_annunN1, pDBAK->MCP_annunN1, "MCP_annunN1", bMCP_annunN1);
		callBooleanItem(pS->MCP_annunSPEED, pDBAK->MCP_annunSPEED, "MCP_annunSPEED", bMCP_annunSPEED);
		callBooleanItem(pS->MCP_annunVNAV, pDBAK->MCP_annunVNAV, "MCP_annunVNAV", bMCP_annunVNAV);
		callBooleanItem(pS->MCP_annunLVL_CHG, pDBAK->MCP_annunLVL_CHG, "MCP_annunLVL_CHG", bMCP_annunLVL_CHG);
		callBooleanItem(pS->MCP_annunHDG_SEL, pDBAK->MCP_annunHDG_SEL, "MCP_annunHDG_SEL", bMCP_annunHDG_SEL);
		callBooleanItem(pS->MCP_annunLNAV, pDBAK->MCP_annunLNAV, "MCP_annunLNAV", bMCP_annunLNAV);
		callBooleanItem(pS->MCP_annunVOR_LOC, pDBAK->MCP_annunVOR_LOC, "MCP_annunVOR_LOC", bMCP_annunVOR_LOC);
		callBooleanItem(pS->MCP_annunAPP, pDBAK->MCP_annunAPP, "MCP_annunAPP", bMCP_annunAPP);
		callBooleanItem(pS->MCP_annunALT_HOLD, pDBAK->MCP_annunALT_HOLD, "MCP_annunALT_HOLD", bMCP_annunALT_HOLD);
		callBooleanItem(pS->MCP_annunVS, pDBAK->MCP_annunVS, "MCP_annunVS", bMCP_annunVS);
		callBooleanItem(pS->MCP_annunCMD_A, pDBAK->MCP_annunCMD_A, "MCP_annunCMD_A", bMCP_annunCMD_A);
		callBooleanItem(pS->MCP_annunCWS_A, pDBAK->MCP_annunCWS_A, "MCP_annunCWS_A", bMCP_annunCWS_A);
		callBooleanItem(pS->MCP_annunCMD_B, pDBAK->MCP_annunCMD_B, "MCP_annunCMD_B", bMCP_annunCMD_B);
		callBooleanItem(pS->MCP_annunCWS_B, pDBAK->MCP_annunCWS_B, "MCP_annunCWS_B", bMCP_annunCWS_B);
		callBooleanItem(pS->MAIN_NoseWheelSteeringSwNORM, pDBAK->MAIN_NoseWheelSteeringSwNORM, "MAIN_NoseWheelSteeringSwNORM", bMAIN_NoseWheelSteeringSwNORM);
		callBooleanItem(pS->MAIN_annunBELOW_GS[0], pDBAK->MAIN_annunBELOW_GS[0], "MAIN_annunBELOW_GS_0", bMAIN_annunBELOW_GS_0);
		callBooleanItem(pS->MAIN_annunAP[0], pDBAK->MAIN_annunAP[0], "MAIN_annunAP_0", bMAIN_annunAP_0);
		callBooleanItem(pS->MAIN_annunAT[0], pDBAK->MAIN_annunAT[0], "MAIN_annunAT_0", bMAIN_annunAT_0);
		callBooleanItem(pS->MAIN_annunFMC[0], pDBAK->MAIN_annunFMC[0], "MAIN_annunFMC_0", bMAIN_annunFMC_0);

		callBooleanItem(pS->MAIN_annunBELOW_GS[1], pDBAK->MAIN_annunBELOW_GS[1], "MAIN_annunBELOW_GS_1", bMAIN_annunBELOW_GS_1);
		callBooleanItem(pS->MAIN_annunAP[1], pDBAK->MAIN_annunAP[1], "MAIN_annunAP_1", bMAIN_annunAP_1);
		callBooleanItem(pS->MAIN_annunAT[1], pDBAK->MAIN_annunAT[1], "MAIN_annunAT_1", bMAIN_annunAT_1);
		callBooleanItem(pS->MAIN_annunFMC[1], pDBAK->MAIN_annunFMC[1], "MAIN_annunFMC_1", bMAIN_annunFMC_1);

		callBooleanItem(pS->MAIN_annunSPEEDBRAKE_ARMED, pDBAK->MAIN_annunSPEEDBRAKE_ARMED, "MAIN_annunSPEEDBRAKE_ARMED", bMAIN_annunSPEEDBRAKE_ARMED);
		callBooleanItem(pS->MAIN_annunSPEEDBRAKE_DO_NOT_ARM, pDBAK->MAIN_annunSPEEDBRAKE_DO_NOT_ARM, "MAIN_annunSPEEDBRAKE_DO_NOT_ARM", bMAIN_annunSPEEDBRAKE_DO_NOT_ARM);
		callBooleanItem(pS->MAIN_annunSPEEDBRAKE_EXTENDED, pDBAK->MAIN_annunSPEEDBRAKE_EXTENDED, "MAIN_annunSPEEDBRAKE_EXTENDED", bMAIN_annunSPEEDBRAKE_EXTENDED);
		callBooleanItem(pS->MAIN_annunSTAB_OUT_OF_TRIM, pDBAK->MAIN_annunSTAB_OUT_OF_TRIM, "MAIN_annunSTAB_OUT_OF_TRIM", bMAIN_annunSTAB_OUT_OF_TRIM);
		callBooleanItem(pS->MAIN_RMISelector1_VOR, pDBAK->MAIN_RMISelector1_VOR, "MAIN_RMISelector1_VOR", bMAIN_RMISelector1_VOR);
		callBooleanItem(pS->MAIN_RMISelector2_VOR, pDBAK->MAIN_RMISelector2_VOR, "MAIN_RMISelector2_VOR", bMAIN_RMISelector2_VOR);
		callBooleanItem(pS->MAIN_annunANTI_SKID_INOP, pDBAK->MAIN_annunANTI_SKID_INOP, "MAIN_annunANTI_SKID_INOP", bMAIN_annunANTI_SKID_INOP);
		callBooleanItem(pS->MAIN_annunAUTO_BRAKE_DISARM, pDBAK->MAIN_annunAUTO_BRAKE_DISARM, "MAIN_annunAUTO_BRAKE_DISARM", bMAIN_annunAUTO_BRAKE_DISARM);
		callBooleanItem(pS->MAIN_annunLE_FLAPS_TRANSIT, pDBAK->MAIN_annunLE_FLAPS_TRANSIT, "MAIN_annunLE_FLAPS_TRANSIT", bMAIN_annunLE_FLAPS_TRANSIT);
		callBooleanItem(pS->MAIN_annunLE_FLAPS_EXT, pDBAK->MAIN_annunLE_FLAPS_EXT, "MAIN_annunLE_FLAPS_EXT", bMAIN_annunLE_FLAPS_EXT);
		callBooleanItem(pS->MAIN_annunGEAR_transit[0], pDBAK->MAIN_annunGEAR_transit[0], "MAIN_annunGEAR_transit_0", bMAIN_annunGEAR_transit_0);
		callBooleanItem(pS->MAIN_annunGEAR_locked[0], pDBAK->MAIN_annunGEAR_locked[0], "MAIN_annunGEAR_locked_0", bMAIN_annunGEAR_locked_0);
		callBooleanItem(pS->MAIN_annunGEAR_transit[1], pDBAK->MAIN_annunGEAR_transit[1], "MAIN_annunGEAR_transit_1", bMAIN_annunGEAR_transit_1);
		callBooleanItem(pS->MAIN_annunGEAR_locked[1], pDBAK->MAIN_annunGEAR_locked[1], "MAIN_annunGEAR_locked_1", bMAIN_annunGEAR_locked_1);
		callBooleanItem(pS->MAIN_annunGEAR_transit[2], pDBAK->MAIN_annunGEAR_transit[2], "MAIN_annunGEAR_transit_2", bMAIN_annunGEAR_transit_2);
		callBooleanItem(pS->MAIN_annunGEAR_locked[2], pDBAK->MAIN_annunGEAR_locked[2], "MAIN_annunGEAR_locked_2", bMAIN_annunGEAR_locked_2);
		callBooleanItem(pS->HGS_annun_AIII, pDBAK->HGS_annun_AIII, "HGS_annun_AIII", bHGS_annun_AIII);
		callBooleanItem(pS->HGS_annun_NO_AIII, pDBAK->HGS_annun_NO_AIII, "HGS_annun_NO_AIII", bHGS_annun_NO_AIII);
		callBooleanItem(pS->HGS_annun_FLARE, pDBAK->HGS_annun_FLARE, "HGS_annun_FLARE", bHGS_annun_FLARE);
		callBooleanItem(pS->HGS_annun_RO, pDBAK->HGS_annun_RO, "HGS_annun_RO", bHGS_annun_RO);
		callBooleanItem(pS->HGS_annun_RO_CTN, pDBAK->HGS_annun_RO_CTN, "HGS_annun_RO_CTN", bHGS_annun_RO_CTN);
		callBooleanItem(pS->HGS_annun_RO_ARM, pDBAK->HGS_annun_RO_ARM, "HGS_annun_RO_ARM", bHGS_annun_RO_ARM);
		callBooleanItem(pS->HGS_annun_TO, pDBAK->HGS_annun_TO, "HGS_annun_TO", bHGS_annun_TO);
		callBooleanItem(pS->HGS_annun_TO_CTN, pDBAK->HGS_annun_TO_CTN, "HGS_annun_TO_CTN", bHGS_annun_TO_CTN);
		callBooleanItem(pS->HGS_annun_APCH, pDBAK->HGS_annun_APCH, "HGS_annun_APCH", bHGS_annun_APCH);
		callBooleanItem(pS->HGS_annun_TO_WARN, pDBAK->HGS_annun_TO_WARN, "HGS_annun_TO_WARN", bHGS_annun_TO_WARN);
		callBooleanItem(pS->HGS_annun_Bar, pDBAK->HGS_annun_Bar, "HGS_annun_Bar", bHGS_annun_Bar);
		callBooleanItem(pS->HGS_annun_FAIL, pDBAK->HGS_annun_FAIL, "HGS_annun_FAIL", bHGS_annun_FAIL);
		callBooleanItem(pS->GPWS_annunINOP, pDBAK->GPWS_annunINOP, "GPWS_annunINOP", bGPWS_annunINOP);
		callBooleanItem(pS->GPWS_FlapInhibitSw_NORM, pDBAK->GPWS_FlapInhibitSw_NORM, "GPWS_FlapInhibitSw_NORM", bGPWS_FlapInhibitSw_NORM);
		callBooleanItem(pS->GPWS_GearInhibitSw_NORM, pDBAK->GPWS_GearInhibitSw_NORM, "GPWS_GearInhibitSw_NORM", bGPWS_GearInhibitSw_NORM);
		callBooleanItem(pS->GPWS_TerrInhibitSw_NORM, pDBAK->GPWS_TerrInhibitSw_NORM, "GPWS_TerrInhibitSw_NORM", bGPWS_TerrInhibitSw_NORM);
		
		callBooleanItem(pS->CDU_annunEXEC[0], pDBAK->CDU_annunEXEC[0], "CDU_annunEXEC_0", bCDU_annunEXEC_0);
		callBooleanItem(pS->CDU_annunCALL[0], pDBAK->CDU_annunCALL[0], "CDU_annunCALL_0", bCDU_annunCALL_0);
		callBooleanItem(pS->CDU_annunFAIL[0], pDBAK->CDU_annunFAIL[0], "CDU_annunFAIL_0", bCDU_annunFAIL_0);
		callBooleanItem(pS->CDU_annunMSG[0], pDBAK->CDU_annunMSG[0], "CDU_annunMSG_0", bCDU_annunMSG_0);
		callBooleanItem(pS->CDU_annunOFST[0], pDBAK->CDU_annunOFST[0], "CDU_annunOFST_0", bCDU_annunOFST_0);

		callBooleanItem(pS->CDU_annunEXEC[1], pDBAK->CDU_annunEXEC[1], "CDU_annunEXEC_1", bCDU_annunEXEC_1);
		callBooleanItem(pS->CDU_annunCALL[1], pDBAK->CDU_annunCALL[1], "CDU_annunCALL_1", bCDU_annunCALL_1);
		callBooleanItem(pS->CDU_annunFAIL[1], pDBAK->CDU_annunFAIL[1], "CDU_annunFAIL_1", bCDU_annunFAIL_1);
		callBooleanItem(pS->CDU_annunMSG[1], pDBAK->CDU_annunMSG[1], "CDU_annunMSG_1", bCDU_annunMSG_1);
		callBooleanItem(pS->CDU_annunOFST[1], pDBAK->CDU_annunOFST[1], "CDU_annunOFST_1", bCDU_annunOFST_1);


		callBooleanItem(pS->TRIM_StabTrimMainElecSw_NORMAL, pDBAK->TRIM_StabTrimMainElecSw_NORMAL, "TRIM_StabTrimMainElecSw_NORMAL", bTRIM_StabTrimMainElecSw_NORMAL);
		callBooleanItem(pS->TRIM_StabTrimAutoPilotSw_NORMAL, pDBAK->TRIM_StabTrimAutoPilotSw_NORMAL, "TRIM_StabTrimAutoPilotSw_NORMAL", bTRIM_StabTrimAutoPilotSw_NORMAL);
		callBooleanItem(pS->PED_annunParkingBrake, pDBAK->PED_annunParkingBrake, "PED_annunParkingBrake", bPED_annunParkingBrake);
		callBooleanItem(pS->FIRE_annunENG_OVERHEAT[0], pDBAK->FIRE_annunENG_OVERHEAT[0], "FIRE_annunENG_OVERHEAT_0", bFIRE_annunENG_OVERHEAT_0);
		callBooleanItem(pS->FIRE_annunENG_OVERHEAT[1], pDBAK->FIRE_annunENG_OVERHEAT[1], "FIRE_annunENG_OVERHEAT_1", bFIRE_annunENG_OVERHEAT_1);
		callBooleanItem(pS->FIRE_HandleIlluminated[0], pDBAK->FIRE_HandleIlluminated[0], "FIRE_HandleIlluminated_0", bFIRE_HandleIlluminated_0);
		callBooleanItem(pS->FIRE_HandleIlluminated[1], pDBAK->FIRE_HandleIlluminated[1], "FIRE_HandleIlluminated_1", bFIRE_HandleIlluminated_1);
		callBooleanItem(pS->FIRE_HandleIlluminated[2], pDBAK->FIRE_HandleIlluminated[2], "FIRE_HandleIlluminated_2", bFIRE_HandleIlluminated_2);
		callBooleanItem(pS->FIRE_annunWHEEL_WELL, pDBAK->FIRE_annunWHEEL_WELL, "FIRE_annunWHEEL_WELL", bFIRE_annunWHEEL_WELL);
		callBooleanItem(pS->FIRE_annunFAULT, pDBAK->FIRE_annunFAULT, "FIRE_annunFAULT", bFIRE_annunFAULT);
		callBooleanItem(pS->FIRE_annunAPU_DET_INOP, pDBAK->FIRE_annunAPU_DET_INOP, "FIRE_annunAPU_DET_INOP", bFIRE_annunAPU_DET_INOP);
		callBooleanItem(pS->FIRE_annunAPU_BOTTLE_DISCHARGE, pDBAK->FIRE_annunAPU_BOTTLE_DISCHARGE, "FIRE_annunAPU_BOTTLE_DISCHARGE", bFIRE_annunAPU_BOTTLE_DISCHARGE);
		callBooleanItem(pS->FIRE_annunBOTTLE_DISCHARGE[0], pDBAK->FIRE_annunBOTTLE_DISCHARGE[0], "FIRE_annunBOTTLE_DISCHARGE_0", bFIRE_annunBOTTLE_DISCHARGE_0);
		callBooleanItem(pS->FIRE_annunBOTTLE_DISCHARGE[1], pDBAK->FIRE_annunBOTTLE_DISCHARGE[1], "FIRE_annunBOTTLE_DISCHARGE_1", bFIRE_annunBOTTLE_DISCHARGE_1);

		callBooleanItem(pS->FIRE_annunExtinguisherTest[0], pDBAK->FIRE_annunExtinguisherTest[0], "FIRE_annunExtinguisherTest_0", bFIRE_annunExtinguisherTest_0);
		callBooleanItem(pS->FIRE_annunExtinguisherTest[1], pDBAK->FIRE_annunExtinguisherTest[1], "FIRE_annunExtinguisherTest_1", bFIRE_annunExtinguisherTest_1);
		callBooleanItem(pS->FIRE_annunExtinguisherTest[2], pDBAK->FIRE_annunExtinguisherTest[2], "FIRE_annunExtinguisherTest_2", bFIRE_annunExtinguisherTest_2);

		callBooleanItem(pS->CARGO_annunExtTest[0], pDBAK->CARGO_annunExtTest[0], "CARGO_annunExtTest_0", bCARGO_annunExtTest_0);
		callBooleanItem(pS->CARGO_ArmedSw[0], pDBAK->CARGO_ArmedSw[0], "CARGO_ArmedSw_0", bCARGO_ArmedSw_0);

		callBooleanItem(pS->CARGO_annunExtTest[1], pDBAK->CARGO_annunExtTest[1], "CARGO_annunExtTest_1", bCARGO_annunExtTest_1);
		callBooleanItem(pS->CARGO_ArmedSw[1], pDBAK->CARGO_ArmedSw[1], "CARGO_ArmedSw_1", bCARGO_ArmedSw_1);

		callBooleanItem(pS->CARGO_annunFWD, pDBAK->CARGO_annunFWD, "CARGO_annunFWD", bCARGO_annunFWD);
		callBooleanItem(pS->CARGO_annunAFT, pDBAK->CARGO_annunAFT, "CARGO_annunAFT", bCARGO_annunAFT);
		callBooleanItem(pS->CARGO_annunDETECTOR_FAULT, pDBAK->CARGO_annunDETECTOR_FAULT, "CARGO_annunDETECTOR_FAULT", bCARGO_annunDETECTOR_FAULT);
		callBooleanItem(pS->CARGO_annunDISCH, pDBAK->CARGO_annunDISCH, "CARGO_annunDISCH", bCARGO_annunDISCH);
		callBooleanItem(pS->HGS_annunRWY, pDBAK->HGS_annunRWY, "HGS_annunRWY", bHGS_annunRWY);
		callBooleanItem(pS->HGS_annunGS, pDBAK->HGS_annunGS, "HGS_annunGS", bHGS_annunGS);
		callBooleanItem(pS->HGS_annunFAULT, pDBAK->HGS_annunFAULT, "HGS_annunFAULT", bHGS_annunFAULT);
		callBooleanItem(pS->HGS_annunCLR, pDBAK->HGS_annunCLR, "HGS_annunCLR", bHGS_annunCLR);
		callBooleanItem(pS->XPDR_XpndrSelector_2, pDBAK->XPDR_XpndrSelector_2, "XPDR_XpndrSelector_2", bXPDR_XpndrSelector_2);
		callBooleanItem(pS->XPDR_AltSourceSel_2, pDBAK->XPDR_AltSourceSel_2, "XPDR_AltSourceSel_2", bXPDR_AltSourceSel_2);
		callBooleanItem(pS->XPDR_annunFAIL, pDBAK->XPDR_annunFAIL, "XPDR_annunFAIL", bXPDR_annunFAIL);
		callBooleanItem(pS->TRIM_StabTrimSw_NORMAL, pDBAK->TRIM_StabTrimSw_NORMAL, "TRIM_StabTrimSw_NORMAL", bTRIM_StabTrimSw_NORMAL);
		callBooleanItem(pS->PED_annunLOCK_FAIL, pDBAK->PED_annunLOCK_FAIL, "PED_annunLOCK_FAIL", bPED_annunLOCK_FAIL);
		callBooleanItem(pS->PED_annunAUTO_UNLK, pDBAK->PED_annunAUTO_UNLK, "PED_annunAUTO_UNLK", bPED_annunAUTO_UNLK);
		callBooleanItem(pS->ENG_StartValve[0], pDBAK->ENG_StartValve[0], "ENG_StartValve_0", bENG_StartValve_0);
		callBooleanItem(pS->ENG_StartValve[1], pDBAK->ENG_StartValve[1], "ENG_StartValve_1", bENG_StartValve_1);
		callBooleanItem(pS->IRS_aligned, pDBAK->IRS_aligned, "IRS_aligned", bIRS_aligned);
		callBooleanItem(pS->WeightInKg, pDBAK->WeightInKg, "WeightInKg", bWeightInKg);
		callBooleanItem(pS->GPWS_V1CallEnabled, pDBAK->GPWS_V1CallEnabled, "GPWS_V1CallEnabled", bGPWS_V1CallEnabled);
		callBooleanItem(pS->GroundConnAvailable, pDBAK->GroundConnAvailable, "GroundConnAvailable", bGroundConnAvailable);
		callBooleanItem(pS->FMC_PerfInputComplete, pDBAK->FMC_PerfInputComplete, "FMC_PerfInputComplete", bFMC_PerfInputComplete);

		callCharItem(pS->FMC_flightNumber[0], pDBAK->FMC_flightNumber[0], "FMC_flightNumber_0", bFMC_flightNumber_0);
		callCharItem(pS->FMC_flightNumber[1], pDBAK->FMC_flightNumber[1], "FMC_flightNumber_1", bFMC_flightNumber_1);
		callCharItem(pS->FMC_flightNumber[2], pDBAK->FMC_flightNumber[2], "FMC_flightNumber_2", bFMC_flightNumber_2);
		callCharItem(pS->FMC_flightNumber[3], pDBAK->FMC_flightNumber[3], "FMC_flightNumber_3", bFMC_flightNumber_3);
		callCharItem(pS->FMC_flightNumber[4], pDBAK->FMC_flightNumber[4], "FMC_flightNumber_4", bFMC_flightNumber_4);
		callCharItem(pS->FMC_flightNumber[5], pDBAK->FMC_flightNumber[5], "FMC_flightNumber_5", bFMC_flightNumber_5);
		callCharItem(pS->FMC_flightNumber[6], pDBAK->FMC_flightNumber[6], "FMC_flightNumber_6", bFMC_flightNumber_6);
		callCharItem(pS->FMC_flightNumber[7], pDBAK->FMC_flightNumber[7], "FMC_flightNumber_7", bFMC_flightNumber_7);
		callCharItem(pS->FMC_flightNumber[8], pDBAK->FMC_flightNumber[8], "FMC_flightNumber_8", bFMC_flightNumber_8);

		callFloatItem(pS->FUEL_FuelTempNeedle, pDBAK->FUEL_FuelTempNeedle, "FUEL_FuelTempNeedle", bFUEL_FuelTempNeedle);
		callFloatItem(pS->APU_EGTNeedle, pDBAK->APU_EGTNeedle, "APU_EGTNeedle", bAPU_EGTNeedle);
		callFloatItem(pS->MCP_IASMach, pDBAK->MCP_IASMach, "MCP_IASMach", bMCP_IASMach);
		callFloatItem(pS->MAIN_TEFlapsNeedle[0], pDBAK->MAIN_TEFlapsNeedle[0], "MAIN_TEFlapsNeedle_0", bMAIN_TEFlapsNeedle_0);
		callFloatItem(pS->MAIN_TEFlapsNeedle[1], pDBAK->MAIN_TEFlapsNeedle[1], "MAIN_TEFlapsNeedle_1", bMAIN_TEFlapsNeedle_1);
		callFloatItem(pS->MAIN_BrakePressNeedle, pDBAK->MAIN_BrakePressNeedle, "MAIN_BrakePressNeedle", bMAIN_BrakePressNeedle);
		callFloatItem(pS->AIR_DuctPress[0], pDBAK->AIR_DuctPress[0], "AIR_DuctPress_0", bAIR_DuctPress_0);
		callFloatItem(pS->AIR_DuctPress[1], pDBAK->AIR_DuctPress[1], "AIR_DuctPress_1", bAIR_DuctPress_1);
		callFloatItem(pS->FUEL_QtyCenter, pDBAK->FUEL_QtyCenter, "FUEL_QtyCenter", bFUEL_QtyCenter);
		callFloatItem(pS->FUEL_QtyLeft, pDBAK->FUEL_QtyLeft, "FUEL_QtyLeft", bFUEL_QtyLeft);
		callFloatItem(pS->FUEL_QtyRight, pDBAK->FUEL_QtyRight, "FUEL_QtyRight", bFUEL_QtyRight);
		callFloatItem(pS->FMC_DistanceToTOD, pDBAK->FMC_DistanceToTOD, "FMC_DistanceToTOD", bFMC_DistanceToTOD);
		callFloatItem(pS->FMC_DistanceToDest, pDBAK->FMC_DistanceToDest, "FMC_DistanceToDest", bFMC_DistanceToDest);

		callShortItem(pS->MCP_VertSpeed, pDBAK->MCP_VertSpeed, "MCP_VertSpeed", bMCP_VertSpeed);
		callShortItem(pS->FMC_LandingAltitude, pDBAK->FMC_LandingAltitude, "FMC_LandingAltitude", bFMC_LandingAltitude);

		callUnsignedIntegerItem(pS->AIR_FltAltWindow, pDBAK->AIR_FltAltWindow, "AIR_FltAltWindow", bAIR_FltAltWindow);
		callUnsignedIntegerItem(pS->AIR_LandAltWindow, pDBAK->AIR_LandAltWindow, "AIR_LandAltWindow", bAIR_LandAltWindow);
		callUnsignedIntegerItem(pS->AIR_OutflowValveSwitch, pDBAK->AIR_OutflowValveSwitch, "AIR_OutflowValveSwitch", bAIR_OutflowValveSwitch);
		callUnsignedIntegerItem(pS->AIR_PressurizationModeSelector, pDBAK->AIR_PressurizationModeSelector, "AIR_PressurizationModeSelector", bAIR_PressurizationModeSelector);

		callUnsignedShortItem(pS->MCP_Course[0], pDBAK->MCP_Course[0], "MCP_Course_0", bMCP_Course_0);
		callUnsignedShortItem(pS->MCP_Course[1], pDBAK->MCP_Course[1], "MCP_Course_1", bMCP_Course_1);
		callUnsignedShortItem(pS->MCP_Heading, pDBAK->MCP_Heading, "MCP_Heading", bMCP_Heading);
		callUnsignedShortItem(pS->MCP_Altitude, pDBAK->MCP_Altitude, "MCP_Altitude", bMCP_Altitude);	
	}

	FSUIPC_Process(&dwResult);
	memcpy(pDBAK, pS, sizeof(PMDG_NGX_Data));

	/*clock_t t2=clock();

	std::string number;
	std::stringstream strstream;
	strstream << t2 - t1;
	strstream >> number;

	printLog("Timer : " + number + " ms");*/
}

void CALLBACK dispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext) {
	if (quit) {
		FSUIPC_Close();
		HRESULT hr = SimConnect_Close(hSimConnect);

		active = false;

		return;
	}

	switch(pData->dwID)	{
		case SIMCONNECT_RECV_ID_CLIENT_DATA: {
			SIMCONNECT_RECV_CLIENT_DATA *pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

			switch(pObjData->dwRequestID) {
				case DATA_REQUEST: {
					PMDG_NGX_Data *pS = (PMDG_NGX_Data*)&pObjData->dwData;
					ProcessNGXData(pS);
					break;
				}
			}

			break;
		}

		case SIMCONNECT_RECV_ID_QUIT: {
			quit = 1;
			break;
		}

		default:
			break;
	}
}

void initCommunication() {
    HRESULT hr;

	//printLog("Sleeping!!!!!!!!!!!!!!!!!!!!!!!!");

	Sleep(5000);

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "ngxSDK2Lua", NULL, 0, 0, 0))) {
		// 1) Set up data connection

		FSUIPC_Open(SIM_FSX, &dwResult);
		
        // Associate an ID with the PMDG data area name
		hr = SimConnect_MapClientDataNameToID (hSimConnect, PMDG_NGX_DATA_NAME, PMDG_NGX_DATA_ID);

        // Define the data area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition (hSimConnect, PMDG_NGX_DATA_DEFINITION, 0, sizeof(PMDG_NGX_Data), 0, 0);

        // Sign up for notification of data change.  
		// SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED flag asks for the data to be sent only when some of the data is changed.
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_NGX_DATA_ID, DATA_REQUEST, PMDG_NGX_DATA_DEFINITION, 
                                                       SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		// 3) Request current aircraft .air file path
		hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");

		pDBAK = new PMDG_NGX_Data;
		
		active = true;
		quit = false;
		//starting = false;
		
		// 5) Main loop
        while( ! quit ) {
			// receive and process the NGX data
			SimConnect_CallDispatch(hSimConnect, dispatchProc, NULL);

			Sleep(50);
        }

		FSUIPC_Close();
		HRESULT hr = SimConnect_Close(hSimConnect);

		active = false;
    }

	else
		printf("\nUnable to connect!\n");
}

DWORD getDWORD(const char* param) {
	std::stringstream strValue;
	strValue << param;

	DWORD pdRES;
	strValue >> pdRES;

	return pdRES;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//								Registering Lua functions.									  //
////////////////////////////////////////////////////////////////////////////////////////////////

static int cpp_registerOffsetMap(lua_State *L) {
	try {
		string item = luaL_checklstring(L, 1, NULL);
		DWORD offset = luaL_checknumber(L, 2);

		int numArgs = lua_gettop(L);

		DWORD bitPos = NULL;
		if (numArgs == 3) {
			bitPos = luaL_checknumber(L, 3);
		}
	
		OffsetCall *oCall = new OffsetCall();
		
		oCall->offset = offset;
		oCall->bitPos = bitPos != NULL ? bitPos : NULL;

		offsetMappings[item] = *oCall;

		bEventCallbacks = true;

		return 0;	
	}
	catch( char* str ) {
		lua_pushstring(L, str);
		return 1;
	}																																
}

static int cpp_registerLuaFile(lua_State *L) {
	try {
		LuaGlobal = L;

		int numArgs = lua_gettop(L);
	
		tstring luaPath = luaL_checklstring(L, 1, NULL);
		tstring luaFile = localdir + "\\Modules\\ngxSDK2Lua\\" + luaPath;

		bEventCallbacks = true;

		if (luaL_loadfile(LuaGlobal, luaFile.c_str()) || lua_pcall(LuaGlobal, 0, 0, 0)) {		// open Lua callback file 
			throw std::string(std::string(lua_tostring(LuaGlobal, -1)));
		}
	}
	catch( char* str ) {
		lua_pushstring(L, str);
		return 1;
	}																																
}

////////////////////////////////////////////////////////////////////////////////////////////////
//												ipc											  //
////////////////////////////////////////////////////////////////////////////////////////////////

static int cpp_FSUIPC_ProcessTimer(lua_State *L) {
	DWORD sleepTime = luaL_checknumber(L, 1);																										
	
    while( ! quit ) {
		FSUIPC_Process(&dwResult);

        Sleep(sleepTime);
    }

	return 0;																																			
}

////////////////////////////////////////////////		UB		/////////////////////////////////

static int cpp_FSUIPC_readUB(lua_State *L) {
	try {
		unsigned char *pdData = new unsigned char[1];

		FSUIPC_Read(luaL_checknumber(L, 1), 1, (void*)pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, *pdData);
		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUB(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 1, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUB_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 1, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		SB		/////////////////////////////////

static int cpp_FSUIPC_readSB(lua_State *L) {
	try {
		signed char *pdData = new signed char[1];

		FSUIPC_Read(luaL_checknumber(L, 1), 1, (void*)pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, *pdData);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSB(lua_State *L) {
	try {
		signed char pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 1, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSB_Delayed(lua_State *L) {
	try {
		signed char pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 1, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		UW		/////////////////////////////////

static int cpp_FSUIPC_readUW(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 2, &genericvalue.w, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.w);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUW(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 2, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUW_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 2, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		SW		/////////////////////////////////

static int cpp_FSUIPC_readSW(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 2, &genericvalue.s, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.s);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSW(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 2, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSW_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 2, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		UD		/////////////////////////////////

static int cpp_FSUIPC_readUD(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 4, &genericvalue.dw, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.dw);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUD(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeUD_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		SD		/////////////////////////////////

static int cpp_FSUIPC_readSD(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 4, &genericvalue.n, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.n);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSD(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeSD_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		FLT		/////////////////////////////////

static int cpp_FSUIPC_readFLT(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 4, &genericvalue.f, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.f);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeFLT(lua_State *L) {
	try{
		const char* pdData = luaL_checklstring(L, 2, NULL);

		float f = atof(pdData);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &f, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
		}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeFLT_Delayed(lua_State *L) {
	try {
		const char* pdData = luaL_checklstring(L, 2, NULL);

		float f = atof(pdData);

		FSUIPC_Write(luaL_checknumber(L, 1), 4, &f, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		DBL		/////////////////////////////////

static int cpp_FSUIPC_readDBL(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 8, &genericvalue.d, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.d);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeDBL(lua_State *L) {
	try{
		const char* pdData = luaL_checklstring(L, 2, NULL);

		double d = atof(pdData);

		FSUIPC_Write(luaL_checknumber(L, 1), 8, &d, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeDBL_Delayed(lua_State *L) {
	try {
		const char* pdData = luaL_checklstring(L, 2, NULL);

		double d = atof(pdData);

		FSUIPC_Write(luaL_checknumber(L, 1), 8, &d, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		DD		/////////////////////////////////

static int cpp_FSUIPC_readDD(lua_State *L) {
	try {
		DWORD offset = luaL_checknumber(L, 1);

		FSUIPC_Read(offset, 8, &genericvalue.n8, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, genericvalue.d);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeDD(lua_State *L) {
	try{
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 8, &pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_writeDD_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), 8, &pdData, &dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		GEN		/////////////////////////////////

static int cpp_FSUIPC_Read(lua_State *L) {
	try {
		int len = luaL_checknumber(L, 2);
		signed char *pdData = new signed char[len];

		FSUIPC_Read(luaL_checknumber(L, 1), len, (void*)pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, *pdData);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_Write(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 3);
		int len = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), luaL_checknumber(L, 2), &pdData, &dwResult);
		//FSUIPC_Write(0x66C0, 16, "d", &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, len);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}


static int cpp_FSUIPC_Write_Delayed(lua_State *L) {
	try {
		DWORD pdData = luaL_checknumber(L, 3);
		int len = luaL_checknumber(L, 2);

		FSUIPC_Write(luaL_checknumber(L, 1), luaL_checknumber(L, 2), &pdData, &dwResult);
		//FSUIPC_Write(0x66C0, 16, "d", &dwResult);
		//FSUIPC_Process(&dwResult);

		lua_pushnumber(L, len);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

////////////////////////////////////////////////		String		/////////////////////////////////

static int cpp_FSUIPC_ReadString(lua_State *L) {
	try {
		int len = luaL_checknumber(L, 2) + 1;
		char *pdData = new char[len];
		
		FSUIPC_Read(luaL_checknumber(L, 1), len, (void*)pdData, &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushstring(L, pdData);

		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_WriteString(lua_State *L) {
	try {
		const char* respS = luaL_checklstring(L, 2, NULL);
		int lenS = strlen(respS); 

		FSUIPC_Write(luaL_checknumber(L, 1), lenS, (void*)respS, &dwResult);
		//FSUIPC_Write(0x66C0, 16, "d", &dwResult);
		FSUIPC_Process(&dwResult);

		lua_pushnumber(L, lenS);
	
		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_WriteString_Delayed(lua_State *L) {
	try {
		const char* respS = luaL_checklstring(L, 2, NULL);
		int lenS = strlen(respS); 

		FSUIPC_Write(luaL_checknumber(L, 1), lenS, (void*)respS, &dwResult);
		//FSUIPC_Write(0x66C0, 16, "d", &dwResult);
		//FSUIPC_Process(&dwResult);

		lua_pushnumber(L, lenS);
	
		return 1;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

static int cpp_FSUIPC_Process(lua_State *L) {
	try {
		FSUIPC_Process(&dwResult);

		return 0;
	}
	catch( char * str ) {
		lua_pushstring(L, str);
		return 1;
	}
}

tstring getDLLPath() {
    TCHAR s[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, s);

    return s;
}

//static int cpp_kill(lua_State *L) {
//	quit = true;
//
//	int n = 1;
//	while (active && n < 20) {
//		quit = true;
//		Sleep(500);
//		n++;
//	}
//
//	if (active) {
//		lua_pushstring(L, "Error: ngxSDK2Lua could not terminate");
//		return 1;
//	}
//
//	lua_pushstring(L, "ngxSDK2Lua kill() exited with code 0");
//
//	return 1;
//}

static int cpp_start(lua_State *L) {
	/*if (starting) {
		lua_pushstring(L, "Error: another instance of ngxSDK2Lua is starting at this time. ");
		return 1;
	}
	starting = true;
	*/
	
	quit = true;

	int n = 1;
	while (active && n < 20) {
		quit = true;
		Sleep(500);
		n++;
	}

	if (active) {
		lua_pushstring(L, "Error: ngxSDK2Lua could not restart");
		return 1;
	}

	localdir = getDLLPath();

	tstring luadir = localdir + "\\Modules\\lua\\lib.lua";

	// Load lib.lua
	luaL_loadfile(LuaGlobal, luadir.c_str()) || lua_pcall(LuaGlobal, 0, 0, 0);
	
	// run installed modules
	tstring dir = localdir + "\\Modules\\ngxSDK2Lua";
	
	DIR *dp;
    struct dirent *dirp;
	struct stat filestat;

    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        string dirpath = (dir + "\\" + dirp->d_name);

		// If the file is in some way invalid we'll skip it 
		if (stat( dirpath.c_str(), &filestat )) 
			continue;

		if (S_ISDIR( filestat.st_mode )) {
			string configpath = (dir + "\\" + dirp->d_name + "\\" + "moduleConfig.lua");

			if (luaL_loadfile(LuaGlobal, configpath.c_str()) || lua_pcall(LuaGlobal, 0, 0, 0)) {		// open module config file 
				continue;
			}

			// attempt to load moduleConfig.lua
			try	{
				LuaCall<NullT, string>(LuaGlobal, "moduleConfig").call(dirp->d_name);

			} catch (string errMsg) {
				try	{
					printLog(errMsg);
				} catch (...) {
					continue;
				}

				continue;
			}
		}
    }

    closedir(dp);

	/*boost::thread thrd(start);
    thrd.detach();
	printLog("Exitttttttttttttttttttttttttttttttttttttttttt");*/

	initCommunication();

	lua_pushstring(L, "ngxSDK2Lua start() exited with code 0");

	return 1;
}

//int cpp_isStarting(lua_State *L) {
//	lua_pushboolean(L, starting);
//
//	return 1;
//}

static void start() {
	initCommunication();

	//return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//								Registered SDK Lua functions.								  //
////////////////////////////////////////////////////////////////////////////////////////////////

static int get_IRS_DisplaySelector(lua_State *L) { lua_pushinteger(L, pDBAK->IRS_DisplaySelector); return 1; }
static int get_IRS_ModeSelector_0(lua_State *L) { lua_pushinteger(L, pDBAK->IRS_ModeSelector[0]); return 1; }
static int get_IRS_ModeSelector_1(lua_State *L) { lua_pushinteger(L, pDBAK->IRS_ModeSelector[1]); return 1; }
static int get_LTS_DomeWhiteSw(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_DomeWhiteSw); return 1; }
static int get_OXY_Needle(lua_State *L) { lua_pushinteger(L, pDBAK->OXY_Needle); return 1; }
static int get_FCTL_FltControl_Sw_0(lua_State *L) { lua_pushinteger(L, pDBAK->FCTL_FltControl_Sw[0]); return 1; }
static int get_FCTL_FltControl_Sw_1(lua_State *L) { lua_pushinteger(L, pDBAK->FCTL_FltControl_Sw[1]); return 1; }
static int get_FCTL_AltnFlaps_Control_Sw(lua_State *L) { lua_pushinteger(L, pDBAK->FCTL_AltnFlaps_Control_Sw); return 1; }
static int get_NAVDIS_VHFNavSelector(lua_State *L) { lua_pushinteger(L, pDBAK->NAVDIS_VHFNavSelector); return 1; }
static int get_NAVDIS_IRSSelector(lua_State *L) { lua_pushinteger(L, pDBAK->NAVDIS_IRSSelector); return 1; }
static int get_NAVDIS_FMCSelector(lua_State *L) { lua_pushinteger(L, pDBAK->NAVDIS_FMCSelector); return 1; }
static int get_NAVDIS_SourceSelector(lua_State *L) { lua_pushinteger(L, pDBAK->NAVDIS_SourceSelector); return 1; }
static int get_NAVDIS_ControlPaneSelector(lua_State *L) { lua_pushinteger(L, pDBAK->NAVDIS_ControlPaneSelector); return 1; }
static int get_ELEC_DCMeterSelector(lua_State *L) { lua_pushinteger(L, pDBAK->ELEC_DCMeterSelector); return 1; }
static int get_ELEC_ACMeterSelector(lua_State *L) { lua_pushinteger(L, pDBAK->ELEC_ACMeterSelector); return 1; }
static int get_ELEC_BatSelector(lua_State *L) { lua_pushinteger(L, pDBAK->ELEC_BatSelector); return 1; }
static int get_ELEC_StandbyPowerSelector(lua_State *L) { lua_pushinteger(L, pDBAK->ELEC_StandbyPowerSelector); return 1; }
static int get_OH_WiperLSelector(lua_State *L) { lua_pushinteger(L, pDBAK->OH_WiperLSelector); return 1; }
static int get_OH_WiperRSelector(lua_State *L) { lua_pushinteger(L, pDBAK->OH_WiperRSelector); return 1; }
static int get_LTS_CircuitBreakerKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_CircuitBreakerKnob); return 1; }
static int get_LTS_OvereadPanelKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_OvereadPanelKnob); return 1; }
static int get_LTS_EmerExitSelector(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_EmerExitSelector); return 1; }
static int get_COMM_NoSmokingSelector(lua_State *L) { lua_pushinteger(L, pDBAK->COMM_NoSmokingSelector); return 1; }
static int get_COMM_FastenBeltsSelector(lua_State *L) { lua_pushinteger(L, pDBAK->COMM_FastenBeltsSelector); return 1; }
static int get_AIR_TempSourceSelector(lua_State *L) { lua_pushinteger(L, pDBAK->AIR_TempSourceSelector); return 1; }
static int get_LTS_LandingLtRetractableSw_0(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_LandingLtRetractableSw[0]); return 1; }
static int get_LTS_LandingLtRetractableSw_1(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_LandingLtRetractableSw[1]); return 1; }
static int get_APU_Selector(lua_State *L) { lua_pushinteger(L, pDBAK->APU_Selector); return 1; }
static int get_ENG_StartSelector_0(lua_State *L) { lua_pushinteger(L, pDBAK->ENG_StartSelector[0]); return 1; }
static int get_ENG_StartSelector_1(lua_State *L) { lua_pushinteger(L, pDBAK->ENG_StartSelector[1]); return 1; }
static int get_ENG_IgnitionSelector(lua_State *L) { lua_pushinteger(L, pDBAK->ENG_IgnitionSelector); return 1; }
static int get_LTS_PositionSw(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_PositionSw); return 1; }
static int get_EFIS_VORADFSel1_0(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_VORADFSel1[0]); return 1; }
static int get_EFIS_VORADFSel1_1(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_VORADFSel1[1]); return 1; }
static int get_EFIS_VORADFSel2_0(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_VORADFSel2[0]); return 1; }
static int get_EFIS_ModeSel_0(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_ModeSel[0]); return 1; }
static int get_EFIS_RangeSel_0(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_RangeSel[0]); return 1; }
static int get_EFIS_VORADFSel2_1(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_VORADFSel2[1]); return 1; }
static int get_EFIS_ModeSel_1(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_ModeSel[1]); return 1; }
static int get_EFIS_RangeSel_1(lua_State *L) { lua_pushinteger(L, pDBAK->EFIS_RangeSel[1]); return 1; }
static int get_MCP_BankLimitSel(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_BankLimitSel); return 1; }
static int get_MAIN_MainPanelDUSel_0(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_MainPanelDUSel[0]); return 1; }
static int get_MAIN_LowerDUSel_0(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_LowerDUSel[0]); return 1; }
static int get_MAIN_DisengageTestSelector_0(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_DisengageTestSelector[0]); return 1; }
static int get_MAIN_MainPanelDUSel_1(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_MainPanelDUSel[1]); return 1; }
static int get_MAIN_LowerDUSel_1(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_LowerDUSel[1]); return 1; }
static int get_MAIN_DisengageTestSelector_1(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_DisengageTestSelector[1]); return 1; }
static int get_MAIN_LightsSelector(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_LightsSelector); return 1; }
static int get_MAIN_N1SetSelector(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_N1SetSelector); return 1; }
static int get_MAIN_SpdRefSelector(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_SpdRefSelector); return 1; }
static int get_MAIN_FuelFlowSelector(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_FuelFlowSelector); return 1; }
static int get_MAIN_AutobrakeSelector(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_AutobrakeSelector); return 1; }
static int get_MAIN_GearLever(lua_State *L) { lua_pushinteger(L, pDBAK->MAIN_GearLever); return 1; }
static int get_LTS_MainPanelKnob_0(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_MainPanelKnob[0]); return 1; }
static int get_LTS_MainPanelKnob_1(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_MainPanelKnob[1]); return 1; }
static int get_LTS_BackgroundKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_BackgroundKnob); return 1; }
static int get_LTS_AFDSFloodKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_AFDSFloodKnob); return 1; }
static int get_LTS_OutbdDUBrtKnob_0(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_OutbdDUBrtKnob[0]); return 1; }
static int get_LTS_InbdDUBrtKnob_0(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_InbdDUBrtKnob[0]); return 1; }
static int get_LTS_InbdDUMapBrtKnob_0(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_InbdDUMapBrtKnob[0]); return 1; }
static int get_LTS_OutbdDUBrtKnob_1(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_OutbdDUBrtKnob[1]); return 1; }
static int get_LTS_InbdDUBrtKnob_1(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_InbdDUBrtKnob[1]); return 1; }
static int get_LTS_InbdDUMapBrtKnob_1(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_InbdDUMapBrtKnob[1]); return 1; }
static int get_LTS_UpperDUBrtKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_UpperDUBrtKnob); return 1; }
static int get_LTS_LowerDUBrtKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_LowerDUBrtKnob); return 1; }
static int get_LTS_LowerDUMapBrtKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_LowerDUMapBrtKnob); return 1; }
static int get_CDU_BrtKnob_0(lua_State *L) { lua_pushinteger(L, pDBAK->CDU_BrtKnob[0]); return 1; }
static int get_FIRE_OvhtDetSw_0(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_OvhtDetSw[0]); return 1; }
static int get_CDU_BrtKnob_1(lua_State *L) { lua_pushinteger(L, pDBAK->CDU_BrtKnob[1]); return 1; }
static int get_FIRE_OvhtDetSw_1(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_OvhtDetSw[1]); return 1; }
static int get_FIRE_DetTestSw(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_DetTestSw); return 1; }
static int get_FIRE_HandlePos_0(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_HandlePos[0]); return 1; }
static int get_FIRE_HandlePos_1(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_HandlePos[1]); return 1; }
static int get_FIRE_HandlePos_2(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_HandlePos[2]); return 1; }
static int get_FIRE_ExtinguisherTestSw(lua_State *L) { lua_pushinteger(L, pDBAK->FIRE_ExtinguisherTestSw); return 1; }
static int get_CARGO_DetSelect_0(lua_State *L) { lua_pushinteger(L, pDBAK->CARGO_DetSelect[0]); return 1; }
static int get_CARGO_DetSelect_1(lua_State *L) { lua_pushinteger(L, pDBAK->CARGO_DetSelect[1]); return 1; }
static int get_XPDR_ModeSel(lua_State *L) { lua_pushinteger(L, pDBAK->XPDR_ModeSel); return 1; }
static int get_LTS_PedFloodKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_PedFloodKnob); return 1; }
static int get_LTS_PedPanelKnob(lua_State *L) { lua_pushinteger(L, pDBAK->LTS_PedPanelKnob); return 1; }
static int get_PED_FltDkDoorSel(lua_State *L) { lua_pushinteger(L, pDBAK->PED_FltDkDoorSel); return 1; }
static int get_FMC_TakeoffFlaps(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_TakeoffFlaps); return 1; }
static int get_FMC_V1(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_V1); return 1; }
static int get_FMC_VR(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_VR); return 1; }
static int get_FMC_V2(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_V2); return 1; }
static int get_FMC_LandingFlaps(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_LandingFlaps); return 1; }
static int get_FMC_LandingVREF(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_LandingVREF); return 1; }

static int get_IRS_SysDisplay_R(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_SysDisplay_R); return 1; }
static int get_IRS_annunGPS(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunGPS); return 1; }
static int get_IRS_annunALIGN_0(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunALIGN[0]); return 1; }
static int get_IRS_annunON_DC_0(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunON_DC[0]); return 1; }
static int get_IRS_annunFAULT_0(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunFAULT[0]); return 1; }
static int get_IRS_annunDC_FAIL_0(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunDC_FAIL[0]); return 1; }
static int get_IRS_annunALIGN_1(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunALIGN[1]); return 1; }
static int get_IRS_annunON_DC_1(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunON_DC[1]); return 1; }
static int get_IRS_annunFAULT_1(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunFAULT[1]); return 1; }
static int get_IRS_annunDC_FAIL_1(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_annunDC_FAIL[1]); return 1; }
static int get_WARN_annunPSEU(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunPSEU); return 1; }
static int get_COMM_ServiceInterphoneSw(lua_State *L) { lua_pushboolean(L, pDBAK->COMM_ServiceInterphoneSw); return 1; }
static int get_ENG_EECSwitch_0(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_EECSwitch[0]); return 1; }
static int get_ENG_annunREVERSER_0(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunREVERSER[0]); return 1; }
static int get_ENG_annunENGINE_CONTROL_0(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunENGINE_CONTROL[0]); return 1; }
static int get_ENG_annunALTN_0(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunALTN[0]); return 1; }
static int get_ENG_EECSwitch_1(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_EECSwitch[1]); return 1; }
static int get_ENG_annunREVERSER_1(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunREVERSER[1]); return 1; }
static int get_ENG_annunENGINE_CONTROL_1(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunENGINE_CONTROL[1]); return 1; }
static int get_ENG_annunALTN_1(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_annunALTN[1]); return 1; }
static int get_OXY_SwNormal(lua_State *L) { lua_pushboolean(L, pDBAK->OXY_SwNormal); return 1; }
static int get_OXY_annunPASS_OXY_ON(lua_State *L) { lua_pushboolean(L, pDBAK->OXY_annunPASS_OXY_ON); return 1; }
static int get_GEAR_annunOvhdLEFT(lua_State *L) { lua_pushboolean(L, pDBAK->GEAR_annunOvhdLEFT); return 1; }
static int get_GEAR_annunOvhdNOSE(lua_State *L) { lua_pushboolean(L, pDBAK->GEAR_annunOvhdNOSE); return 1; }
static int get_GEAR_annunOvhdRIGHT(lua_State *L) { lua_pushboolean(L, pDBAK->GEAR_annunOvhdRIGHT); return 1; }
static int get_FLTREC_SwNormal(lua_State *L) { lua_pushboolean(L, pDBAK->FLTREC_SwNormal); return 1; }
static int get_FLTREC_annunOFF(lua_State *L) { lua_pushboolean(L, pDBAK->FLTREC_annunOFF); return 1; }
static int get_FCTL_Spoiler_Sw_0(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_Spoiler_Sw[0]); return 1; }
static int get_FCTL_Spoiler_Sw_1(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_Spoiler_Sw[1]); return 1; }
static int get_FCTL_YawDamper_Sw(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_YawDamper_Sw); return 1; }
static int get_FCTL_AltnFlaps_Sw_ARM(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_AltnFlaps_Sw_ARM); return 1; }
static int get_FCTL_annunFC_LOW_PRESSURE_0(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunFC_LOW_PRESSURE[0]); return 1; }
static int get_FCTL_annunFC_LOW_PRESSURE_1(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunFC_LOW_PRESSURE[1]); return 1; }
static int get_FCTL_annunYAW_DAMPER(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunYAW_DAMPER); return 1; }
static int get_FCTL_annunLOW_QUANTITY(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunLOW_QUANTITY); return 1; }
static int get_FCTL_annunLOW_PRESSURE(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunLOW_PRESSURE); return 1; }
static int get_FCTL_annunLOW_STBY_RUD_ON(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunLOW_STBY_RUD_ON); return 1; }
static int get_FCTL_annunFEEL_DIFF_PRESS(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunFEEL_DIFF_PRESS); return 1; }
static int get_FCTL_annunSPEED_TRIM_FAIL(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunSPEED_TRIM_FAIL); return 1; }
static int get_FCTL_annunMACH_TRIM_FAIL(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunMACH_TRIM_FAIL); return 1; }
static int get_FCTL_annunAUTO_SLAT_FAIL(lua_State *L) { lua_pushboolean(L, pDBAK->FCTL_annunAUTO_SLAT_FAIL); return 1; }
static int get_FUEL_CrossFeedSw(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_CrossFeedSw); return 1; }
static int get_FUEL_PumpFwdSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpFwdSw[0]); return 1; }
static int get_FUEL_PumpAftSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpAftSw[0]); return 1; }
static int get_FUEL_PumpCtrSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpCtrSw[0]); return 1; }
static int get_FUEL_PumpFwdSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpFwdSw[1]); return 1; }
static int get_FUEL_PumpAftSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpAftSw[1]); return 1; }
static int get_FUEL_PumpCtrSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_PumpCtrSw[1]); return 1; }
static int get_FUEL_annunENG_VALVE_CLOSED_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunENG_VALVE_CLOSED[0]); return 1; }
static int get_FUEL_annunSPAR_VALVE_CLOSED_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunSPAR_VALVE_CLOSED[0]); return 1; }
static int get_FUEL_annunFILTER_BYPASS_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunFILTER_BYPASS[0]); return 1; }
static int get_FUEL_annunENG_VALVE_CLOSED_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunENG_VALVE_CLOSED[1]); return 1; }
static int get_FUEL_annunSPAR_VALVE_CLOSED_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunSPAR_VALVE_CLOSED[1]); return 1; }
static int get_FUEL_annunFILTER_BYPASS_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunFILTER_BYPASS[1]); return 1; }
static int get_FUEL_annunXFEED_VALVE_OPEN(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunXFEED_VALVE_OPEN); return 1; }
static int get_FUEL_annunLOWPRESS_Fwd_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Fwd[0]); return 1; }
static int get_FUEL_annunLOWPRESS_Aft_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Aft[0]); return 1; }
static int get_FUEL_annunLOWPRESS_Ctr_0(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Ctr[0]); return 1; }
static int get_FUEL_annunLOWPRESS_Fwd_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Fwd[1]); return 1; }
static int get_FUEL_annunLOWPRESS_Aft_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Aft[1]); return 1; }
static int get_FUEL_annunLOWPRESS_Ctr_1(lua_State *L) { lua_pushboolean(L, pDBAK->FUEL_annunLOWPRESS_Ctr[1]); return 1; }
static int get_ELEC_annunBAT_DISCHARGE(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunBAT_DISCHARGE); return 1; }
static int get_ELEC_annunTR_UNIT(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunTR_UNIT); return 1; }
static int get_ELEC_annunELEC(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunELEC); return 1; }
static int get_ELEC_CabUtilSw(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_CabUtilSw); return 1; }
static int get_ELEC_IFEPassSeatSw(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_IFEPassSeatSw); return 1; }
static int get_ELEC_annunDRIVE_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunDRIVE[0]); return 1; }
static int get_ELEC_annunDRIVE_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunDRIVE[1]); return 1; }
static int get_ELEC_annunSTANDBY_POWER_OFF(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunSTANDBY_POWER_OFF); return 1; }
static int get_ELEC_IDGDisconnectSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_IDGDisconnectSw[0]); return 1; }
static int get_ELEC_IDGDisconnectSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_IDGDisconnectSw[1]); return 1; }
static int get_ELEC_annunGRD_POWER_AVAILABLE(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunGRD_POWER_AVAILABLE); return 1; }
static int get_ELEC_GrdPwrSw(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_GrdPwrSw); return 1; }
static int get_ELEC_BusTransSw_AUTO(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_BusTransSw_AUTO); return 1; }
static int get_ELEC_GenSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_GenSw[0]); return 1; }
static int get_ELEC_APUGenSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_APUGenSw[0]); return 1; }
static int get_ELEC_annunTRANSFER_BUS_OFF_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunTRANSFER_BUS_OFF[0]); return 1; }
static int get_ELEC_annunSOURCE_OFF_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunSOURCE_OFF[0]); return 1; }
static int get_ELEC_annunGEN_BUS_OFF_0(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunGEN_BUS_OFF[0]); return 1; }
static int get_ELEC_GenSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_GenSw[1]); return 1; }
static int get_ELEC_APUGenSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_APUGenSw[1]); return 1; }
static int get_ELEC_annunTRANSFER_BUS_OFF_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunTRANSFER_BUS_OFF[1]); return 1; }
static int get_ELEC_annunSOURCE_OFF_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunSOURCE_OFF[1]); return 1; }
static int get_ELEC_annunGEN_BUS_OFF_1(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunGEN_BUS_OFF[1]); return 1; }
static int get_ELEC_annunAPU_GEN_OFF_BUS(lua_State *L) { lua_pushboolean(L, pDBAK->ELEC_annunAPU_GEN_OFF_BUS); return 1; }
static int get_APU_annunMAINT(lua_State *L) { lua_pushboolean(L, pDBAK->APU_annunMAINT); return 1; }
static int get_APU_annunLOW_OIL_PRESSURE(lua_State *L) { lua_pushboolean(L, pDBAK->APU_annunLOW_OIL_PRESSURE); return 1; }
static int get_APU_annunFAULT(lua_State *L) { lua_pushboolean(L, pDBAK->APU_annunFAULT); return 1; }
static int get_APU_annunOVERSPEED(lua_State *L) { lua_pushboolean(L, pDBAK->APU_annunOVERSPEED); return 1; }
static int get_AIR_EquipCoolingSupplyNORM(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_EquipCoolingSupplyNORM); return 1; }
static int get_AIR_EquipCoolingExhaustNORM(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_EquipCoolingExhaustNORM); return 1; }
static int get_AIR_annunEquipCoolingSupplyOFF(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunEquipCoolingSupplyOFF); return 1; }
static int get_AIR_annunEquipCoolingExhaustOFF(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunEquipCoolingExhaustOFF); return 1; }
static int get_LTS_annunEmerNOT_ARMED(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_annunEmerNOT_ARMED); return 1; }
static int get_COMM_annunCALL(lua_State *L) { lua_pushboolean(L, pDBAK->COMM_annunCALL); return 1; }
static int get_COMM_annunPA_IN_USE(lua_State *L) { lua_pushboolean(L, pDBAK->COMM_annunPA_IN_USE); return 1; }
static int get_ICE_annunOVERHEAT_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunOVERHEAT[0]); return 1; }
static int get_ICE_annunOVERHEAT_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunOVERHEAT[1]); return 1; }
static int get_ICE_annunOVERHEAT_2(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunOVERHEAT[2]); return 1; }
static int get_ICE_annunOVERHEAT_3(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunOVERHEAT[3]); return 1; }
static int get_ICE_annunON_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunON[0]); return 1; }
static int get_ICE_WindowHeatSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_WindowHeatSw[0]); return 1; }
static int get_ICE_annunON_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunON[1]); return 1; }
static int get_ICE_WindowHeatSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_WindowHeatSw[1]); return 1; }
static int get_ICE_annunON_2(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunON[2]); return 1; }
static int get_ICE_WindowHeatSw_2(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_WindowHeatSw[2]); return 1; }
static int get_ICE_annunON_3(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunON[3]); return 1; }
static int get_ICE_WindowHeatSw_3(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_WindowHeatSw[3]); return 1; }
static int get_ICE_annunCAPT_PITOT(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunCAPT_PITOT); return 1; }
static int get_ICE_annunL_ELEV_PITOT(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunL_ELEV_PITOT); return 1; }
static int get_ICE_annunL_ALPHA_VANE(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunL_ALPHA_VANE); return 1; }
static int get_ICE_annunL_TEMP_PROBE(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunL_TEMP_PROBE); return 1; }
static int get_ICE_annunFO_PITOT(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunFO_PITOT); return 1; }
static int get_ICE_annunR_ELEV_PITOT(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunR_ELEV_PITOT); return 1; }
static int get_ICE_annunR_ALPHA_VANE(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunR_ALPHA_VANE); return 1; }
static int get_ICE_annunAUX_PITOT(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunAUX_PITOT); return 1; }
static int get_ICE_TestProbeHeatSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_TestProbeHeatSw[0]); return 1; }
static int get_ICE_annunVALVE_OPEN_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunVALVE_OPEN[0]); return 1; }
static int get_ICE_annunCOWL_ANTI_ICE_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunCOWL_ANTI_ICE[0]); return 1; }
static int get_ICE_annunCOWL_VALVE_OPEN_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunCOWL_VALVE_OPEN[0]); return 1; }
static int get_ICE_TestProbeHeatSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_TestProbeHeatSw[1]); return 1; }
static int get_ICE_annunVALVE_OPEN_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunVALVE_OPEN[1]); return 1; }
static int get_ICE_annunCOWL_ANTI_ICE_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunCOWL_ANTI_ICE[1]); return 1; }
static int get_ICE_annunCOWL_VALVE_OPEN_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_annunCOWL_VALVE_OPEN[1]); return 1; }
static int get_ICE_WingAntiIceSw(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_WingAntiIceSw); return 1; }
static int get_ICE_EngAntiIceSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_EngAntiIceSw[0]); return 1; }
static int get_HYD_annunLOW_PRESS_eng_0(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunLOW_PRESS_eng[0]); return 1; }
static int get_HYD_annunLOW_PRESS_elec_0(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunLOW_PRESS_elec[0]); return 1; }
static int get_HYD_annunOVERHEAT_elec_0(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunOVERHEAT_elec[0]); return 1; }
static int get_HYD_PumpSw_eng_0(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_PumpSw_eng[0]); return 1; }
static int get_HYD_PumpSw_elec_0(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_PumpSw_elec[0]); return 1; }
static int get_ICE_EngAntiIceSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->ICE_EngAntiIceSw[1]); return 1; }
static int get_HYD_annunLOW_PRESS_eng_1(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunLOW_PRESS_eng[1]); return 1; }
static int get_HYD_annunLOW_PRESS_elec_1(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunLOW_PRESS_elec[1]); return 1; }
static int get_HYD_annunOVERHEAT_elec_1(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_annunOVERHEAT_elec[1]); return 1; }
static int get_HYD_PumpSw_eng_1(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_PumpSw_eng[1]); return 1; }
static int get_HYD_PumpSw_elec_1(lua_State *L) { lua_pushboolean(L, pDBAK->HYD_PumpSw_elec[1]); return 1; }
static int get_AIR_TrimAirSwitch(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_TrimAirSwitch); return 1; }
static int get_AIR_annunZoneTemp_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunZoneTemp[0]); return 1; }
static int get_AIR_annunZoneTemp_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunZoneTemp[1]); return 1; }
static int get_AIR_annunZoneTemp_2(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunZoneTemp[2]); return 1; }
static int get_AIR_annunDualBleed(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunDualBleed); return 1; }
static int get_AIR_annunRamDoorL(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunRamDoorL); return 1; }
static int get_AIR_annunRamDoorR(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunRamDoorR); return 1; }
static int get_AIR_RecircFanSwitch_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_RecircFanSwitch[0]); return 1; }
static int get_AIR_BleedAirSwitch_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_BleedAirSwitch[0]); return 1; }
static int get_AIR_RecircFanSwitch_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_RecircFanSwitch[1]); return 1; }
static int get_AIR_BleedAirSwitch_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_BleedAirSwitch[1]); return 1; }
static int get_AIR_APUBleedAirSwitch(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_APUBleedAirSwitch); return 1; }
static int get_AIR_IsolationValveSwitch(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_IsolationValveSwitch); return 1; }
static int get_AIR_annunPackTripOff_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunPackTripOff[0]); return 1; }
static int get_AIR_annunWingBodyOverheat_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunWingBodyOverheat[0]); return 1; }
static int get_AIR_annunBleedTripOff_0(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunBleedTripOff[0]); return 1; }
static int get_LTS_LandingLtFixedSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_LandingLtFixedSw[0]); return 1; }
static int get_LTS_RunwayTurnoffSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_RunwayTurnoffSw[0]); return 1; }
static int get_AIR_annunPackTripOff_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunPackTripOff[1]); return 1; }
static int get_AIR_annunWingBodyOverheat_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunWingBodyOverheat[1]); return 1; }
static int get_AIR_annunBleedTripOff_1(lua_State *L) { lua_pushboolean(L, pDBAK->AIR_annunBleedTripOff[1]); return 1; }
static int get_LTS_LandingLtFixedSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_LandingLtFixedSw[1]); return 1; }
static int get_LTS_RunwayTurnoffSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_RunwayTurnoffSw[1]); return 1; }
static int get_LTS_TaxiSw(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_TaxiSw); return 1; }
static int get_LTS_LogoSw(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_LogoSw); return 1; }
static int get_LTS_AntiCollisionSw(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_AntiCollisionSw); return 1; }
static int get_LTS_WingSw(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_WingSw); return 1; }
static int get_LTS_WheelWellSw(lua_State *L) { lua_pushboolean(L, pDBAK->LTS_WheelWellSw); return 1; }
static int get_WARN_annunFIRE_WARN_0(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunFIRE_WARN[0]); return 1; }
static int get_WARN_annunMASTER_CAUTION_0(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunMASTER_CAUTION[0]); return 1; }
static int get_WARN_annunFIRE_WARN_1(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunFIRE_WARN[1]); return 1; }
static int get_WARN_annunMASTER_CAUTION_1(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunMASTER_CAUTION[1]); return 1; }
static int get_WARN_annunFLT_CONT(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunFLT_CONT); return 1; }
static int get_WARN_annunIRS(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunIRS); return 1; }
static int get_WARN_annunFUEL(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunFUEL); return 1; }
static int get_WARN_annunELEC(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunELEC); return 1; }
static int get_WARN_annunAPU(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunAPU); return 1; }
static int get_WARN_annunOVHT_DET(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunOVHT_DET); return 1; }
static int get_WARN_annunANTI_ICE(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunANTI_ICE); return 1; }
static int get_WARN_annunHYD(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunHYD); return 1; }
static int get_WARN_annunDOORS(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunDOORS); return 1; }
static int get_WARN_annunENG(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunENG); return 1; }
static int get_WARN_annunOVERHEAD(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunOVERHEAD); return 1; }
static int get_WARN_annunAIR_COND(lua_State *L) { lua_pushboolean(L, pDBAK->WARN_annunAIR_COND); return 1; }
static int get_EFIS_MinsSelBARO_0(lua_State *L) { lua_pushboolean(L, pDBAK->EFIS_MinsSelBARO[0]); return 1; }
static int get_EFIS_BaroSelHPA_0(lua_State *L) { lua_pushboolean(L, pDBAK->EFIS_BaroSelHPA[0]); return 1; }
static int get_EFIS_MinsSelBARO_1(lua_State *L) { lua_pushboolean(L, pDBAK->EFIS_MinsSelBARO[1]); return 1; }
static int get_EFIS_BaroSelHPA_1(lua_State *L) { lua_pushboolean(L, pDBAK->EFIS_BaroSelHPA[1]); return 1; }
static int get_MCP_IASBlank(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_IASBlank); return 1; }
static int get_MCP_IASOverspeedFlash(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_IASOverspeedFlash); return 1; }
static int get_MCP_IASUnderspeedFlash(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_IASUnderspeedFlash); return 1; }
static int get_MCP_VertSpeedBlank(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_VertSpeedBlank); return 1; }
static int get_MCP_FDSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_FDSw[0]); return 1; }
static int get_MCP_FDSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_FDSw[1]); return 1; }
static int get_MCP_ATArmSw(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_ATArmSw); return 1; }
static int get_MCP_DisengageBar(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_DisengageBar); return 1; }
static int get_MCP_annunFD_0(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunFD[0]); return 1; }
static int get_MCP_annunFD_1(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunFD[1]); return 1; }
static int get_MCP_annunATArm(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunATArm); return 1; }
static int get_MCP_annunN1(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunN1); return 1; }
static int get_MCP_annunSPEED(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunSPEED); return 1; }
static int get_MCP_annunVNAV(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunVNAV); return 1; }
static int get_MCP_annunLVL_CHG(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunLVL_CHG); return 1; }
static int get_MCP_annunHDG_SEL(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunHDG_SEL); return 1; }
static int get_MCP_annunLNAV(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunLNAV); return 1; }
static int get_MCP_annunVOR_LOC(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunVOR_LOC); return 1; }
static int get_MCP_annunAPP(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunAPP); return 1; }
static int get_MCP_annunALT_HOLD(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunALT_HOLD); return 1; }
static int get_MCP_annunVS(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunVS); return 1; }
static int get_MCP_annunCMD_A(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunCMD_A); return 1; }
static int get_MCP_annunCWS_A(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunCWS_A); return 1; }
static int get_MCP_annunCMD_B(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunCMD_B); return 1; }
static int get_MCP_annunCWS_B(lua_State *L) { lua_pushboolean(L, pDBAK->MCP_annunCWS_B); return 1; }
static int get_MAIN_NoseWheelSteeringSwNORM(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_NoseWheelSteeringSwNORM); return 1; }
static int get_MAIN_annunBELOW_GS_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunBELOW_GS[0]); return 1; }
static int get_MAIN_annunAP_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunAP[0]); return 1; }
static int get_MAIN_annunAT_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunAT[0]); return 1; }
static int get_MAIN_annunFMC_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunFMC[0]); return 1; }
static int get_MAIN_annunBELOW_GS_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunBELOW_GS[1]); return 1; }
static int get_MAIN_annunAP_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunAP[1]); return 1; }
static int get_MAIN_annunAT_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunAT[1]); return 1; }
static int get_MAIN_annunFMC_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunFMC[1]); return 1; }
static int get_MAIN_annunSPEEDBRAKE_ARMED(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunSPEEDBRAKE_ARMED); return 1; }
static int get_MAIN_annunSPEEDBRAKE_DO_NOT_ARM(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunSPEEDBRAKE_DO_NOT_ARM); return 1; }
static int get_MAIN_annunSPEEDBRAKE_EXTENDED(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunSPEEDBRAKE_EXTENDED); return 1; }
static int get_MAIN_annunSTAB_OUT_OF_TRIM(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunSTAB_OUT_OF_TRIM); return 1; }
static int get_MAIN_RMISelector1_VOR(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_RMISelector1_VOR); return 1; }
static int get_MAIN_RMISelector2_VOR(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_RMISelector2_VOR); return 1; }
static int get_MAIN_annunANTI_SKID_INOP(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunANTI_SKID_INOP); return 1; }
static int get_MAIN_annunAUTO_BRAKE_DISARM(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunAUTO_BRAKE_DISARM); return 1; }
static int get_MAIN_annunLE_FLAPS_TRANSIT(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunLE_FLAPS_TRANSIT); return 1; }
static int get_MAIN_annunLE_FLAPS_EXT(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunLE_FLAPS_EXT); return 1; }
static int get_MAIN_annunGEAR_transit_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_transit[0]); return 1; }
static int get_MAIN_annunGEAR_locked_0(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_locked[0]); return 1; }
static int get_MAIN_annunGEAR_transit_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_transit[1]); return 1; }
static int get_MAIN_annunGEAR_locked_1(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_locked[1]); return 1; }
static int get_MAIN_annunGEAR_transit_2(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_transit[2]); return 1; }
static int get_MAIN_annunGEAR_locked_2(lua_State *L) { lua_pushboolean(L, pDBAK->MAIN_annunGEAR_locked[2]); return 1; }
static int get_HGS_annun_AIII(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_AIII); return 1; }
static int get_HGS_annun_NO_AIII(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_NO_AIII); return 1; }
static int get_HGS_annun_FLARE(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_FLARE); return 1; }
static int get_HGS_annun_RO(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_RO); return 1; }
static int get_HGS_annun_RO_CTN(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_RO_CTN); return 1; }
static int get_HGS_annun_RO_ARM(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_RO_ARM); return 1; }
static int get_HGS_annun_TO(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_TO); return 1; }
static int get_HGS_annun_TO_CTN(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_TO_CTN); return 1; }
static int get_HGS_annun_APCH(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_APCH); return 1; }
static int get_HGS_annun_TO_WARN(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_TO_WARN); return 1; }
static int get_HGS_annun_Bar(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_Bar); return 1; }
static int get_HGS_annun_FAIL(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annun_FAIL); return 1; }
static int get_GPWS_annunINOP(lua_State *L) { lua_pushboolean(L, pDBAK->GPWS_annunINOP); return 1; }
static int get_GPWS_FlapInhibitSw_NORM(lua_State *L) { lua_pushboolean(L, pDBAK->GPWS_FlapInhibitSw_NORM); return 1; }
static int get_GPWS_GearInhibitSw_NORM(lua_State *L) { lua_pushboolean(L, pDBAK->GPWS_GearInhibitSw_NORM); return 1; }
static int get_GPWS_TerrInhibitSw_NORM(lua_State *L) { lua_pushboolean(L, pDBAK->GPWS_TerrInhibitSw_NORM); return 1; }
static int get_CDU_annunEXEC_0(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunEXEC[0]); return 1; }
static int get_CDU_annunCALL_0(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunCALL[0]); return 1; }
static int get_CDU_annunFAIL_0(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunFAIL[0]); return 1; }
static int get_CDU_annunMSG_0(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunMSG[0]); return 1; }
static int get_CDU_annunOFST_0(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunOFST[0]); return 1; }
static int get_CDU_annunEXEC_1(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunEXEC[1]); return 1; }
static int get_CDU_annunCALL_1(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunCALL[1]); return 1; }
static int get_CDU_annunFAIL_1(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunFAIL[1]); return 1; }
static int get_CDU_annunMSG_1(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunMSG[1]); return 1; }
static int get_CDU_annunOFST_1(lua_State *L) { lua_pushboolean(L, pDBAK->CDU_annunOFST[1]); return 1; }
static int get_TRIM_StabTrimMainElecSw_NORMAL(lua_State *L) { lua_pushboolean(L, pDBAK->TRIM_StabTrimMainElecSw_NORMAL); return 1; }
static int get_TRIM_StabTrimAutoPilotSw_NORMAL(lua_State *L) { lua_pushboolean(L, pDBAK->TRIM_StabTrimAutoPilotSw_NORMAL); return 1; }
static int get_PED_annunParkingBrake(lua_State *L) { lua_pushboolean(L, pDBAK->PED_annunParkingBrake); return 1; }
static int get_FIRE_annunENG_OVERHEAT_0(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunENG_OVERHEAT[0]); return 1; }
static int get_FIRE_annunENG_OVERHEAT_1(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunENG_OVERHEAT[1]); return 1; }
static int get_FIRE_HandleIlluminated_0(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_HandleIlluminated[0]); return 1; }
static int get_FIRE_HandleIlluminated_1(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_HandleIlluminated[1]); return 1; }
static int get_FIRE_HandleIlluminated_2(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_HandleIlluminated[2]); return 1; }
static int get_FIRE_annunWHEEL_WELL(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunWHEEL_WELL); return 1; }
static int get_FIRE_annunFAULT(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunFAULT); return 1; }
static int get_FIRE_annunAPU_DET_INOP(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunAPU_DET_INOP); return 1; }
static int get_FIRE_annunAPU_BOTTLE_DISCHARGE(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunAPU_BOTTLE_DISCHARGE); return 1; }
static int get_FIRE_annunBOTTLE_DISCHARGE_0(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunBOTTLE_DISCHARGE[0]); return 1; }
static int get_FIRE_annunBOTTLE_DISCHARGE_1(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunBOTTLE_DISCHARGE[1]); return 1; }
static int get_FIRE_annunExtinguisherTest_0(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunExtinguisherTest[0]); return 1; }
static int get_FIRE_annunExtinguisherTest_1(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunExtinguisherTest[1]); return 1; }
static int get_FIRE_annunExtinguisherTest_2(lua_State *L) { lua_pushboolean(L, pDBAK->FIRE_annunExtinguisherTest[2]); return 1; }
static int get_CARGO_annunExtTest_0(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunExtTest[0]); return 1; }
static int get_CARGO_ArmedSw_0(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_ArmedSw[0]); return 1; }
static int get_CARGO_annunExtTest_1(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunExtTest[1]); return 1; }
static int get_CARGO_ArmedSw_1(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_ArmedSw[1]); return 1; }
static int get_CARGO_annunFWD(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunFWD); return 1; }
static int get_CARGO_annunAFT(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunAFT); return 1; }
static int get_CARGO_annunDETECTOR_FAULT(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunDETECTOR_FAULT); return 1; }
static int get_CARGO_annunDISCH(lua_State *L) { lua_pushboolean(L, pDBAK->CARGO_annunDISCH); return 1; }
static int get_HGS_annunRWY(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annunRWY); return 1; }
static int get_HGS_annunGS(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annunGS); return 1; }
static int get_HGS_annunFAULT(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annunFAULT); return 1; }
static int get_HGS_annunCLR(lua_State *L) { lua_pushboolean(L, pDBAK->HGS_annunCLR); return 1; }
static int get_XPDR_XpndrSelector_2(lua_State *L) { lua_pushboolean(L, pDBAK->XPDR_XpndrSelector_2); return 1; }
static int get_XPDR_AltSourceSel_2(lua_State *L) { lua_pushboolean(L, pDBAK->XPDR_AltSourceSel_2); return 1; }
static int get_XPDR_annunFAIL(lua_State *L) { lua_pushboolean(L, pDBAK->XPDR_annunFAIL); return 1; }
static int get_TRIM_StabTrimSw_NORMAL(lua_State *L) { lua_pushboolean(L, pDBAK->TRIM_StabTrimSw_NORMAL); return 1; }
static int get_PED_annunLOCK_FAIL(lua_State *L) { lua_pushboolean(L, pDBAK->PED_annunLOCK_FAIL); return 1; }
static int get_PED_annunAUTO_UNLK(lua_State *L) { lua_pushboolean(L, pDBAK->PED_annunAUTO_UNLK); return 1; }
static int get_ENG_StartValve_0(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_StartValve[0]); return 1; }
static int get_ENG_StartValve_1(lua_State *L) { lua_pushboolean(L, pDBAK->ENG_StartValve[1]); return 1; }
static int get_IRS_aligned(lua_State *L) { lua_pushboolean(L, pDBAK->IRS_aligned); return 1; }
static int get_WeightInKg(lua_State *L) { lua_pushboolean(L, pDBAK->WeightInKg); return 1; }
static int get_GPWS_V1CallEnabled(lua_State *L) { lua_pushboolean(L, pDBAK->GPWS_V1CallEnabled); return 1; }
static int get_GroundConnAvailable(lua_State *L) { lua_pushboolean(L, pDBAK->GroundConnAvailable); return 1; }
static int get_FMC_PerfInputComplete(lua_State *L) { lua_pushboolean(L, pDBAK->FMC_PerfInputComplete); return 1; }

static int get_FMC_flightNumber_0(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[0]); return 1; }
static int get_FMC_flightNumber_1(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[1]); return 1; }
static int get_FMC_flightNumber_2(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[2]); return 1; }
static int get_FMC_flightNumber_3(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[3]); return 1; }
static int get_FMC_flightNumber_4(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[4]); return 1; }
static int get_FMC_flightNumber_5(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[5]); return 1; }
static int get_FMC_flightNumber_6(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[6]); return 1; }
static int get_FMC_flightNumber_7(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[7]); return 1; }
static int get_FMC_flightNumber_8(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_flightNumber[8]); return 1; }

static int get_FUEL_FuelTempNeedle(lua_State *L) { lua_pushnumber(L, pDBAK->FUEL_FuelTempNeedle); return 1; }
static int get_APU_EGTNeedle(lua_State *L) { lua_pushnumber(L, pDBAK->APU_EGTNeedle); return 1; }
static int get_MCP_IASMach(lua_State *L) { lua_pushnumber(L, pDBAK->MCP_IASMach); return 1; }
static int get_MAIN_TEFlapsNeedle_0(lua_State *L) { lua_pushnumber(L, pDBAK->MAIN_TEFlapsNeedle[0]); return 1; }
static int get_MAIN_TEFlapsNeedle_1(lua_State *L) { lua_pushnumber(L, pDBAK->MAIN_TEFlapsNeedle[1]); return 1; }
static int get_MAIN_BrakePressNeedle(lua_State *L) { lua_pushnumber(L, pDBAK->MAIN_BrakePressNeedle); return 1; }
static int get_AIR_DuctPress_0(lua_State *L) { lua_pushnumber(L, pDBAK->AIR_DuctPress[0]); return 1; }
static int get_AIR_DuctPress_1(lua_State *L) { lua_pushnumber(L, pDBAK->AIR_DuctPress[1]); return 1; }
static int get_FUEL_QtyCenter(lua_State *L) { lua_pushnumber(L, pDBAK->FUEL_QtyCenter); return 1; }
static int get_FUEL_QtyLeft(lua_State *L) { lua_pushnumber(L, pDBAK->FUEL_QtyLeft); return 1; }
static int get_FUEL_QtyRight(lua_State *L) { lua_pushnumber(L, pDBAK->FUEL_QtyRight); return 1; }
static int get_FMC_DistanceToTOD(lua_State *L) { lua_pushnumber(L, pDBAK->FMC_DistanceToTOD); return 1; }
static int get_FMC_DistanceToDest(lua_State *L) { lua_pushnumber(L, pDBAK->FMC_DistanceToDest); return 1; }

static int get_MCP_VertSpeed(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_VertSpeed); return 1; }
static int get_FMC_LandingAltitude(lua_State *L) { lua_pushinteger(L, pDBAK->FMC_LandingAltitude); return 1; }

static int get_AIR_FltAltWindow(lua_State *L) { lua_pushinteger(L, pDBAK->AIR_FltAltWindow); return 1; }
static int get_AIR_LandAltWindow(lua_State *L) { lua_pushinteger(L, pDBAK->AIR_LandAltWindow); return 1; }
static int get_AIR_OutflowValveSwitch(lua_State *L) { lua_pushinteger(L, pDBAK->AIR_OutflowValveSwitch); return 1; }
static int get_AIR_PressurizationModeSelector(lua_State *L) { lua_pushinteger(L, pDBAK->AIR_PressurizationModeSelector); return 1; }

static int get_MCP_Course_0(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_Course[0]); return 1; }
static int get_MCP_Course_1(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_Course[1]); return 1; }
static int get_MCP_Heading(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_Heading); return 1; }
static int get_MCP_Altitude(lua_State *L) { lua_pushinteger(L, pDBAK->MCP_Altitude); return 1; }


////////////////////////////////////////////////////////////////////////////////////////////////
//										Lua Entry point.									  //
////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int luaopen_ngxSDK2Lua(lua_State *L){
	static const luaL_Reg asd [] = {
		{"FSUIPC_Read", cpp_FSUIPC_Read},
		{"FSUIPC_Write", cpp_FSUIPC_Write},
		{"FSUIPC_Write_Delayed", cpp_FSUIPC_Write_Delayed},

		{"readUB", cpp_FSUIPC_readUB},
		{"writeUB", cpp_FSUIPC_writeUB},
		{"writeUB_Delayed", cpp_FSUIPC_writeUB_Delayed},

		{"readSB", cpp_FSUIPC_readSB},
		{"writeSB", cpp_FSUIPC_writeSB},
		{"writeSB_Delayed", cpp_FSUIPC_writeSB_Delayed},

		{"readUW", cpp_FSUIPC_readUW},
		{"writeUW", cpp_FSUIPC_writeUW},
		{"writeUW_Delayed", cpp_FSUIPC_writeUW_Delayed},

		{"readSW", cpp_FSUIPC_readSW},
		{"writeSW", cpp_FSUIPC_writeSW},
		{"writeSW_Delayed", cpp_FSUIPC_writeSW_Delayed},

		{"readUD", cpp_FSUIPC_readUD},
		{"writeUD", cpp_FSUIPC_writeUD},
		{"writeUD_Delayed", cpp_FSUIPC_writeUD_Delayed},

		{"readSD", cpp_FSUIPC_readSD},
		{"writeSD", cpp_FSUIPC_writeSD},
		{"writeSD_Delayed", cpp_FSUIPC_writeSD_Delayed},

		{"readFLT", cpp_FSUIPC_readFLT},
		{"writeFLT", cpp_FSUIPC_writeFLT},
		{"writeFLT_Delayed", cpp_FSUIPC_writeFLT_Delayed},

		{"readDBL", cpp_FSUIPC_readDBL},
		{"writeDBL", cpp_FSUIPC_writeDBL},
		{"writeDBL_Delayed", cpp_FSUIPC_writeDBL_Delayed},

		{"readDD", cpp_FSUIPC_readDD},
		{"writeDD", cpp_FSUIPC_writeDD},
		{"writeDD_Delayed", cpp_FSUIPC_writeDD_Delayed},

				
		{"readString", cpp_FSUIPC_ReadString},
		{"writeString", cpp_FSUIPC_WriteString},
		{"writeString_Delayed", cpp_FSUIPC_WriteString_Delayed},
		
		{"FSUIPC_Process", cpp_FSUIPC_Process},
		{"registerOffsetMap", cpp_registerOffsetMap},
		{"registerLuaFile", cpp_registerLuaFile},
		{"start", cpp_start},
		//{"kill", cpp_kill},
		//{"isStarting", cpp_isStarting},
		{"FSUIPC_ProcessTimer", cpp_FSUIPC_ProcessTimer},
		
		{"get_IRS_DisplaySelector", get_IRS_DisplaySelector},
		{"get_IRS_ModeSelector_0", get_IRS_ModeSelector_0},
		{"get_IRS_ModeSelector_1", get_IRS_ModeSelector_1},
		{"get_LTS_DomeWhiteSw", get_LTS_DomeWhiteSw},
		{"get_OXY_Needle", get_OXY_Needle},
		{"get_FCTL_FltControl_Sw_0", get_FCTL_FltControl_Sw_0},
		{"get_FCTL_FltControl_Sw_1", get_FCTL_FltControl_Sw_1},
		{"get_FCTL_AltnFlaps_Control_Sw", get_FCTL_AltnFlaps_Control_Sw},
		{"get_NAVDIS_VHFNavSelector", get_NAVDIS_VHFNavSelector},
		{"get_NAVDIS_IRSSelector", get_NAVDIS_IRSSelector},
		{"get_NAVDIS_FMCSelector", get_NAVDIS_FMCSelector},
		{"get_NAVDIS_SourceSelector", get_NAVDIS_SourceSelector},
		{"get_NAVDIS_ControlPaneSelector", get_NAVDIS_ControlPaneSelector},
		{"get_ELEC_DCMeterSelector", get_ELEC_DCMeterSelector},
		{"get_ELEC_ACMeterSelector", get_ELEC_ACMeterSelector},
		{"get_ELEC_BatSelector", get_ELEC_BatSelector},
		{"get_ELEC_StandbyPowerSelector", get_ELEC_StandbyPowerSelector},
		{"get_OH_WiperLSelector", get_OH_WiperLSelector},
		{"get_OH_WiperRSelector", get_OH_WiperRSelector},
		{"get_LTS_CircuitBreakerKnob", get_LTS_CircuitBreakerKnob},
		{"get_LTS_OvereadPanelKnob", get_LTS_OvereadPanelKnob},
		{"get_LTS_EmerExitSelector", get_LTS_EmerExitSelector},
		{"get_COMM_NoSmokingSelector", get_COMM_NoSmokingSelector},
		{"get_COMM_FastenBeltsSelector", get_COMM_FastenBeltsSelector},
		{"get_AIR_TempSourceSelector", get_AIR_TempSourceSelector},
		{"get_LTS_LandingLtRetractableSw_0", get_LTS_LandingLtRetractableSw_0},
		{"get_LTS_LandingLtRetractableSw_1", get_LTS_LandingLtRetractableSw_1},
		{"get_APU_Selector", get_APU_Selector},
		{"get_ENG_StartSelector_0", get_ENG_StartSelector_0},
		{"get_ENG_StartSelector_0", get_ENG_StartSelector_0},
		{"get_ENG_IgnitionSelector", get_ENG_IgnitionSelector},
		{"get_LTS_PositionSw", get_LTS_PositionSw},
		{"get_EFIS_VORADFSel1_0", get_EFIS_VORADFSel1_0},
		{"get_EFIS_VORADFSel1_1", get_EFIS_VORADFSel1_1},
		{"get_EFIS_VORADFSel2_0", get_EFIS_VORADFSel2_0},
		{"get_EFIS_ModeSel_0", get_EFIS_ModeSel_0},
		{"get_EFIS_RangeSel_0", get_EFIS_RangeSel_0},
		{"get_EFIS_VORADFSel2_1", get_EFIS_VORADFSel2_1},
		{"get_EFIS_ModeSel_1", get_EFIS_ModeSel_1},
		{"get_EFIS_RangeSel_1", get_EFIS_RangeSel_1},
		{"get_MCP_BankLimitSel", get_MCP_BankLimitSel},
		{"get_MAIN_MainPanelDUSel_1", get_MAIN_MainPanelDUSel_1},
		{"get_MAIN_LowerDUSel_0", get_MAIN_LowerDUSel_0},
		{"get_MAIN_DisengageTestSelector_0", get_MAIN_DisengageTestSelector_0},
		{"get_MAIN_MainPanelDUSel_1", get_MAIN_MainPanelDUSel_1},
		{"get_MAIN_LowerDUSel_1", get_MAIN_LowerDUSel_1},
		{"get_MAIN_DisengageTestSelector_1", get_MAIN_DisengageTestSelector_1},
		{"get_MAIN_LightsSelector", get_MAIN_LightsSelector},
		{"get_MAIN_N1SetSelector", get_MAIN_N1SetSelector},
		{"get_MAIN_SpdRefSelector", get_MAIN_SpdRefSelector},
		{"get_MAIN_FuelFlowSelector", get_MAIN_FuelFlowSelector},
		{"get_MAIN_AutobrakeSelector", get_MAIN_AutobrakeSelector},
		{"get_MAIN_GearLever", get_MAIN_GearLever},
		{"get_LTS_MainPanelKnob_0", get_LTS_MainPanelKnob_0},
		{"get_LTS_MainPanelKnob_1", get_LTS_MainPanelKnob_1},
		{"get_LTS_BackgroundKnob", get_LTS_BackgroundKnob},
		{"get_LTS_AFDSFloodKnob", get_LTS_AFDSFloodKnob},
		{"get_LTS_OutbdDUBrtKnob_0", get_LTS_OutbdDUBrtKnob_0},
		{"get_LTS_InbdDUBrtKnob_0", get_LTS_InbdDUBrtKnob_0},
		{"get_LTS_InbdDUMapBrtKnob_0", get_LTS_InbdDUMapBrtKnob_0},
		{"get_LTS_OutbdDUBrtKnob_1", get_LTS_OutbdDUBrtKnob_1},
		{"get_LTS_InbdDUBrtKnob_1", get_LTS_InbdDUBrtKnob_1},
		{"get_LTS_InbdDUMapBrtKnob_1", get_LTS_InbdDUMapBrtKnob_1},
		{"get_LTS_UpperDUBrtKnob", get_LTS_UpperDUBrtKnob},
		{"get_LTS_LowerDUBrtKnob", get_LTS_LowerDUBrtKnob},
		{"get_LTS_LowerDUMapBrtKnob", get_LTS_LowerDUMapBrtKnob},
		{"get_CDU_BrtKnob_0", get_CDU_BrtKnob_0},
		{"get_FIRE_OvhtDetSw_0", get_FIRE_OvhtDetSw_0},
		{"get_CDU_BrtKnob_1", get_CDU_BrtKnob_1},
		{"get_FIRE_OvhtDetSw_1", get_FIRE_OvhtDetSw_1},
		{"get_FIRE_DetTestSw", get_FIRE_DetTestSw},
		{"get_FIRE_HandlePos_0", get_FIRE_HandlePos_0},
		{"get_FIRE_HandlePos_1", get_FIRE_HandlePos_1},
		{"get_FIRE_HandlePos_2", get_FIRE_HandlePos_2},
		{"get_FIRE_ExtinguisherTestSw", get_FIRE_ExtinguisherTestSw},
		{"get_CARGO_DetSelect_0", get_CARGO_DetSelect_0},
		{"get_CARGO_DetSelect_1", get_CARGO_DetSelect_1},
		{"get_XPDR_ModeSel", get_XPDR_ModeSel},
		{"get_LTS_PedFloodKnob", get_LTS_PedFloodKnob},
		{"get_LTS_PedPanelKnob", get_LTS_PedPanelKnob},
		{"get_PED_FltDkDoorSel", get_PED_FltDkDoorSel},
		{"get_FMC_TakeoffFlaps", get_FMC_TakeoffFlaps},
		{"get_FMC_V1", get_FMC_V1},
		{"get_FMC_VR", get_FMC_VR},
		{"get_FMC_V2", get_FMC_V2},
		{"get_FMC_LandingFlaps", get_FMC_LandingFlaps},
		{"get_FMC_LandingVREF", get_FMC_LandingVREF},
		{"get_IRS_SysDisplay_R", get_IRS_SysDisplay_R},
		{"get_IRS_annunGPS", get_IRS_annunGPS},
		{"get_IRS_annunALIGN_0", get_IRS_annunALIGN_0},
		{"get_IRS_annunON_DC_0", get_IRS_annunON_DC_0},
		{"get_IRS_annunFAULT_0", get_IRS_annunFAULT_0},
		{"get_IRS_annunDC_FAIL_0", get_IRS_annunDC_FAIL_0},
		{"get_IRS_annunALIGN_1", get_IRS_annunALIGN_1},
		{"get_IRS_annunON_DC_1", get_IRS_annunON_DC_1},
		{"get_IRS_annunFAULT_1", get_IRS_annunFAULT_1},
		{"get_IRS_annunDC_FAIL_1", get_IRS_annunDC_FAIL_1},
		{"get_WARN_annunPSEU", get_WARN_annunPSEU},
		{"get_COMM_ServiceInterphoneSw", get_COMM_ServiceInterphoneSw},
		{"get_ENG_EECSwitch_0", get_ENG_EECSwitch_0},
		{"get_ENG_annunREVERSER_0", get_ENG_annunREVERSER_0},
		{"get_ENG_annunENGINE_CONTROL_0", get_ENG_annunENGINE_CONTROL_0},
		{"get_ENG_annunALTN_0", get_ENG_annunALTN_0},
		{"get_ENG_EECSwitch_1", get_ENG_EECSwitch_1},
		{"get_ENG_annunREVERSER_1", get_ENG_annunREVERSER_1},
		{"get_ENG_annunENGINE_CONTROL_1", get_ENG_annunENGINE_CONTROL_1},
		{"get_ENG_annunALTN_1", get_ENG_annunALTN_1},
		{"get_OXY_SwNormal", get_OXY_SwNormal},
		{"get_OXY_annunPASS_OXY_ON", get_OXY_annunPASS_OXY_ON},
		{"get_GEAR_annunOvhdLEFT", get_GEAR_annunOvhdLEFT},
		{"get_GEAR_annunOvhdNOSE", get_GEAR_annunOvhdNOSE},
		{"get_GEAR_annunOvhdRIGHT", get_GEAR_annunOvhdRIGHT},
		{"get_FLTREC_SwNormal", get_FLTREC_SwNormal},
		{"get_FLTREC_annunOFF", get_FLTREC_annunOFF},
		{"get_FCTL_Spoiler_Sw_0", get_FCTL_Spoiler_Sw_0},
		{"get_FCTL_Spoiler_Sw_1", get_FCTL_Spoiler_Sw_1},
		{"get_FCTL_YawDamper_Sw", get_FCTL_YawDamper_Sw},
		{"get_FCTL_AltnFlaps_Sw_ARM", get_FCTL_AltnFlaps_Sw_ARM},
		{"get_FCTL_annunFC_LOW_PRESSURE_0", get_FCTL_annunFC_LOW_PRESSURE_0},
		{"get_FCTL_annunFC_LOW_PRESSURE_1", get_FCTL_annunFC_LOW_PRESSURE_1},
		{"get_FCTL_annunYAW_DAMPER", get_FCTL_annunYAW_DAMPER},
		{"get_FCTL_annunLOW_QUANTITY", get_FCTL_annunLOW_QUANTITY},
		{"get_FCTL_annunLOW_PRESSURE", get_FCTL_annunLOW_PRESSURE},
		{"get_FCTL_annunLOW_STBY_RUD_ON", get_FCTL_annunLOW_STBY_RUD_ON},
		{"get_FCTL_annunFEEL_DIFF_PRESS", get_FCTL_annunFEEL_DIFF_PRESS},
		{"get_FCTL_annunSPEED_TRIM_FAIL", get_FCTL_annunSPEED_TRIM_FAIL},
		{"get_FCTL_annunMACH_TRIM_FAIL", get_FCTL_annunMACH_TRIM_FAIL},
		{"get_FCTL_annunAUTO_SLAT_FAIL", get_FCTL_annunAUTO_SLAT_FAIL},
		{"get_FUEL_CrossFeedSw", get_FUEL_CrossFeedSw},
		{"get_FUEL_PumpFwdSw_0", get_FUEL_PumpFwdSw_0},
		{"get_FUEL_PumpAftSw_0", get_FUEL_PumpAftSw_0},
		{"get_FUEL_PumpCtrSw_0", get_FUEL_PumpCtrSw_0},
		{"get_FUEL_PumpFwdSw_1", get_FUEL_PumpFwdSw_1},
		{"get_FUEL_PumpAftSw_1", get_FUEL_PumpAftSw_1},
		{"get_FUEL_PumpCtrSw_1", get_FUEL_PumpCtrSw_1},
		{"get_FUEL_annunENG_VALVE_CLOSED_0", get_FUEL_annunENG_VALVE_CLOSED_0},
		{"get_FUEL_annunSPAR_VALVE_CLOSED_0", get_FUEL_annunSPAR_VALVE_CLOSED_0},
		{"get_FUEL_annunFILTER_BYPASS_0", get_FUEL_annunFILTER_BYPASS_0},
		{"get_FUEL_annunENG_VALVE_CLOSED_1", get_FUEL_annunENG_VALVE_CLOSED_1},
		{"get_FUEL_annunSPAR_VALVE_CLOSED_1", get_FUEL_annunSPAR_VALVE_CLOSED_1},
		{"get_FUEL_annunFILTER_BYPASS_1", get_FUEL_annunFILTER_BYPASS_1},
		{"get_FUEL_annunXFEED_VALVE_OPEN", get_FUEL_annunXFEED_VALVE_OPEN},
		{"get_FUEL_annunLOWPRESS_Fwd_0", get_FUEL_annunLOWPRESS_Fwd_0},
		{"get_FUEL_annunLOWPRESS_Aft_0", get_FUEL_annunLOWPRESS_Aft_0},
		{"get_FUEL_annunLOWPRESS_Ctr_0", get_FUEL_annunLOWPRESS_Ctr_0},
		{"get_FUEL_annunLOWPRESS_Fwd_1", get_FUEL_annunLOWPRESS_Fwd_1},
		{"get_FUEL_annunLOWPRESS_Aft_1", get_FUEL_annunLOWPRESS_Aft_1},
		{"get_FUEL_annunLOWPRESS_Ctr_1", get_FUEL_annunLOWPRESS_Ctr_1},
		{"get_ELEC_annunBAT_DISCHARGE", get_ELEC_annunBAT_DISCHARGE},
		{"get_ELEC_annunTR_UNIT", get_ELEC_annunTR_UNIT},
		{"get_ELEC_annunELEC", get_ELEC_annunELEC},
		{"get_ELEC_CabUtilSw", get_ELEC_CabUtilSw},
		{"get_ELEC_IFEPassSeatSw", get_ELEC_IFEPassSeatSw},
		{"get_ELEC_annunDRIVE_0", get_ELEC_annunDRIVE_0},
		{"get_ELEC_annunDRIVE_1", get_ELEC_annunDRIVE_1},
		{"get_ELEC_annunSTANDBY_POWER_OFF", get_ELEC_annunSTANDBY_POWER_OFF},
		{"get_ELEC_IDGDisconnectSw_0", get_ELEC_IDGDisconnectSw_0},
		{"get_ELEC_IDGDisconnectSw_1", get_ELEC_IDGDisconnectSw_1},
		{"get_ELEC_annunGRD_POWER_AVAILABLE", get_ELEC_annunGRD_POWER_AVAILABLE},
		{"get_ELEC_GrdPwrSw", get_ELEC_GrdPwrSw},
		{"get_ELEC_BusTransSw_AUTO", get_ELEC_BusTransSw_AUTO},
		{"get_ELEC_GenSw_0", get_ELEC_GenSw_0},
		{"get_ELEC_APUGenSw_0", get_ELEC_APUGenSw_0},
		{"get_ELEC_annunTRANSFER_BUS_OFF_0", get_ELEC_annunTRANSFER_BUS_OFF_0},
		{"get_ELEC_annunSOURCE_OFF_0", get_ELEC_annunSOURCE_OFF_0},
		{"get_ELEC_annunGEN_BUS_OFF_0", get_ELEC_annunGEN_BUS_OFF_0},
		{"get_ELEC_GenSw_1", get_ELEC_GenSw_1},
		{"get_ELEC_APUGenSw_1", get_ELEC_APUGenSw_1},
		{"get_ELEC_annunTRANSFER_BUS_OFF_1", get_ELEC_annunTRANSFER_BUS_OFF_1},
		{"get_ELEC_annunSOURCE_OFF_1", get_ELEC_annunSOURCE_OFF_1},
		{"get_ELEC_annunGEN_BUS_OFF_1", get_ELEC_annunGEN_BUS_OFF_1},
		{"get_ELEC_annunAPU_GEN_OFF_BUS", get_ELEC_annunAPU_GEN_OFF_BUS},
		{"get_APU_annunMAINT", get_APU_annunMAINT},
		{"get_APU_annunLOW_OIL_PRESSURE", get_APU_annunLOW_OIL_PRESSURE},
		{"get_APU_annunFAULT", get_APU_annunFAULT},
		{"get_APU_annunOVERSPEED", get_APU_annunOVERSPEED},
		{"get_AIR_EquipCoolingSupplyNORM", get_AIR_EquipCoolingSupplyNORM},
		{"get_AIR_EquipCoolingExhaustNORM", get_AIR_EquipCoolingExhaustNORM},
		{"get_AIR_annunEquipCoolingSupplyOFF", get_AIR_annunEquipCoolingSupplyOFF},
		{"get_AIR_annunEquipCoolingExhaustOFF", get_AIR_annunEquipCoolingExhaustOFF},
		{"get_LTS_annunEmerNOT_ARMED", get_LTS_annunEmerNOT_ARMED},
		{"get_COMM_annunCALL", get_COMM_annunCALL},
		{"get_COMM_annunPA_IN_USE", get_COMM_annunPA_IN_USE},
		{"get_ICE_annunOVERHEAT_0", get_ICE_annunOVERHEAT_0},
		{"get_ICE_annunOVERHEAT_1", get_ICE_annunOVERHEAT_1},
		{"get_ICE_annunOVERHEAT_2", get_ICE_annunOVERHEAT_2},
		{"get_ICE_annunOVERHEAT_3", get_ICE_annunOVERHEAT_3},
		{"get_ICE_annunON_0", get_ICE_annunON_0},
		{"get_ICE_WindowHeatSw_0", get_ICE_WindowHeatSw_0},
		{"get_ICE_annunON_1", get_ICE_annunON_1},
		{"get_ICE_WindowHeatSw_1", get_ICE_WindowHeatSw_1},
		{"get_ICE_annunON_2", get_ICE_annunON_2},
		{"get_ICE_WindowHeatSw_2", get_ICE_WindowHeatSw_2},
		{"get_ICE_annunON_3", get_ICE_annunON_3},
		{"get_ICE_WindowHeatSw_3", get_ICE_WindowHeatSw_3},
		{"get_ICE_annunCAPT_PITOT", get_ICE_annunCAPT_PITOT},
		{"get_ICE_annunL_ELEV_PITOT", get_ICE_annunL_ELEV_PITOT},
		{"get_ICE_annunL_ALPHA_VANE", get_ICE_annunL_ALPHA_VANE},
		{"get_ICE_annunL_TEMP_PROBE", get_ICE_annunL_TEMP_PROBE},
		{"get_ICE_annunFO_PITOT", get_ICE_annunFO_PITOT},
		{"get_ICE_annunR_ELEV_PITOT", get_ICE_annunR_ELEV_PITOT},
		{"get_ICE_annunR_ALPHA_VANE", get_ICE_annunR_ALPHA_VANE},
		{"get_ICE_annunAUX_PITOT", get_ICE_annunAUX_PITOT},
		{"get_ICE_TestProbeHeatSw_0", get_ICE_TestProbeHeatSw_0},
		{"get_ICE_annunVALVE_OPEN_0", get_ICE_annunVALVE_OPEN_0},
		{"get_ICE_annunCOWL_ANTI_ICE_0", get_ICE_annunCOWL_ANTI_ICE_0},
		{"get_ICE_annunCOWL_VALVE_OPEN_0", get_ICE_annunCOWL_VALVE_OPEN_0},
		{"get_ICE_TestProbeHeatSw_1", get_ICE_TestProbeHeatSw_1},
		{"get_ICE_annunVALVE_OPEN_1", get_ICE_annunVALVE_OPEN_1},
		{"get_ICE_annunCOWL_ANTI_ICE_1", get_ICE_annunCOWL_ANTI_ICE_1},
		{"get_ICE_annunCOWL_VALVE_OPEN_1", get_ICE_annunCOWL_VALVE_OPEN_1},
		{"get_ICE_WingAntiIceSw", get_ICE_WingAntiIceSw},
		{"get_ICE_EngAntiIceSw_0", get_ICE_EngAntiIceSw_0},
		{"get_HYD_annunLOW_PRESS_eng_0", get_HYD_annunLOW_PRESS_eng_0},
		{"get_HYD_annunLOW_PRESS_elec_0", get_HYD_annunLOW_PRESS_elec_0},
		{"get_HYD_annunOVERHEAT_elec_0", get_HYD_annunOVERHEAT_elec_0},
		{"get_HYD_PumpSw_eng_0", get_HYD_PumpSw_eng_0},
		{"get_HYD_PumpSw_elec_0", get_HYD_PumpSw_elec_0},
		{"get_ICE_EngAntiIceSw_1", get_ICE_EngAntiIceSw_1},
		{"get_HYD_annunLOW_PRESS_eng_1", get_HYD_annunLOW_PRESS_eng_1},
		{"get_HYD_annunLOW_PRESS_elec_1", get_HYD_annunLOW_PRESS_elec_1},
		{"get_HYD_annunOVERHEAT_elec_1", get_HYD_annunOVERHEAT_elec_1},
		{"get_HYD_PumpSw_eng_1", get_HYD_PumpSw_eng_1},
		{"get_HYD_PumpSw_elec_1", get_HYD_PumpSw_elec_1},
		{"get_AIR_TrimAirSwitch", get_AIR_TrimAirSwitch},
		{"get_AIR_annunZoneTemp_0", get_AIR_annunZoneTemp_0},
		{"get_AIR_annunZoneTemp_1", get_AIR_annunZoneTemp_1},
		{"get_AIR_annunZoneTemp_2", get_AIR_annunZoneTemp_2},
		{"get_AIR_annunDualBleed", get_AIR_annunDualBleed},
		{"get_AIR_annunRamDoorL", get_AIR_annunRamDoorL},
		{"get_AIR_annunRamDoorR", get_AIR_annunRamDoorR},
		{"get_AIR_RecircFanSwitch_0", get_AIR_RecircFanSwitch_0},
		{"get_AIR_BleedAirSwitch_0", get_AIR_BleedAirSwitch_0},
		{"get_AIR_RecircFanSwitch_1", get_AIR_RecircFanSwitch_1},
		{"get_AIR_BleedAirSwitch_1", get_AIR_BleedAirSwitch_1},
		{"get_AIR_APUBleedAirSwitch", get_AIR_APUBleedAirSwitch},
		{"get_AIR_IsolationValveSwitch", get_AIR_IsolationValveSwitch},
		{"get_AIR_annunPackTripOff_0", get_AIR_annunPackTripOff_0},
		{"get_AIR_annunWingBodyOverheat_0", get_AIR_annunWingBodyOverheat_0},
		{"get_AIR_annunBleedTripOff_0", get_AIR_annunBleedTripOff_0},
		{"get_LTS_LandingLtFixedSw_0", get_LTS_LandingLtFixedSw_0},
		{"get_LTS_RunwayTurnoffSw_0", get_LTS_RunwayTurnoffSw_0},
		{"get_AIR_annunPackTripOff_1", get_AIR_annunPackTripOff_1},
		{"get_AIR_annunWingBodyOverheat_1", get_AIR_annunWingBodyOverheat_1},
		{"get_AIR_annunBleedTripOff_1", get_AIR_annunBleedTripOff_1},
		{"get_LTS_LandingLtFixedSw_1", get_LTS_LandingLtFixedSw_1},
		{"get_LTS_RunwayTurnoffSw_1", get_LTS_RunwayTurnoffSw_1},
		{"get_LTS_TaxiSw", get_LTS_TaxiSw},
		{"get_LTS_LogoSw", get_LTS_LogoSw},
		{"get_LTS_AntiCollisionSw", get_LTS_AntiCollisionSw},
		{"get_LTS_WingSw", get_LTS_WingSw},
		{"get_LTS_WheelWellSw", get_LTS_WheelWellSw},
		{"get_WARN_annunFIRE_WARN_0", get_WARN_annunFIRE_WARN_0},
		{"get_WARN_annunMASTER_CAUTION_0", get_WARN_annunMASTER_CAUTION_0},
		{"get_WARN_annunFIRE_WARN_1", get_WARN_annunFIRE_WARN_1},
		{"get_WARN_annunMASTER_CAUTION_1", get_WARN_annunMASTER_CAUTION_1},
		{"get_WARN_annunFLT_CONT", get_WARN_annunFLT_CONT},
		{"get_WARN_annunIRS", get_WARN_annunIRS},
		{"get_WARN_annunFUEL", get_WARN_annunFUEL},
		{"get_WARN_annunELEC", get_WARN_annunELEC},
		{"get_WARN_annunAPU", get_WARN_annunAPU},
		{"get_WARN_annunOVHT_DET", get_WARN_annunOVHT_DET},
		{"get_WARN_annunANTI_ICE", get_WARN_annunANTI_ICE},
		{"get_WARN_annunHYD", get_WARN_annunHYD},
		{"get_WARN_annunDOORS", get_WARN_annunDOORS},
		{"get_WARN_annunENG", get_WARN_annunENG},
		{"get_WARN_annunOVERHEAD", get_WARN_annunOVERHEAD},
		{"get_WARN_annunAIR_COND", get_WARN_annunAIR_COND},
		{"get_EFIS_MinsSelBARO_0", get_EFIS_MinsSelBARO_0},
		{"get_EFIS_BaroSelHPA_0", get_EFIS_BaroSelHPA_0},
		{"get_EFIS_MinsSelBARO_1", get_EFIS_MinsSelBARO_1},
		{"get_EFIS_BaroSelHPA_1", get_EFIS_BaroSelHPA_1},
		{"get_MCP_IASBlank", get_MCP_IASBlank},
		{"get_MCP_IASOverspeedFlash", get_MCP_IASOverspeedFlash},
		{"get_MCP_IASUnderspeedFlash", get_MCP_IASUnderspeedFlash},
		{"get_MCP_VertSpeedBlank", get_MCP_VertSpeedBlank},
		{"get_MCP_FDSw_0", get_MCP_FDSw_0},
		{"get_MCP_FDSw_1", get_MCP_FDSw_1},
		{"get_MCP_ATArmSw", get_MCP_ATArmSw},
		{"get_MCP_DisengageBar", get_MCP_DisengageBar},
		{"get_MCP_annunFD_0", get_MCP_annunFD_0},
		{"get_MCP_annunFD_1", get_MCP_annunFD_1},
		{"get_MCP_annunATArm", get_MCP_annunATArm},
		{"get_MCP_annunN1", get_MCP_annunN1},
		{"get_MCP_annunSPEED", get_MCP_annunSPEED},
		{"get_MCP_annunVNAV", get_MCP_annunVNAV},
		{"get_MCP_annunLVL_CHG", get_MCP_annunLVL_CHG},
		{"get_MCP_annunHDG_SEL", get_MCP_annunHDG_SEL},
		{"get_MCP_annunLNAV", get_MCP_annunLNAV},
		{"get_MCP_annunVOR_LOC", get_MCP_annunVOR_LOC},
		{"get_MCP_annunAPP", get_MCP_annunAPP},
		{"get_MCP_annunALT_HOLD", get_MCP_annunALT_HOLD},
		{"get_MCP_annunVS", get_MCP_annunVS},
		{"get_MCP_annunCMD_A", get_MCP_annunCMD_A},
		{"get_MCP_annunCWS_A", get_MCP_annunCWS_A},
		{"get_MCP_annunCMD_B", get_MCP_annunCMD_B},
		{"get_MCP_annunCWS_B", get_MCP_annunCWS_B},
		{"get_MAIN_NoseWheelSteeringSwNORM", get_MAIN_NoseWheelSteeringSwNORM},
		{"get_MAIN_annunBELOW_GS_0", get_MAIN_annunBELOW_GS_0},
		{"get_MAIN_annunAP_0", get_MAIN_annunAP_0},
		{"get_MAIN_annunAT_0", get_MAIN_annunAT_0},
		{"get_MAIN_annunFMC_0", get_MAIN_annunFMC_0},
		{"get_MAIN_annunBELOW_GS_1", get_MAIN_annunBELOW_GS_1},
		{"get_MAIN_annunAP_1", get_MAIN_annunAP_1},
		{"get_MAIN_annunAT_1", get_MAIN_annunAT_1},
		{"get_MAIN_annunFMC_1", get_MAIN_annunFMC_1},
		{"get_MAIN_annunSPEEDBRAKE_ARMED", get_MAIN_annunSPEEDBRAKE_ARMED},
		{"get_MAIN_annunSPEEDBRAKE_DO_NOT_ARM", get_MAIN_annunSPEEDBRAKE_DO_NOT_ARM},
		{"get_MAIN_annunSPEEDBRAKE_EXTENDED", get_MAIN_annunSPEEDBRAKE_EXTENDED},
		{"get_MAIN_annunSTAB_OUT_OF_TRIM", get_MAIN_annunSTAB_OUT_OF_TRIM},
		{"get_MAIN_RMISelector1_VOR", get_MAIN_RMISelector1_VOR},
		{"get_MAIN_RMISelector2_VOR", get_MAIN_RMISelector2_VOR},
		{"get_MAIN_annunANTI_SKID_INOP", get_MAIN_annunANTI_SKID_INOP},
		{"get_MAIN_annunAUTO_BRAKE_DISARM", get_MAIN_annunAUTO_BRAKE_DISARM},
		{"get_MAIN_annunLE_FLAPS_TRANSIT", get_MAIN_annunLE_FLAPS_TRANSIT},
		{"get_MAIN_annunLE_FLAPS_EXT", get_MAIN_annunLE_FLAPS_EXT},
		{"get_MAIN_annunGEAR_transit_0", get_MAIN_annunGEAR_transit_0},
		{"get_MAIN_annunGEAR_locked_0", get_MAIN_annunGEAR_locked_0},
		{"get_MAIN_annunGEAR_transit_1", get_MAIN_annunGEAR_transit_1},
		{"get_MAIN_annunGEAR_locked_1", get_MAIN_annunGEAR_locked_1},
		{"get_MAIN_annunGEAR_transit_2", get_MAIN_annunGEAR_transit_2},
		{"get_MAIN_annunGEAR_locked_2", get_MAIN_annunGEAR_locked_2},
		{"get_HGS_annun_AIII", get_HGS_annun_AIII},
		{"get_HGS_annun_NO_AIII", get_HGS_annun_NO_AIII},
		{"get_HGS_annun_FLARE", get_HGS_annun_FLARE},
		{"get_HGS_annun_RO", get_HGS_annun_RO},
		{"get_HGS_annun_RO_CTN", get_HGS_annun_RO_CTN},
		{"get_HGS_annun_RO_ARM", get_HGS_annun_RO_ARM},
		{"get_HGS_annun_TO", get_HGS_annun_TO},
		{"get_HGS_annun_TO_CTN", get_HGS_annun_TO_CTN},
		{"get_HGS_annun_APCH", get_HGS_annun_APCH},
		{"get_HGS_annun_TO_WARN", get_HGS_annun_TO_WARN},
		{"get_HGS_annun_Bar", get_HGS_annun_Bar},
		{"get_HGS_annun_FAIL", get_HGS_annun_FAIL},
		{"get_GPWS_annunINOP", get_GPWS_annunINOP},
		{"get_GPWS_FlapInhibitSw_NORM", get_GPWS_FlapInhibitSw_NORM},
		{"get_GPWS_GearInhibitSw_NORM", get_GPWS_GearInhibitSw_NORM},
		{"get_GPWS_TerrInhibitSw_NORM", get_GPWS_TerrInhibitSw_NORM},
		{"get_CDU_annunEXEC_0", get_CDU_annunEXEC_0},
		{"get_CDU_annunCALL_0", get_CDU_annunCALL_0},
		{"get_CDU_annunFAIL_0", get_CDU_annunFAIL_0},
		{"get_CDU_annunMSG_0", get_CDU_annunMSG_0},
		{"get_CDU_annunOFST_0", get_CDU_annunOFST_0},
		{"get_CDU_annunEXEC_1", get_CDU_annunEXEC_1},
		{"get_CDU_annunCALL_1", get_CDU_annunCALL_1},
		{"get_CDU_annunFAIL_1", get_CDU_annunFAIL_1},
		{"get_CDU_annunMSG_1", get_CDU_annunMSG_1},
		{"get_CDU_annunOFST_1", get_CDU_annunOFST_1},
		{"get_TRIM_StabTrimMainElecSw_NORMAL", get_TRIM_StabTrimMainElecSw_NORMAL},
		{"get_TRIM_StabTrimAutoPilotSw_NORMAL", get_TRIM_StabTrimAutoPilotSw_NORMAL},
		{"get_PED_annunParkingBrake", get_PED_annunParkingBrake},
		{"get_FIRE_annunENG_OVERHEAT_0", get_FIRE_annunENG_OVERHEAT_0},
		{"get_FIRE_annunENG_OVERHEAT_1", get_FIRE_annunENG_OVERHEAT_1},
		{"get_FIRE_HandleIlluminated_0", get_FIRE_HandleIlluminated_0},
		{"get_FIRE_HandleIlluminated_1", get_FIRE_HandleIlluminated_1},
		{"get_FIRE_HandleIlluminated_2", get_FIRE_HandleIlluminated_2},
		{"get_FIRE_annunWHEEL_WELL", get_FIRE_annunWHEEL_WELL},
		{"get_FIRE_annunFAULT", get_FIRE_annunFAULT},
		{"get_FIRE_annunAPU_DET_INOP", get_FIRE_annunAPU_DET_INOP},
		{"get_FIRE_annunAPU_BOTTLE_DISCHARGE", get_FIRE_annunAPU_BOTTLE_DISCHARGE},
		{"get_FIRE_annunBOTTLE_DISCHARGE_0", get_FIRE_annunBOTTLE_DISCHARGE_0},
		{"get_FIRE_annunBOTTLE_DISCHARGE_1", get_FIRE_annunBOTTLE_DISCHARGE_1},
		{"get_FIRE_annunExtinguisherTest_0", get_FIRE_annunExtinguisherTest_0},
		{"get_FIRE_annunExtinguisherTest_1", get_FIRE_annunExtinguisherTest_1},
		{"get_FIRE_annunExtinguisherTest_2", get_FIRE_annunExtinguisherTest_2},
		{"get_CARGO_annunExtTest_0", get_CARGO_annunExtTest_0},
		{"get_CARGO_ArmedSw_0", get_CARGO_ArmedSw_0},
		{"get_CARGO_annunExtTest_1", get_CARGO_annunExtTest_1},
		{"get_CARGO_ArmedSw_1", get_CARGO_ArmedSw_1},
		{"get_CARGO_annunFWD", get_CARGO_annunFWD},
		{"get_CARGO_annunAFT", get_CARGO_annunAFT},
		{"get_CARGO_annunDETECTOR_FAULT", get_CARGO_annunDETECTOR_FAULT},
		{"get_CARGO_annunDISCH", get_CARGO_annunDISCH},
		{"get_HGS_annunRWY", get_HGS_annunRWY},
		{"get_HGS_annunGS", get_HGS_annunGS},
		{"get_HGS_annunFAULT", get_HGS_annunFAULT},
		{"get_HGS_annunCLR", get_HGS_annunCLR},
		{"get_XPDR_XpndrSelector_2", get_XPDR_XpndrSelector_2},
		{"get_XPDR_AltSourceSel_2", get_XPDR_AltSourceSel_2},
		{"get_XPDR_annunFAIL", get_XPDR_annunFAIL},
		{"get_TRIM_StabTrimSw_NORMAL", get_TRIM_StabTrimSw_NORMAL},
		{"get_PED_annunLOCK_FAIL", get_PED_annunLOCK_FAIL},
		{"get_PED_annunAUTO_UNLK", get_PED_annunAUTO_UNLK},
		{"get_ENG_StartValve_0", get_ENG_StartValve_0},
		{"get_ENG_StartValve_1", get_ENG_StartValve_1},
		{"get_IRS_aligned", get_IRS_aligned},
		{"get_WeightInKg", get_WeightInKg},
		{"get_GPWS_V1CallEnabled", get_GPWS_V1CallEnabled},
		{"get_GroundConnAvailable", get_GroundConnAvailable},
		{"get_FMC_PerfInputComplete", get_FMC_PerfInputComplete},
		{"get_FMC_flightNumber_0", get_FMC_flightNumber_0},
		{"get_FMC_flightNumber_1", get_FMC_flightNumber_1},
		{"get_FMC_flightNumber_2", get_FMC_flightNumber_2},
		{"get_FMC_flightNumber_3", get_FMC_flightNumber_3},
		{"get_FMC_flightNumber_4", get_FMC_flightNumber_4},
		{"get_FMC_flightNumber_5", get_FMC_flightNumber_5},
		{"get_FMC_flightNumber_6", get_FMC_flightNumber_6},
		{"get_FMC_flightNumber_7", get_FMC_flightNumber_7},
		{"get_FMC_flightNumber_8", get_FMC_flightNumber_8},
		{"get_FUEL_FuelTempNeedle", get_FUEL_FuelTempNeedle},
		{"get_APU_EGTNeedle", get_APU_EGTNeedle},
		{"get_MCP_IASMach", get_MCP_IASMach},
		{"get_MAIN_TEFlapsNeedle_0", get_MAIN_TEFlapsNeedle_0},
		{"get_MAIN_TEFlapsNeedle_1", get_MAIN_TEFlapsNeedle_1},
		{"get_MAIN_BrakePressNeedle", get_MAIN_BrakePressNeedle},
		{"get_AIR_DuctPress_0", get_AIR_DuctPress_0},
		{"get_AIR_DuctPress_1", get_AIR_DuctPress_1},
		{"get_FUEL_QtyCenter", get_FUEL_QtyCenter},
		{"get_FUEL_QtyLeft", get_FUEL_QtyLeft},
		{"get_FUEL_QtyRight", get_FUEL_QtyRight},
		{"get_FMC_DistanceToTOD", get_FMC_DistanceToTOD},
		{"get_FMC_DistanceToDest", get_FMC_DistanceToDest},
		{"get_MCP_VertSpeed", get_MCP_VertSpeed}, 
		{"get_FMC_LandingAltitude", get_FMC_LandingAltitude},
		{"get_AIR_FltAltWindow", get_AIR_FltAltWindow},
		{"get_AIR_LandAltWindow", get_AIR_LandAltWindow},
		{"get_AIR_OutflowValveSwitch", get_AIR_OutflowValveSwitch},
		{"get_AIR_PressurizationModeSelector", get_AIR_PressurizationModeSelector},
		{"get_MCP_Course_0", get_MCP_Course_0},
		{"get_MCP_Course_1", get_MCP_Course_1},
		{"get_MCP_Heading", get_MCP_Heading},
		{"get_MCP_Altitude", get_MCP_Altitude},

        {NULL,NULL}
    };

	luaL_register(L,"ngxSDK2Lua", asd);

	if (active) {
		return 0;
	}

	LuaGlobal = L;

	return 0;
}