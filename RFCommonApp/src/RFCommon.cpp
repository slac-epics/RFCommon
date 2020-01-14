#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <string>
#include <sstream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#include <cantProceed.h>
#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsPrint.h>
#include <ellLib.h>
#include <iocsh.h>

#include <yaml-cpp/yaml.h>
#include <yamlLoader.h>

#include <drvSup.h>
#include <epicsExport.h>
#include <registryFunction.h>

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

#include "RFCommon.h"




static const char *driverName  = "RFCommonAsynDriver";
static ELLLIST *pDrvList = (ELLLIST *) NULL;



RFCommonAsynDriver::RFCommonAsynDriver(const char *portName, const char *pathString, const char *named_root)
    :asynPortDriver(portName,
                     1, /* number of elements of this device */
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
                     NUM_RFCOMMON_DET_PARAMS, /* number of asyn params of be cleared for each device */
#endif /* asyn version check, under 4.32 */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask, /* Interface mask */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynEnumMask    | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask,  /* Interrupt mask */
                     1, /* asynFlags.  This driver does block and it is not multi-device, so flag is 1 */
                     1, /* Autoconnect */
                     0, /* Default priority */
                     0) /* Default stack size*/
                    
{
    Path p_root;
    Path p_rfCommon;

    port = epicsStrDup(portName);
    path = epicsStrDup(pathString);

    try {
        p_root     = (named_root && strlen(named_root))? cpswGetNamedRoot(named_root): cpswGetRoot();
        p_rfCommon = p_root->findByName(pathString);
    } catch (CPSWError &e) {
        fprintf(stderr, "CPSW Error: %s, file: %s, line: %d\n", e.getInfo().c_str(), __FILE__, __LINE__);
    }


    rfCommon = IRFCommonFw::create(p_rfCommon);
    rfCommon->createStream(p_root);
    ParameterSetup();

    uint32_t val;
    rfCommon->getDemodVersion(&val); setIntegerParam(p_demod_version,  val);
    rfCommon->getPllVersion(&val);   setIntegerParam(p_pll_version,    val);
    rfCommon->getUpCnvVersion(&val); setIntegerParam(p_upConv_version, val);
}

RFCommonAsynDriver::~RFCommonAsynDriver() {}

void RFCommonAsynDriver::report(int level)
{
    printf("Driver Class: %s (port: %s, path: %s)\n", driverName, port, path);
}

void RFCommonAsynDriver::poll(void)
{
    for(int i = 0; i < MAX_CHN; i++) {
        double phase, amplitude;
        uint32_t raw_phase, raw_amplitude;
        uint32_t rot_i, rot_q;

        phase     = rfCommon->getPhase(&raw_phase, i);
        amplitude = rfCommon->getAmp(&raw_amplitude, i);
        setDoubleParam(p_phase[i], phase);          setIntegerParam(p_phaseRaw[i], raw_phase);
        setDoubleParam(p_amplitude[i], amplitude);  setIntegerParam(p_amplitudeRaw[i], raw_amplitude);

        rfCommon->getRotationIQ(&rot_i, &rot_q, i);
        setIntegerParam(p_getRotI[i], rot_i);
        setIntegerParam(p_getRotQ[i], rot_q);

    }

    double   val;
    uint32_t raw;

    val = rfCommon->getLOPhase(&raw); setDoubleParam(p_loPhase,     val); setIntegerParam(p_loPhaseRaw,     raw);
    val = rfCommon->getLOAmp(&raw);   setDoubleParam(p_loAmplitude, val); setIntegerParam(p_loAmplitudeRaw, raw);
    rfCommon->getLOLockCount(&raw);   setIntegerParam(p_loLockCount,  raw);
    rfCommon->getLOLockStatus(&raw);  setIntegerParam(p_loLockStatus, raw);
    rfCommon->getLOLossStatus(&raw);  setIntegerParam(p_loLossStatus, raw);
    val = rfCommon->getClockPhase(&raw); setDoubleParam(p_clockPhase,     val); setIntegerParam(p_clockPhaseRaw,     raw);
    val = rfCommon->getClockAmp(&raw);   setDoubleParam(p_clockAmplitude, val); setIntegerParam(p_clockAmplitudeRaw, raw);
    rfCommon->getClockLockCount(&raw);   setIntegerParam(p_clockLockCount,  raw);
    rfCommon->getClockLockStatus(&raw);  setIntegerParam(p_clockLockStatus, raw);
    rfCommon->getClockLossStatus(&raw);  setIntegerParam(p_clockLossStatus, raw);

    for(int i = 0; i < MAX_REF; i++) {
        double phase, amplitude;
        uint32_t raw_phase, raw_amplitude;

        phase = rfCommon->getRefPhase(&raw_phase, i);
        amplitude = rfCommon->getRefAmp(&raw_amplitude, i);
        setDoubleParam(p_refPhase[i], phase);         setIntegerParam(p_refPhaseRaw[i], raw_phase);
        setDoubleParam(p_refAmplitude[i], amplitude); setIntegerParam(p_refAmplitudeRaw[i], raw_amplitude);
    }






    callParamCallbacks();
}


