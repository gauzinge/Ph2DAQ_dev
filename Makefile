all: Utils HWDescription HWInterface System tools src

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
doc::
	$(MAKE) -C $@

clean:
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd doc; make clean)
	(rm -f lib/* bin/*)

