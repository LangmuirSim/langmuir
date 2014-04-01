help:
	@echo "commands:"
	@echo "  make home"
	@echo "  make hutch"
	@echo "  make frank"

home: clean
	mkdir ./build
	cd ./build; cmake ../
	$(MAKE) -C ./build

xsede: clean
	mkdir ./build
	cd ./build; /nics/d/home/ghutchis/src/cmake-2.8.12.2/bin/cmake \
	-DQt5Core_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5Core/ \
	-DQt5Gui_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5Gui/ \
	-DQt5OpenGL_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5OpenGL/ \
	-DQt5Widgets_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5Widgets/ \
	-DQt5Concurrent_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5Concurrent/ \
	-DQt5PrintSupport_DIR=/nics/d/home/ghutchis/src/qt5/qtbase/lib/cmake/Qt5PrintSupport/ \
	-DBOOST_INCLUDEDIR=/sw/kfs/boost/1.44.0/centos6.2_gnu4.4.6/include/ \
	../
	$(MAKE) -C ./build langmuir

hutch: clean
	mkdir ./build
	cd ./build; cmake -DUSE_QT4=ON -DCMAKE_CXX_COMPILER=g++ -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake ../
	$(MAKE) -C ./build

clean:
	rm -rf ./build

