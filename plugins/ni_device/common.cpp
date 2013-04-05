//
//  common.cpp
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#include "common.h"

void configureTimebase (tMSeries* board)
{
    board->Clock_and_FOUT.setSlow_Internal_Timebase(1);
    board->Clock_and_FOUT.flush();
    
    return;
}

void pllReset (tMSeries* board)
{
    board->Clock_And_Fout2.setTB1_Select(tMSeries::tClock_And_Fout2::kTB1_SelectSelect_OSC);
    board->Clock_And_Fout2.setTB3_Select(tMSeries::tClock_And_Fout2::kTB3_SelectSelect_OSC);
    board->Clock_And_Fout2.flush();
    
    board->PLL_Control.setPLL_Enable (kFalse);
    board->PLL_Control.flush ();
    
    return;
}

void analogTriggerReset (tMSeries* board)
{
    board->Analog_Trigger_Etc.setAnalog_Trigger_Reset(1);
    board->Analog_Trigger_Etc.setAnalog_Trigger_Mode(tMSeries::tAnalog_Trigger_Etc::kAnalog_Trigger_ModeLow_Window);
    board->Analog_Trigger_Etc.flush();
    
    board->Analog_Trigger_Control.setAnalog_Trigger_Select(tMSeries::tAnalog_Trigger_Control::kAnalog_Trigger_SelectGround);
    board->Analog_Trigger_Control.flush();
    
    board->Gen_PWM[0].writeRegister(0);
    board->Gen_PWM[1].writeRegister(0);
    
    board->Analog_Trigger_Etc.setAnalog_Trigger_Enable(tMSeries::tAnalog_Trigger_Etc::kAnalog_Trigger_EnableDisabled);
    board->Analog_Trigger_Etc.flush();
    
    return; 
}


