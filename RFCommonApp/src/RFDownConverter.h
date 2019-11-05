#ifndef RF_DOWNCONV_ASYN_DRIVER_H
#define RF_DOWNCONV_ASYN_DRIVER_H

#include <asynPortDriver.h>
#include <epicsEvent.h>
#include <epicsTypes.h>
#include <epicsTime.h>

#include <cpsw_api_user.h>
#include <llrfDownConverter.h>
#include <vector>
#include <string>
#include <dlfcn.h>

#include <stdio.h>
#include <sstream>
#include <fstream>

#define DN_MAX_TEMP_CHN      4
#define DN_MAX_ATTEN_CHN     6


class RFDownConvAsynDriver:asynPortDriver {
    public:
        RFDownConvAsynDriver(const char *portName, const char *pathString);
        ~RFDownConvAsynDriver();
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        void report(int level);
        void poll(void);

    private:
        char *port;
        char *path;
        LlrfDownConverterFw   llrfDownConv;
        void ParameterSetup(void);

    protected:
        int firstRFDownConvParam;
#define FIRST_RFDOWNCONV_PARAM    firstRFDownConvParam
        int p_temp[DN_MAX_TEMP_CHN];
        int p_atten[DN_MAX_ATTEN_CHN];
        int lastRFDownConvParam;
#define LAST_RFDOWNCONV_PARAM    lastRFDownConvParam

};

#define NUM_RFDOWNCONV_DET_PARAMS ((int) (&LAST_RFDOWNCONV_PARAM - &FIRST_RFDOWNCONV_PARAM-1))

typedef struct {
    char     *portName;
    char     *pathName;
    RFDownConvAsynDriver *pDrv;
}  downConvNode_t;


#define STR_TEMP            "temp_%d"
#define STR_ATTEN           "atten_%d"

#endif /* RF_DOWNCONV_ASYN_DRIVER_H */