void RFCommonAsynDriver::ParameterSetup(void)
{
    char param_name[64];

    sprintf(param_name, STR_DEMOD_VER); createParam(param_name, asynParamInt32, &p_demod_version);

    for(int i = 0; i<MAX_CHN; i++) {
        sprintf(param_name, STR_PHASE, i);             createParam(param_name, asynParamFloat64, &(p_phase[i]));
        sprintf(param_name, STR_AMPLITUDE, i);         createParam(param_name, asynParamFloat64, &(p_amplitude[i]));
        sprintf(param_name, STR_PHASE_RAW, i);         createParam(param_name, asynParamInt32,   &(p_phaseRaw[i]));
        sprintf(param_name, STR_AMPLITUDE_RAW, i);     createParam(param_name, asynParamInt32,   &(p_amplitudeRaw[i]));
        sprintf(param_name, STR_SET_ROT_PHASE, i);     createParam(param_name, asynParamFloat64, &(p_setRotPhase[i]));
        sprintf(param_name, STR_SET_ROT_AMPLITUDE, i); createParam(param_name, asynParamFloat64, &(p_setRotAmplitude[i]));
        sprintf(param_name, STR_GET_ROT_I, i);         createParam(param_name, asynParamInt32, &(p_getRotI[i]));
        sprintf(param_name, STR_GET_ROT_Q, i);         createParam(param_name, asynParamInt32, &(p_getRotQ[i]));
    }

    sprintf(param_name, STR_PLL_VER);          createParam(param_name, asynParamInt32, &p_pll_version);
    sprintf(param_name, STR_LO_PHASE);         createParam(param_name, asynParamFloat64, &p_loPhase);
    sprintf(param_name, STR_LO_AMPLITUDE);     createParam(param_name, asynParamFloat64, &p_loAmplitude);
    sprintf(param_name, STR_LO_PHASE_RAW);     createParam(param_name, asynParamInt32, &p_loPhaseRaw);
    sprintf(param_name, STR_LO_AMPLITUDE_RAW); createParam(param_name, asynParamInt32, &p_loAmplitudeRaw);
    sprintf(param_name, STR_LO_LOCK_COUNT);    createParam(param_name, asynParamInt32, &p_loLockCount);
    sprintf(param_name, STR_LO_LOCK_STATUS);   createParam(param_name, asynParamInt32, &p_loLockStatus);
    sprintf(param_name, STR_LO_LOSS_STATUS);   createParam(param_name, asynParamInt32, &p_loLossStatus);
    sprintf(param_name, STR_CLOCK_PHASE);      createParam(param_name, asynParamFloat64, &p_clockPhase);
    sprintf(param_name, STR_CLOCK_AMPLITUDE);  createParam(param_name, asynParamFloat64, &p_clockAmplitude);
    sprintf(param_name, STR_CLOCK_PHASE_RAW);  createParam(param_name, asynParamInt32, &p_clockPhaseRaw);
    sprintf(param_name, STR_CLOCK_AMPLITUDE_RAW); createParam(param_name, asynParamInt32, &p_clockAmplitudeRaw);
    sprintf(param_name, STR_CLOCK_LOCK_COUNT);    createParam(param_name, asynParamInt32, &p_clockLockCount);
    sprintf(param_name, STR_CLOCK_LOCK_STATUS);   createParam(param_name, asynParamInt32, &p_clockLockStatus);
    sprintf(param_name, STR_CLOCK_LOSS_STATUS);   createParam(param_name, asynParamInt32, &p_clockLossStatus);
    sprintf(param_name, STR_LOCK_THRESHOLD);      createParam(param_name, asynParamInt32, &p_lockThreshold);
    sprintf(param_name, STR_LOSS_THRESHOLD);      createParam(param_name, asynParamInt32, &p_lossThreshold);
    sprintf(param_name, STR_LO_LOCK_COUNT_RESET); createParam(param_name, asynParamInt32, &p_loLockCountReset);
    sprintf(param_name, STR_CLOCK_LOCK_COUNT_RESET); createParam(param_name, asynParamInt32, &p_clockLockCountReset);
    sprintf(param_name, STR_LO_DAC_CONTROL_MUX);     createParam(param_name, asynParamInt32, &p_loDacControlMux);
    sprintf(param_name, STR_CLOCK_DAC_CONTROL_MUX);  createParam(param_name, asynParamInt32, &p_clockDacControlMux);
    sprintf(param_name, STR_USER_DAC_CONTROL_MUX);   createParam(param_name, asynParamInt32, &p_userDacControlMux);
    sprintf(param_name, STR_LO_REF_SELECT);          createParam(param_name, asynParamInt32, &p_loRefSelect);
    sprintf(param_name, STR_CLOCK_REF_SELECT);       createParam(param_name, asynParamInt32, &p_clockRefSelect);
    sprintf(param_name, STR_LO_POLARITY);            createParam(param_name, asynParamInt32, &p_loPolarity);
    sprintf(param_name, STR_CLOCK_POLARITY);         createParam(param_name, asynParamInt32, &p_clockPolarity);
    sprintf(param_name, STR_LO_DAC);                 createParam(param_name, asynParamInt32, &p_loDac);
    sprintf(param_name, STR_CLOCK_DAC);              createParam(param_name, asynParamInt32, &p_clockDac);
    sprintf(param_name, STR_LO_SET_PHASE);           createParam(param_name, asynParamInt32, &p_loSetPhase);
    sprintf(param_name, STR_CLOCK_SET_PHASE);        createParam(param_name, asynParamInt32, &p_clockSetPhase);
    sprintf(param_name, STR_LO_KP);                  createParam(param_name, asynParamInt32, &p_loKp);
    sprintf(param_name, STR_LO_KI);                  createParam(param_name, asynParamInt32, &p_loKi);
    sprintf(param_name, STR_CLOCK_KP);               createParam(param_name, asynParamInt32, &p_clockKp);
    sprintf(param_name, STR_CLOCK_KI);               createParam(param_name, asynParamInt32, &p_clockKi);

    sprintf(param_name, STR_UPCONV_VERSION);         createParam(param_name, asynParamInt32, &p_upConv_version);

    for(int i = 0; i < MAX_REF; i++) {
        sprintf(param_name, STR_REF_PHASE,         i);  createParam(param_name, asynParamFloat64, &(p_refPhase[i]));
        sprintf(param_name, STR_REF_AMPLITUDE,     i);  createParam(param_name, asynParamFloat64, &(p_refAmplitude[i]));
        sprintf(param_name, STR_REF_PHASE_RAW,     i);  createParam(param_name, asynParamInt32,   &(p_refPhaseRaw[i]));
        sprintf(param_name, STR_REF_AMPLITUDE_RAW, i);  createParam(param_name, asynParamInt32,   &(p_refAmplitudeRaw[i]));
    }
    sprintf(param_name, STR_RF_ENABLE);              createParam(param_name, asynParamInt32, &p_rfEnable);
    sprintf(param_name, STR_MODE_SELECT);            createParam(param_name, asynParamInt32, &p_modeSelect);
    sprintf(param_name, STR_LO_SELECT);              createParam(param_name, asynParamInt32, &p_loSelect);

    for(int i = 0; i < MAX_REF; i++) {
        sprintf(param_name, STR_REF_SELECT, i);      createParam(param_name, asynParamInt32, &(p_refSelect[i]));
    }


    sprintf(param_name, STR_INVERT_CORRECTION);      createParam(param_name, asynParamInt32, &p_invertCorrection);
    sprintf(param_name, STR_BASEBAND_I);             createParam(param_name, asynParamInt32, &p_basebandI);
    sprintf(param_name, STR_BASEBAND_Q);             createParam(param_name, asynParamInt32, &p_basebandQ);
    sprintf(param_name, STR_SET_BASEBAND_PHASE);     createParam(param_name, asynParamFloat64, &p_setBasebandPhase);
    sprintf(param_name, STR_SET_BASEBAND_AMPLITUDE); createParam(param_name, asynParamFloat64, &p_setBasebandAmplitude);
    sprintf(param_name, STR_DAC_OFFSET);             createParam(param_name, asynParamInt32, &p_dacOffset);
    sprintf(param_name, STR_DAC_MIN);                createParam(param_name, asynParamInt32, &p_dacMin);
    sprintf(param_name, STR_DAC_MAX);                createParam(param_name, asynParamInt32, &p_dacMax);
}


