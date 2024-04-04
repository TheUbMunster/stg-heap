build:
	g++ stg-heap/*.cpp -O2
build-test:
	g++ stg-heap/*.cpp --coverage -g -O0 -o bin/coverable.out
test: build-test
	cd bin
	./coverable.out
	gcovr --cobertura test-coverage.xml
