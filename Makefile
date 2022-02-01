all:
	$(MAKE) -C wiringPi $@
	$(MAKE) -C devLib $@
	-ln -fs libwiringPiDev.so.2.6 devLib/libwiringPiDev.so
	-ln -fs libwiringPi.so.2.6 wiringPi/libwiringPi.so
	$(MAKE) -C gpio 'INCLUDE=-I../devLib -I../wiringPi' 'LDFLAGS=-L../devLib -L../wiringPi' $@

%:
	$(MAKE) -C wiringPi $@
	$(MAKE) -C devLib $@
	$(MAKE) -C gpio $@
