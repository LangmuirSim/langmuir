all: help

help:
	@echo "commands:     "
	@echo "  make clean  "
	@echo "  make doxygen"
	@echo "  make html   "
	@echo "  make latex  "
	@echo "  make show   "

clean:
	@rm -rf ./html
	@rm -rf ./latex

doxygen: clean
	doxygen Doxyfile

html: doxygen

latex: doxygen
	@$(MAKE) -s -C ./latex

show: doxygen
	gnome-www-browser ./html/index.html
