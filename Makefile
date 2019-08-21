#
# OMNeT++/OMNEST Makefile for Amod_Simulator
#
# This file was generated with the command:
#  opp_makemake -f --deep -O out
#

# Name of target to be created (-o option)
TARGET = Amod_Simulator$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -Isimulations \
    -Isimulations/results \
    -Isimulations/results/simHours-4 \
    -Isimulations/results/simHours-4/coordType-Heuristic \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/seats-1 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/seats-2 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/seats-1 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10 \
    -Isimulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/seats-2 \
    -Isrc \
    -Isrc/common \
    -Isrc/coordinator \
    -Isrc/networkmanager \
    -Isrc/node

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/src/common/StopPoint.o \
    $O/src/common/TripRequest.o \
    $O/src/common/Vehicle.o \
    $O/src/common/VehicleState.o \
    $O/src/coordinator/BaseCoord.o \
    $O/src/coordinator/HeuristicCoord.o \
    $O/src/coordinator/RadioTaxiCoord.o \
    $O/src/coordinator/StopPointOrderingProposal.o \
    $O/src/networkmanager/AdaptiveNetworkManager.o \
    $O/src/networkmanager/ManhattanNetworkManager.o \
    $O/src/node/App.o \
    $O/src/node/L2Queue.o \
    $O/src/node/ManhattanRouting.o \
    $O/src/node/Routing.o \
    $O/src/node/TripRequestSubmitter.o \
    $O/src/common/Packet_m.o

# Message files
MSGFILES = \
    src/common/Packet.msg

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f Amod_Simulator Amod_Simulator.exe libAmod_Simulator.so libAmod_Simulator.a libAmod_Simulator.dll libAmod_Simulator.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h
	$(Q)-rm -f simulations/*_m.cc simulations/*_m.h
	$(Q)-rm -f simulations/results/*_m.cc simulations/results/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/*_m.cc simulations/results/simHours-4/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/*_m.cc simulations/results/simHours-4/coordType-Heuristic/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/seats-1/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/seats-1/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/seats-2/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/seats-2/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/seats-1/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/seats-1/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/*_m.h
	$(Q)-rm -f simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/seats-2/*_m.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/seats-2/*_m.h
	$(Q)-rm -f src/*_m.cc src/*_m.h
	$(Q)-rm -f src/common/*_m.cc src/common/*_m.h
	$(Q)-rm -f src/coordinator/*_m.cc src/coordinator/*_m.h
	$(Q)-rm -f src/networkmanager/*_m.cc src/networkmanager/*_m.h
	$(Q)-rm -f src/node/*_m.cc src/node/*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc simulations/*.cc simulations/results/*.cc simulations/results/simHours-4/*.cc simulations/results/simHours-4/coordType-Heuristic/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-10/seats-1/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-1500/seats-2/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-228/vehicles-500/seats-1/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/*.cc simulations/results/simHours-4/coordType-Heuristic/maxDelay-5/sendIaTime-57/vehicles-10/seats-2/*.cc src/*.cc src/common/*.cc src/coordinator/*.cc src/networkmanager/*.cc src/node/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/src/common/Packet_m.o: src/common/Packet_m.cc \
	src/common/Packet_m.h
$O/src/common/StopPoint.o: src/common/StopPoint.cc \
	src/common/StopPoint.h
$O/src/common/TripRequest.o: src/common/TripRequest.cc \
	src/common/StopPoint.h \
	src/common/TripRequest.h
$O/src/common/Vehicle.o: src/common/Vehicle.cc \
	src/common/Packet_m.h \
	src/common/Vehicle.h
$O/src/common/VehicleState.o: src/common/VehicleState.cc \
	src/common/VehicleState.h
$O/src/coordinator/BaseCoord.o: src/coordinator/BaseCoord.cc \
	src/common/Packet_m.h \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/common/Vehicle.h \
	src/common/VehicleState.h \
	src/coordinator/BaseCoord.h \
	src/coordinator/StopPointOrderingProposal.h \
	src/networkmanager/AbstractNetworkManager.h
$O/src/coordinator/HeuristicCoord.o: src/coordinator/HeuristicCoord.cc \
	src/common/Packet_m.h \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/common/Vehicle.h \
	src/common/VehicleState.h \
	src/coordinator/BaseCoord.h \
	src/coordinator/HeuristicCoord.h \
	src/coordinator/StopPointOrderingProposal.h \
	src/networkmanager/AbstractNetworkManager.h
$O/src/coordinator/RadioTaxiCoord.o: src/coordinator/RadioTaxiCoord.cc \
	src/common/Packet_m.h \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/common/Vehicle.h \
	src/common/VehicleState.h \
	src/coordinator/BaseCoord.h \
	src/coordinator/RadioTaxiCoord.h \
	src/coordinator/StopPointOrderingProposal.h \
	src/networkmanager/AbstractNetworkManager.h
$O/src/coordinator/StopPointOrderingProposal.o: src/coordinator/StopPointOrderingProposal.cc \
	src/common/StopPoint.h \
	src/coordinator/StopPointOrderingProposal.h
$O/src/networkmanager/AdaptiveNetworkManager.o: src/networkmanager/AdaptiveNetworkManager.cc \
	src/networkmanager/AbstractNetworkManager.h \
	src/networkmanager/AdaptiveNetworkManager.h
$O/src/networkmanager/ManhattanNetworkManager.o: src/networkmanager/ManhattanNetworkManager.cc \
	src/networkmanager/AbstractNetworkManager.h \
	src/networkmanager/ManhattanNetworkManager.h
$O/src/node/App.o: src/node/App.cc \
	src/common/Packet_m.h \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/common/Vehicle.h \
	src/common/VehicleState.h \
	src/coordinator/BaseCoord.h \
	src/coordinator/StopPointOrderingProposal.h \
	src/networkmanager/AbstractNetworkManager.h
$O/src/node/L2Queue.o: src/node/L2Queue.cc
$O/src/node/ManhattanRouting.o: src/node/ManhattanRouting.cc \
	src/common/Packet_m.h \
	src/common/Vehicle.h \
	src/node/ManhattanRouting.h
$O/src/node/Routing.o: src/node/Routing.cc \
	src/common/Packet_m.h \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/common/Vehicle.h \
	src/common/VehicleState.h \
	src/coordinator/BaseCoord.h \
	src/coordinator/StopPointOrderingProposal.h \
	src/networkmanager/AbstractNetworkManager.h \
	src/node/Routing.h
$O/src/node/TripRequestSubmitter.o: src/node/TripRequestSubmitter.cc \
	src/common/StopPoint.h \
	src/common/TripRequest.h \
	src/networkmanager/AbstractNetworkManager.h