asynStatus RFCommonAsynDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = (asynStatus) setIntegerParam(function, value);

    if(function == p_lockThreshold)              rfCommon->setLockThreshold((uint32_t) value);
    else if(function == p_lossThreshold)         rfCommon->setLossThreshold((uint32_t) value);
    else if(function == p_loLockCountReset)      rfCommon->loLockCountReset((uint32_t) value);
    else if(function == p_clockLockCountReset)   rfCommon->clockLockCountReset((uint32_t) value);
    else if(function == p_loDacControlMux)       rfCommon->loDacControlMux((uint32_t) value);
    else if(function == p_clockDacControlMux)    rfCommon->clockDacControlMux((uint32_t) value);
    else if(function == p_userDacControlMux)     rfCommon->userDacControlMux((uint32_t) value);
    else if(function == p_loRefSelect)           rfCommon->loReferenceSelect((uint32_t) value);
    else if(function == p_clockRefSelect)        rfCommon->clockReferenceSelect((uint32_t) value);
    else if(function == p_loPolarity)            rfCommon->loPolarity((uint32_t) value);
    else if(function == p_clockPolarity)         rfCommon->clockPolarity((uint32_t) value);
    else if(function == p_loDac)                 rfCommon->setLODac((uint32_t) value);
    else if(function == p_clockDac)              rfCommon->setClockDac((uint32_t) value);
    else if(function == p_loPhase)               rfCommon->setLOPhase((uint32_t) value);
    else if(function == p_clockPhase)            rfCommon->setClockPhase((uint32_t) value);
    else if(function == p_loKp)                  rfCommon->setLOKp((uint32_t) value);
    else if(function == p_loKi)                  rfCommon->setLOKi((uint32_t) value);
    else if(function == p_clockKp)               rfCommon->setClockKp((uint32_t) value);
    else if(function == p_clockKi)               rfCommon->setClockKi((uint32_t) value);
    else if(function == p_rfEnable)              rfCommon->rfEnable((uint32_t) value);
    else if(function == p_modeSelect)            rfCommon->modeSelect((uint32_t) value);
    else if(function == p_loSelect)              rfCommon->loSelect((uint32_t) value);
    else if(function == p_invertCorrection)      rfCommon->invertCorrection((uint32_t) value);
    else if(function == p_dacOffset)             rfCommon->setDacOffset((uint32_t) value);
    else if(function == p_dacMin)                rfCommon->setDacMin((uint32_t) value);
    else if(function == p_dacMax)                rfCommon->setDacMax((uint32_t) value);
    else 

    for(int i = 0; i < MAX_REF; i++) {
        if(function == p_refSelect[i]) {
            rfCommon->refChnSelect((uint32_t) value, i);
            break;
        }
    }

    return status;
}


