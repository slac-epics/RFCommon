#ifndef RF_COMMON_ASYN_DRIVER_H
#define RF_COMMON_ASYN_DRIVER_H

#include <asynPortDriver.h>
#include <epicsEvent.h>
#include <epicsTypes.h>
#include <epicsTime.h>

#include <cpsw_api_user.h>
#include <rfCommon.h>
#include <vector>
#include <string>
#include <dlfcn.h>

#include <stdio.h>
#include <sstream>
#include <fstream>

#include "RFUpConverter.h"
#include "RFDownConverter.h"

#define MAX_CHN   12
#define MAX_REF   2


class RFCommonAsynDriver:asynPortDriver {

    public:
        RFCommonAsynDriver(const char *portName, const char *pathString, const char *named_root = NULL);
        ~RFCommonAsynDriver();
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
        asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);



        void report(int level);
        void poll(void);



    private:
        char *port;
        char *path;
        RFCommonFw       rfCommon;
        void ParameterSetup(void);

    protected:
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
        int firstRFCommonParam;
#define FIRST_RFCOMMON_PARAM     firstRFCommonParam
#endif /* asyn version check, under 4.32 */

        int p_demod_version;

        int p_phase[MAX_CHN];
        int p_phaseRaw[MAX_CHN];
        int p_amplitude[MAX_CHN];
        int p_amplitudeRaw[MAX_CHN];

        int p_maxHoldReset;
        int p_phaseMaxHold[MAX_CHN];
        int p_phaseRawMaxHold[MAX_CHN];
        int p_amplitudeMaxHold[MAX_CHN];
        int p_amplitudeRawMaxHold[MAX_CHN];

        int p_setRotPhase[MAX_CHN];
        int p_setRotAmplitude[MAX_CHN];
        int p_getRotI[MAX_CHN];
        int p_getRotQ[MAX_CHN];

        int p_pll_version;
        int p_loPhase;
        int p_loAmplitude;
        int p_loPhaseRaw;
        int p_loAmplitudeRaw;
        int p_loLockCount;
        int p_loLockStatus;
        int p_loLossStatus;
        int p_clockPhase;
        int p_clockAmplitude;
        int p_clockPhaseRaw;
        int p_clockAmplitudeRaw;
        int p_clockLockCount;
        int p_clockLockStatus;
        int p_clockLossStatus;
        int p_lockThreshold;
        int p_lossThreshold;
        int p_loLockCountReset;
        int p_clockLockCountReset;
        int p_loDacControlMux;
        int p_clockDacControlMux;
        int p_userDacControlMux;
        int p_loRefSelect;
        int p_clockRefSelect;
        int p_loPolarity;
        int p_clockPolarity;
        int p_loDac;
        int p_clockDac;
        int p_userDac;
        int p_loSetPhase;
        int p_clockSetPhase;
        int p_loKp;
        int p_loKi;
        int p_clockKp;
        int p_clockKi;

        int p_upConv_version;
        int p_refPhase[MAX_REF];
        int p_refPhaseRaw[MAX_REF];
        int p_refAmplitude[MAX_REF];
        int p_refAmplitudeRaw[MAX_REF];
        int p_rfEnable;
        int p_modeSelect;
        int p_loSelect;
        int p_refSelect[MAX_REF];
        int p_invertCorrection;
        int p_basebandI;
        int p_basebandQ;
        int p_setBasebandPhase;
        int p_setBasebandAmplitude;
        int p_dacOffset;
        int p_dacMin;
        int p_dacMax;

#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
        int lastRFCommonParam;
#define LAST_RFCOMMON_PARAM      lastRFCommonParam
#endif /* asyn version check, under 4.32 */


};

#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
#define NUM_RFCOMMON_DET_PARAMS ((int) (&LAST_RFCOMMON_PARAM - &FIRST_RFCOMMON_PARAM-1))
#endif /* asyn version check, under 4.32 */

// RFCommon - Democulator
#define STR_DEMOD_VER         "demod_version"
#define STR_MAXHOLD_RESET     "maxHold_reset"

#define STR_PHASE             "phase_%d"
#define STR_AMPLITUDE         "amplitude_%d"
#define STR_PHASE_RAW         "phase_raw_%d"
#define STR_AMPLITUDE_RAW     "amplitude_raw_%d"

