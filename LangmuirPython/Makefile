all: help

install:
	python setup.py install

install-user:
	python setup.py install --user

build:
	python setup.py build

pdf:
	$(MAKE) -C ./doc latexpdf

html:
	$(MAKE) -C ./doc html

show: html
	gnome-www-browser ./doc/build/html/index.html

clean:
	$(MAKE) -C ./doc clean
	rm -rf ./build

doc: html latex

bin:
	python ./misc/make_bin.py

help:
	@echo "Makefile Targets"
	@echo "\tmake build"
	@echo "\tmake install"
	@echo "\tmake install-user"
	@echo "\tmake clean"
	@echo "\tmake pdf"
	@echo "\tmake html"
	@echo "\tmake show"
	@echo "\tmake help"
	@echo "\tmake bin"

.PHONY: html
.PHONY: latex
.PHONY: bin
