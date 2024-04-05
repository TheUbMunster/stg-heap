src = $(wildcard stg-heap/source/*.cpp)

build: clean
	mkdir -p bin
	g++ $(src) -O2 -o bin/testing.out
build-test: clean
	mkdir -p bin
	g++ $(src) --coverage -g -O0 -o bin/coverable.out
test: build-test
	./bin/coverable.out
	cd .. ;	gcovr --txt -r stg-heap/stg-heap/source/ --cobertura stg-heap/bin/test-coverage.xml stg-heap/bin/
	mv bin/test-coverage.xml test-coverage.xml
clean:
	rm -rf bin
