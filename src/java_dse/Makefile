JAVA=java
JAVAC=javac
JAVAHOME=/Library/Java/JavaVirtualMachines/jdk-10.0.2.jdk/Contents/Home
CC=gcc

DUMP=javadump.txt
CLASSNAME=DSEAttack2

$(DUMP) : $(CLASSNAME).class libAlwaysFalse_native.dylib
	$(JAVA) -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly $(CLASSNAME) > $@

%.class : %.java
	$(JAVAC) $<

%.h : %.java
	$(JAVAC) -h . $<

lib%.dylib : %.c %.h
	$(CC) -I"$(JAVAHOME)/include" -I"$(JAVAHOME)/include/darwin" -dynamiclib $< -o $@

clean :
	-rm $(DUMP) *.class *.dylib
