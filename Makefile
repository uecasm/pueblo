# Pueblo/UE master Makefile
#
# Include common makefile information
!include "Common.mak"

# Define entry-point links
all : BuildDLLs
	cd $(PROJROOT)\client\msw
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) all
  @cd $(PROJROOT)
	@echo Compilation complete.
	
run : BuildDLLs
	cd $(PROJROOT)\client\msw
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) run
  @cd $(PROJROOT)
	@echo Pueblo/UE is starting up...

!include "Common2.mak"

BuildDLLs : OutDir BuildAPIUtil BuildAPI BuildModules

BuildModules : ModuleDir BuildWorld BuildSound

BuildAPI :
	cd $(PROJROOT)\api
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) all
	
BuildAPIUtil :
	cd $(PROJROOT)\apiutil
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) all

BuildWorld :
	cd $(PROJROOT)\modules\client\msw\chworld
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) all
	
BuildSound :
	cd $(PROJROOT)\modules\client\msw\chsound
	$(MAKEDIR)\$(MAKE) -$(MAKEFLAGS) all