#define STR_PHASE_MAXHOLD             "phaseMaxHold_%d"
#define STR_AMPLITUDE_MAXHOLD         "amplitudeMaxHold_%d"
#define STR_PHASE_RAW_MAXHOLD         "phase_rawMaxHold_%d"
#define STR_AMPLITUDE_RAW_MAXHOLD     "amplitude_rawMaxHold_%d"

#define STR_SET_ROT_PHASE     "set_rot_phase_%d"
#define STR_SET_ROT_AMPLITUDE "set_rot_amplitude_%d"
#define STR_GET_ROT_I         "get_rot_i_%d"
#define STR_GET_ROT_Q         "get_rot_q_%d"
// RFCommon - PLL
#define STR_PLL_VER           "pll_version"
#define STR_LO_PHASE          "lo_phase"
#define STR_LO_AMPLITUDE      "lo_amplitude"
#define STR_LO_PHASE_RAW      "lo_phase_raw"
#define STR_LO_AMPLITUDE_RAW  "lo_amplitude_raw"
#define STR_LO_LOCK_COUNT     "lo_lock_count"
#define STR_LO_LOCK_STATUS    "lo_lock_status"
#define STR_LO_LOSS_STATUS    "lo_loss_status"
#define STR_CLOCK_PHASE       "clock_phase"
#define STR_CLOCK_AMPLITUDE   "clock_amplitude"
#define STR_CLOCK_PHASE_RAW   "clock_phase_raw"
#define STR_CLOCK_AMPLITUDE_RAW "clock_amplitude_raw"
#define STR_CLOCK_LOCK_COUNT    "clock_lock_count"
#define STR_CLOCK_LOCK_STATUS   "clock_lock_status"
#define STR_CLOCK_LOSS_STATUS   "clock_loss_status"
#define STR_LOCK_THRESHOLD      "lock_threshold"
#define STR_LOSS_THRESHOLD      "loss_threshold"
#define STR_LO_LOCK_COUNT_RESET    "lo_lock_count_reset"
#define STR_CLOCK_LOCK_COUNT_RESET "clock_lock_count_reset"
#define STR_LO_DAC_CONTROL_MUX     "lo_dac_control_mux"
#define STR_CLOCK_DAC_CONTROL_MUX  "clock_dac_control_mux"
#define STR_USER_DAC_CONTROL_MUX    "user_dac_control_mux"
#define STR_LO_REF_SELECT           "lo_ref_select"
#define STR_CLOCK_REF_SELECT        "clock_ref_select"
#define STR_LO_POLARITY             "lo_polarity"
#define STR_CLOCK_POLARITY          "clock_polarity"
#define STR_LO_DAC                  "lo_dac"
#define STR_CLOCK_DAC               "clock_dac"
#define STR_USER_DAC                "user_dac"
#define STR_LO_SET_PHASE            "lo_set_phase"
#define STR_CLOCK_SET_PHASE         "clock_set_phase"
#define STR_LO_KP                   "lo_kp"
#define STR_LO_KI                   "lo_ki"
#define STR_CLOCK_KP                "clock_kp"
#define STR_CLOCK_KI                "clock_ki"
// RFCommon - UpConverter
#define STR_UPCONV_VERSION          "upconv_version"
#define STR_REF_PHASE               "ref_phase_%d"
#define STR_REF_PHASE_RAW           "ref_phase_raw_%d"
#define STR_REF_AMPLITUDE           "ref_amplitude_%d"
#define STR_REF_AMPLITUDE_RAW       "ref_amplitude_raw_%d"
#define STR_RF_ENABLE               "rf_enable"
#define STR_MODE_SELECT             "mode_select"
#define STR_LO_SELECT               "lo_select"
#define STR_REF_SELECT              "ref_select_%d"
#define STR_INVERT_CORRECTION       "invert_correction"
#define STR_BASEBAND_I              "baseband_i"
#define STR_BASEBAND_Q              "baseband_q"
#define STR_SET_BASEBAND_PHASE      "set_baseband_phase"
#define STR_SET_BASEBAND_AMPLITUDE  "set_baseband_amplitude"
#define STR_DAC_OFFSET              "dac_offset"
#define STR_DAC_MIN                 "dac_min"
#define STR_DAC_MAX                 "dac_max"



typedef struct {
    ELLNODE     node;
    char        *named_root;
    char        *portName;
    char        *pathName;
    RFCommonAsynDriver *pDrv;
    upConvNode_t       *pUpConv;
    downConvNode_t     *pDownConv;
}   drvNode_t;

drvNode_t* last_drvList_RFCommon(void);


#endif /* RF_COMMON_ASYN_DRIVER_H */
