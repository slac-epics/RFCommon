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
#include "RFUpConverter.h"

static const char *driverName = "RFUpConvAsynDriver";

RFUpConvAsynDriver::RFUpConvAsynDriver(const char *portName, const char *pathString, const char *named_root)
    : asynPortDriver(portName,
                     1, /* number of elements of this device */
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
                     NUM_RFUPCONV_DET_PARAMS, /* number of asyn params of be cleared for each device */
#endif /* asyn version check, under 4.32 */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask, /* Interface mask */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynEnumMask    | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask,  /* Interrupt mask */
                     1, /* asynFlags.  This driver does block and it is not multi-device, so flag is 1 */
                     1, /* Autoconnect */
                     0, /* Default priority */
                     0) /* Default stack size*/
{
    Path p_root;
    Path p_rfUpConv;

    port = epicsStrDup(portName);
    path = epicsStrDup(pathString);

    try {
        p_root     = (named_root)?cpswGetNamedRoot(named_root):cpswGetRoot();
        p_rfUpConv = p_root->findByName(pathString);
    } catch (CPSWError &e) {
        fprintf(stderr, "CPSW Error: %s, file: %s, line %d\n", e.getInfo().c_str(), __FILE__, __LINE__);
    }

    llrfUpConv = ILlrfUpConverterFw::create(p_rfUpConv);


    ParameterSetup();
    llrfUpConv->acqTemp();
}

RFUpConvAsynDriver::~RFUpConvAsynDriver() {}

void RFUpConvAsynDriver::report(int level)
{
    printf("  Driver Class: %s (part: %s, path: %s)\n", driverName, port, path);
}

void RFUpConvAsynDriver::poll(void)
{
    for(int i = 0; i < UP_MAX_TEMP_CHN; i++) {
        setDoubleParam(p_temp[i], llrfUpConv->getTemp(i));
    }

    llrfUpConv->acqTemp();
    callParamCallbacks();
}


void RFUpConvAsynDriver::ParameterSetup(void)
{
    char param_name[64];

    for(int i = 0; i < UP_MAX_TEMP_CHN; i++) {
        sprintf(param_name, STR_TEMP, i);    createParam(param_name, asynParamFloat64, &(p_temp[i]));
    }

    for(int i = 0; i < UP_MAX_ATTEN_CHN; i++) {
        sprintf(param_name, STR_ATTEN, i);   createParam(param_name, asynParamInt32,   &(p_atten[i]));
    }
}

asynStatus RFUpConvAsynDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = (asynStatus) setIntegerParam(function, value);

    for(int i = 0; i < UP_MAX_ATTEN_CHN; i++) {
        if(function == p_atten[i]) {
            llrfUpConv->setAtten((uint32_t) value, i);
            break;
        }
    }

    return status;
}


extern "C" {
int cpswLlrfUpConvAsynDriverConfigure(const char *portName, const char *pathName)
{
    drvNode_t    *pList   = last_drvList_RFCommon();
    upConvNode_t *pUpConv = (upConvNode_t *) mallocMustSucceed(sizeof(upConvNode_t), "LlrfUpConverter Driver");

    pUpConv->portName = epicsStrDup(portName);
    pUpConv->pathName = epicsStrDup(pathName);

    pUpConv->pDrv = new RFUpConvAsynDriver((const char *) pUpConv->portName, (const char *) pUpConv->pathName,
                                           (pList && pList->named_root)? (const char *) pList->named_root: (const char *) NULL);

    if(pList && pUpConv) pList->pUpConv = pUpConv;
   

    return 0;
}
} /* extern C */


/* EPICS ioc shell command */

static const iocshArg     initArg0 = {"portName", iocshArgString};
static const iocshArg     initArg1 = {"pathName", iocshArgString};
static const iocshArg     * const initArgs[] = {&initArg0, &initArg1};
static const iocshFuncDef initFuncDef = {"cpswLlrfUpConvAsynDriverConfigure", 2, initArgs};
static void  initCallFunc(const iocshArgBuf *args)
{
    cpswLlrfUpConvAsynDriverConfigure(args[0].sval, args[1].sval);
}

void cpswLlrfUpConvAsynDriverRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(cpswLlrfUpConvAsynDriverRegister);
