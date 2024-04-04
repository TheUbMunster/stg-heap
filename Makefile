src = $(wildcard stg-heap/source/*.cpp)

build: clean
	mkdir -p bin
	g++ $(src) -O2 -o testing.out
	mv testing.out bin
build-test: clean
	mkdir -p bin
	g++ $(src) --coverage -g -O0 -o coverable.out
	mv coverable.out bin
test: build-test
	cd bin
	./coverable.out
	gcovr --cobertura test-coverage.xml
	cd ..

clean:
	rm -rf bin
