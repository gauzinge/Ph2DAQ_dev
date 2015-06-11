all: Utils HWDescription HWInterface System tools src GUI

libs: Utils HWDescription HWInterface System srcnoroot

gui: Utils HWDescription HWInterface System tools src GUI

simple: Utils HWDescription HWInterface System tools src
	
HWDescription::
	$(MAKE) -C $@
Utils::
	$(MAKE) -C $@
HWInterface::
	$(MAKE) -C $@
System::
	$(MAKE) -C $@
tools::
	$(MAKE) -C $@
src::
	$(MAKE) -C $@
srcnoroot::
	$(MAKE) -C src noroot
GUI::
	$(MAKE) -C GUI/Macros
	$(MAKE) -C $@
	cp $@/Ph2_ACF ./bin
doc::
	$(MAKE) -C $@

clean:
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd GUI; make clean)
	(cd GUI; make clean; cd GUI/Macros; make clean)
	(cd doc; make clean)
	(rm -f lib/* bin/*)

