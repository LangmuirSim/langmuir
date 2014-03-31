help:
	@echo "commands:"
	@echo "  make home"
	@echo "  make hutch"
	@echo "  make frank"

home: clean
	mkdir ./build
	cd ./build; cmake ../
	$(MAKE) -C ./build

hutch: clean
	mkdir ./build
	cd ./build; cmake -DUSE_QT4=ON -DCMAKE_CXX_COMPILER=g++ -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake ../
	$(MAKE) -C ./build

clean:
	rm -rf ./build

