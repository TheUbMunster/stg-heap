build:
	cd stg-heap
	g++ *.cpp -O2
	cd ..
build-test:
	cd stg-heap
	g++ *.cpp --coverage -g -O0 -o bin/coverable.out
	cd ..
test: build-test
	cd bin
	./coverable.out
	gcovr --cobertura test-coverage.xml
