TOP=../..

include $(TOP)/configure/CONFIG

USR_CXXFLAGS += -DHAVE_YAML

#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

INCLUDES += $(addprefix -I,$(YAML_INCLUDE))
INCLUDES += $(addprefix -I,$(BOOST_INCLUDE))

## this part is just for yaml loader driver
INCLUDES += $(addprefix -I,$(CPSW_FRAMEWORK_INCLUDE))

INCLUDES += $(addprefix -I,$(COMMONRF_INCLUDE))

INC += ATCACommon.h
INC += RFCommon.h
INC += RFDownConverter.h
INC += RFUpConverter.h

DBD += ATCACommon.dbd
DBD += RFCommon.dbd


LIBRARY_IOC += ATCACommon
LIBRARY_IOC += RFCommon 

ATCACommon_DBD  += ATCACommon.dbd
ATCACommon_SRCS += ATCACommon.cpp

RFCommon_DBD    += RFCommon.dbd
RFCommon_SRCS   += RFCommon.cpp
RFCommon_SRCS   += RFDownConverter.cpp
RFCommon_SRCS   += RFUpConverter.cpp

cpsw_DIR = $(CPSW_FRAMEWORK_LIB)
commonRF_DIR = $(COMMONRF_LIB)

ATCACommon_LIBS += cpsw
ATCACommon_LIBS += commonRF
ATCACommon_LIBS += asyn
ATCACommon_LIBS += yamlLoader
ATCACommon_LIBS += $(EPICS_BASE_IOC_LIBS)


RFCommon_LIBS += cpsw
RFCommon_LIBS += commonRF
RFCommon_LIBS += asyn
RFCommon_LIBS += yamlLoader
RFCommon_LIBS += $(EPICS_BASE_IOC_LIBS)





#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
#
