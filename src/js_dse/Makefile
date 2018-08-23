DBG_JS=~/code/mozjs-60.0.2pre2/js/src/build_DBG.OBJ/dist/bin/js
OPT_JS=~/code/mozjs-60.0.2pre2/js/src/build_OPT.OBJ/dist/bin/js
D8=~/code/v8/v8/out.gn/x64.optdebug/d8

SMFLAGS= --ion-offthread-compile=off --ion-sink=on --ion-instruction-reordering=on -D
D8FLAGS= --single-threaded --print-opt-code --code-comments --no-debug-code --trace-turbo

IONFLAGS=codegen
IONFILTER=
SCRIPT=demo.js

.PHONY: all
all : sm_opt_dump.txt sm_dbg_dump.txt sm_opt_spew.txt sm_dbg_spew.txt d8_dump.txt

sm_opt_dump.txt : $(OPT_JS) $(SCRIPT)
	$(OPT_JS) $(SMFLAGS) $(SCRIPT) > $@

sm_dbg_dump.txt : $(DBG_JS) $(SCRIPT)
	$(DBG_JS) $(SMFLAGS) $(SCRIPT) > $@

sm_opt_spew.txt : $(OPT_JS) $(SCRIPT)
	IONFILTER=$(IONFILTER) IONFLAGS=$(IONFLAGS) $(OPT_JS) $(SMFLAGS) $(SCRIPT) 2> $@ > /dev/null

sm_dbg_spew.txt : $(DBG_JS) $(SCRIPT)
	IONFILTER=$(IONFILTER) IONFLAGS=$(IONFLAGS) $(DBG_JS) $(SMFLAGS) $(SCRIPT) 2> $@ > /dev/null

d8_dump.txt : $(SCRIPT) $(D8)
	$(D8) $(D8FLAGS) $< > $@

.PHONY: clean
clean :
	-rm *spew.txt *dump.txt
