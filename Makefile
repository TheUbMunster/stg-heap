src = $(wildcard *.cpp)

build:
	cd stg-heap/source
	g++ $(src) -O2 -o ../../bin/testing.out
	cd ../..
build-test:
	cd stg-heap/source
	g++ $(src) --coverage -g -O0 -o ../../bin/coverable.out
	cd ../..
test: build-test
	cd bin
	./coverable.out
	gcovr --cobertura test-coverage.xml
	cd ..