asynStatus RFCommonAsynDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeFloat64";

    status = (asynStatus) setDoubleParam(function, value);

    for(int i = 0; i < MAX_CHN; i++) {
        if(function == p_setRotPhase[i] || function == p_setRotAmplitude[i]) {
            double phase, amplitude;
            getDoubleParam(p_setRotPhase[i], &phase);
            getDoubleParam(p_setRotAmplitude[i], &amplitude);
            rfCommon->setRotationPA(phase, amplitude, i);
            break;
        }

    }

    if(function == p_setBasebandPhase || function == p_setBasebandAmplitude) {
        double phase, amplitude;
        uint32_t i, q;

        getDoubleParam(p_setBasebandPhase, &phase);
        getDoubleParam(p_setBasebandAmplitude, &amplitude);
        rfCommon->setBasebandPA(phase, amplitude);

        rfCommon->getBasebandIQ(&i, &q);
        setIntegerParam(p_basebandI, i);
        setIntegerParam(p_basebandQ, q);
    }


    return status;
}



drvNode_t* last_drvList_RFCommon(void)
{
    drvNode_t *p = NULL;

    if(!(pDrvList && ellCount(pDrvList))) return p;

    p = (drvNode_t *) ellLast(pDrvList);

    return p;
}




static void init_drvList(void)
{
    if(!pDrvList) {
        pDrvList = (ELLLIST *) mallocMustSucceed(sizeof(ELLLIST), "rfCommonAsynDriver linked list");
        ellInit(pDrvList);
    }

}

