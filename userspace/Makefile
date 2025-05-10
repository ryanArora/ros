all: bin/init

bin/init: mkdirp
	$(MAKE) -C src/init
	cp src/init/init bin/

clean:
	rm -rf bin
	$(MAKE) -C src/init clean

mkdirp:
	mkdir -p bin

.PHONY: all clean mkdirp
