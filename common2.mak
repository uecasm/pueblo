# UE Common rules for Pueblo/UE
#
KillCSM:
  if exist $(CSM) del $(CSM)

OutDir :
    if not exist $(OUTDIR)\nul mkdir $(OUTDIR)

ModuleDir :
    if not exist $(OUTDIR)\modules\nul mkdir $(OUTDIR)\modules
    
IntDir :
    if not exist $(INTDIR)\nul mkdir $(INTDIR)
