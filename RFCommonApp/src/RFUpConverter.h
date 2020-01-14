#ifndef RF_UPCONV_ASYN_DRIVER_H
#define RF_UPCONV_ASYN_DRIVER_H

#include <asynPortDriver.h>
#include <epicsEvent.h>
#include <epicsTypes.h>
#include <epicsTime.h>

#include <cpsw_api_user.h>
#include <llrfUpConverter.h>
#include <vector>
#include <string>
#include <dlfcn.h>

#include <stdio.h>
#include <sstream>
#include <fstream>

#define UP_MAX_TEMP_CHN      4
#define UP_MAX_ATTEN_CHN     4


class RFUpConvAsynDriver:asynPortDriver {
    public:
        RFUpConvAsynDriver(const char *portName, const char *pathString, const char *named_root = NULL);
        ~RFUpConvAsynDriver();
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        void report(int level);
        void poll(void);

    private:
        char *port;
        char *path;
        LlrfUpConverterFw   llrfUpConv;
        void ParameterSetup(void);

    protected:
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
        int firstRFUpConvParam;
#define FIRST_RFUPCONV_PARAM    firstRFUpConvParam
#endif /* asyn version check, under 4.32 */
        int p_temp[UP_MAX_TEMP_CHN];
        int p_atten[UP_MAX_ATTEN_CHN];
#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
        int lastRFUpConvParam;
#define LAST_RFUPCONV_PARAM    lastRFUpConvParam
#endif /* asyn version check, under 4.32 */
};

#if (ASYN_VERSION <<8 | ASYN_REVISION) < (4<<8 | 32)
#define NUM_RFUPCONV_DET_PARAMS ((int) (&LAST_RFUPCONV_PARAM - &FIRST_RFUPCONV_PARAM-1))
#endif /* asyn version check, under 4.32 */

typedef struct {
    char     *portName;
    char     *pathName;
    RFUpConvAsynDriver *pDrv;
}  upConvNode_t;


#define STR_TEMP            "temp_%d"
#define STR_ATTEN           "atten_%d"

#endif /* RF_UPCONV_ASYN_DRIVER_H */
