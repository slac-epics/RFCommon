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
#include "RFDownConverter.h"

static const char *driverName = "RFDownConvAsynDriver";

RFDownConvAsynDriver::RFDownConvAsynDriver(const char *portName, const char *pathString)
    : asynPortDriver(portName,
                     1, /* number of elements of this device */
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 || 32)
                     NUM_RFDOWNCONV_DET_PARAMS, /* number of asyn params of be cleared for each device */
#endif /* asyn version check, under 4.32 */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask, /* Interface mask */
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynEnumMask    | asynInt16ArrayMask | asynInt32ArrayMask | asynFloat64ArrayMask,  /* Interrupt mask */
                     1, /* asynFlags.  This driver does block and it is not multi-device, so flag is 1 */
                     1, /* Autoconnect */
                     0, /* Default priority */
                     0) /* Default stack size*/
{
    Path p_root;
    Path p_rfDownConv;

    port = epicsStrDup(portName);
    path = epicsStrDup(pathString);

    try {
        p_root     = cpswGetRoot();
        p_rfDownConv = p_root->findByName(pathString);
    } catch (CPSWError &e) {
        fprintf(stderr, "CPSW Error: %s, file: %s, line %d\n", e.getInfo().c_str(), __FILE__, __LINE__);
    }

    llrfDownConv = ILlrfDownConverterFw::create(p_rfDownConv);

    ParameterSetup();
    llrfDownConv->acqTemp();
}

RFDownConvAsynDriver::~RFDownConvAsynDriver() {}

void RFDownConvAsynDriver::report(int level)
{
    printf("  Driver Class: %s (port: %s, path: %s)\n", driverName, port, path);
}

void RFDownConvAsynDriver::poll(void)
{
    for(int i = 0; i < DN_MAX_TEMP_CHN; i++) {
        setDoubleParam(p_temp[i], llrfDownConv->getTemp(i));
    }

    llrfDownConv->acqTemp();
    callParamCallbacks();
}


void RFDownConvAsynDriver::ParameterSetup(void)
{
    char param_name[64];

    for(int i = 0; i < DN_MAX_TEMP_CHN; i++) {
        sprintf(param_name, STR_TEMP, i);    createParam(param_name, asynParamFloat64, &(p_temp[i]));
    }

    for(int i = 0; i < DN_MAX_ATTEN_CHN; i++) {
        sprintf(param_name, STR_ATTEN, i);   createParam(param_name, asynParamInt32,   &(p_atten[i]));
    }
}


asynStatus RFDownConvAsynDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = (asynStatus) setIntegerParam(function, value);

    for(int i = 0; i < DN_MAX_ATTEN_CHN; i++) {
        if(function == p_atten[i]) {
            llrfDownConv->setAtten((uint32_t) value, i);
            break;
        }
    }


    return status;
}

extern "C" {
int cpswLlrfDownConvAsynDriverConfigure(const char *portName, const char *pathName)
{
    drvNode_t    *pList   = last_drvList();
    downConvNode_t *pDownConv = (downConvNode_t *) mallocMustSucceed(sizeof(downConvNode_t), "LlrfDownConverter Driver");

    pDownConv->portName = epicsStrDup(portName);
    pDownConv->pathName = epicsStrDup(pathName);

    pDownConv->pDrv = new RFDownConvAsynDriver((const char *) pDownConv->portName, (const char *) pDownConv->pathName);

    if(pList && pDownConv) pList->pDownConv = pDownConv;
   

    return 0;
}
} /* extern C */


/* EPICS ioc shell command */

static const iocshArg     initArg0 = {"portName", iocshArgString};
static const iocshArg     initArg1 = {"pathName", iocshArgString};
static const iocshArg     * const initArgs[] = {&initArg0, &initArg1};
static const iocshFuncDef initFuncDef = {"cpswLlrfDownConvAsynDriverConfigure", 2, initArgs};
static void  initCallFunc(const iocshArgBuf *args)
{
    cpswLlrfDownConvAsynDriverConfigure(args[0].sval, args[1].sval);
}

void cpswLlrfDownConvAsynDriverRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(cpswLlrfDownConvAsynDriverRegister);


