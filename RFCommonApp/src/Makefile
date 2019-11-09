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

INC += RFCommon.h
INC += RFDownConverter.h
INC += RFUpConverter.h

DBD += RFCommon.dbd


LIBRARY_IOC += RFCommon 


RFCommon_DBD    += RFCommon.dbd
RFCommon_SRCS   += RFCommon.cpp
RFCommon_SRCS   += RFDownConverter.cpp
RFCommon_SRCS   += RFUpConverter.cpp

cpsw_DIR = $(CPSW_FRAMEWORK_LIB)
commonRF_DIR = $(COMMONRF_LIB)


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
