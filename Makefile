src = $(wildcard stg-heap/source/*.cpp)

build: clean
	mkdir -p bin
	g++ $(src) -O2 -o bin/testing.out
build-test: clean
	mkdir -p bin
	g++ $(src) --coverage -g -O0 -o bin/coverable.out
test: build-test
	./bin/coverable.out
	gcovr --txt -r stg-heap/source/ --cobertura bin/test-coverage.xml bin/
clean:
	rm -rf bin
