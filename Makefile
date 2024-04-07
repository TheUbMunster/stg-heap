src = $(wildcard stg-heap/source/*.cpp)

build-tests: clean
	echo "PERFORMING: ${@}..."
	echo "'make ${@}' NOT YET IMPLEMENTED"
run-tests: build-tests
	echo "PERFORMING: ${@}..."
	echo "'make ${@}' NOT YET IMPLEMENTED"
build-coverage: clean
	echo "PERFORMING: ${@}..."
	mkdir -p bin
	g++ $(src) -I"stg-heap/include" --coverage -g -O0 -o bin/coverable.out
run-coverage: build-coverage
	echo "PERFORMING: ${@}..."
	./bin/coverable.out
	gcovr --txt -r stg-heap/source/ --cobertura bin/test-coverage.xml --root ${CI_PROJECT_DIR} bin/
build-release: clean
	echo "PERFORMING: ${@}..."
	mkdir -p bin
	g++ $(src) -I"stg-heap/include" -O2 -o bin/testing.out
clean:
	echo "PERFORMING: ${@}..."
	rm -rf bin