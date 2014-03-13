all: help

install:
	python setup.py install

install-user:
	python setup.py install --user

build:
	python setup.py build

clean:
	rm -rf ./build

pyc:
	-find . -name \*.pyc | xargs rm

pdf:
	$(MAKE) -C ./doc latexpdf

html:
	$(MAKE) -C ./doc html

doc: html latex

show: html
	gnome-www-browser ./doc/build/html/index.html

clean-doc:
	$(MAKE) -C ./doc clean

bin:
	python ./misc/make_bin.py

help:
	@echo "Makefile Targets"
	@echo " make build        : compile module"
	@echo " make install      : install module for root"
	@echo " make install-user : install module for user"
	@echo " make clean        : clean build"
	@echo " make pdf          : build docs pdflatex"
	@echo " make html         : build docs html"
	@echo " make show         : build docs html + open web browser"
	@echo " make clean-doc    : clean docs"
	@echo " make bin          : create scripts"
	@echo " make help         : show this message"

.PHONY: html
.PHONY: latex
.PHONY: bin
