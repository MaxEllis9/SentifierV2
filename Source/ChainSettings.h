/*
  ==============================================================================

    ChainSettings.h
    Created: 20 Nov 2022 6:31:43pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

struct ChainSettings{
    
    float lowCutFreq {0}, highCutFreq {0}, inputgain {0}, outputgain {0}, lowDrive {0}, midDrive {0},
    highDrive {0}, mix {0}, lowBandFreq {0}, highBandFreq {0};
    int lowDistortionMode {0}, midDistortionMode {0}, highDistortionMode {0};
    bool powerSwitch {true}, lowCutBypassed {false}, highCutBypassed {false},
    lowBandBypassed {false}, highBandBypassed {false}, midBandBypassed {false}, lowBandSolo {false},
    highBandSolo {false}, midBandSolo {false}, lowBandMute {false}, highBandMute {false}, midBandMute {false};
};