static void add_drvList(drvNode_t *p)
{
    if(!pDrvList) init_drvList();

    ellAdd(pDrvList, (ELLNODE *) p);
}




static long rfCommonAsynDriverReport(int interest)
{
    drvNode_t *p = NULL;

    if(!pDrvList) init_drvList();
    if(!ellCount(pDrvList)) return 0;

    p = (drvNode_t *) ellFirst(pDrvList);
    while(p && p->pDrv) {
        printf("named_root:  %s\n", p->named_root);
        p->pDrv->report(interest);
        if(p->pUpConv   && p->pUpConv->pDrv)    p->pUpConv->pDrv->report(interest);
        if(p->pDownConv && p->pDownConv->pDrv)  p->pDownConv->pDrv->report(interest);
        p = (drvNode_t *) ellNext(&p->node);
    }    
    

    return 0;
}


static long rfCommonAsynDriverPoll(void)
{
    drvNode_t *p = NULL;

    if(!pDrvList) init_drvList();
    if(!ellCount(pDrvList)) return 0;

    p = (drvNode_t *) ellFirst (pDrvList);
    while(p && p->pDrv) {
        p->pDrv->poll();
        if(p->pUpConv   && p->pUpConv->pDrv)    p->pUpConv->pDrv->poll();
        if(p->pDownConv && p->pDownConv->pDrv)  p->pDownConv->pDrv->poll();
        p = (drvNode_t *) ellNext(&p->node);
    }
}

static long rfCommonAsynDriverPollThread(void *p)
{

    while(1) {
        rfCommonAsynDriverPoll();
        epicsThreadSleep(.5);
    }
 
    return 0;   
}


static long rfCommonAsynDriverInitialize(void)
{
    if(!pDrvList || !ellCount(pDrvList)) return 0;

    char name[64];

    sprintf(name, "mon_%s", driverName);
    epicsThreadCreate(name, epicsThreadPriorityHigh - 10,
                      epicsThreadGetStackSize(epicsThreadStackMedium),
                      (EPICSTHREADFUNC) rfCommonAsynDriverPollThread, (void *) NULL);

    return 0;
}



static long rfCommonAsynDriverReport(int interest);
static long rfCommonAsynDriverInitialize(void);

drvet rfCommonAsynDriver = {
    2,
    (DRVSUPFUN) rfCommonAsynDriverReport,
    (DRVSUPFUN) rfCommonAsynDriverInitialize
};

epicsExportAddress(drvet, rfCommonAsynDriver);

extern "C" {
/* consideration for Cexp */
int cpswRFCommonAsynDriverConfigure(const char *portName, const char *pathName, const char *named_root)
{
    drvNode_t *p= (drvNode_t *) mallocMustSucceed(sizeof(drvNode_t), "RFCommon Driver");

    p->named_root = epicsStrDup( (named_root && strlen(named_root))? named_root: cpswGetRootName() );
    p->portName = epicsStrDup(portName);
    p->pathName = epicsStrDup(pathName);

    p->pDrv     = new RFCommonAsynDriver((const char *) p->portName, (const char *) p->pathName, (const char *)p->named_root);
    p->pUpConv  = NULL;
    p->pDownConv = NULL;

    add_drvList(p);

    return 0;
}


} /* end of extern C */


/* EPICS ioc shell command */

static const iocshArg   initArg0 = {"portName", iocshArgString};
static const iocshArg   initArg1 = {"pathName", iocshArgString};
static const iocshArg   initArg2 = {"named_root (optional)", iocshArgString};
static const iocshArg   * const initArgs[] = { &initArg0, &initArg1, &initArg2 };
static const iocshFuncDef initFuncDef = {"cpswRFCommonAsynDriverConfigure", 3, initArgs};
static void  initCallFunc(const iocshArgBuf *args)
{
    cpswRFCommonAsynDriverConfigure(args[0].sval, args[1].sval,
                                    (args[2].sval && strlen(args[2].sval))? args[2].sval: NULL);
}

void cpswRFCommonAsynDriverRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(cpswRFCommonAsynDriverRegister);

