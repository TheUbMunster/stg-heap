.ONESHELL: build-release-linux build-tests
.PHONY: clean build-release-linux build-releases test

test: SHELL:=/bin/bash
test: clean
	echo "PERFORMING: ${@}..."
	#make sure the variable representing the root of the project is assigned even if not being run via gitlab CI
	@echo PROJECT DIR $${CI_PROJECT_DIR:=$$(pwd)}
	#make directories
	@mkdir -p bin/test
	@cd bin/test
	#capture source files & compile
	@src=$$(find $${CI_PROJECT_DIR}/stg-heap/source/ -type f -name "*.cpp")
	@g++ -I"$${CI_PROJECT_DIR}/googletest/googletest/include" -L"$${CI_PROJECT_DIR}/googletest/build/lib" -lgtest_main -lgtest \
	-I"$${CI_PROJECT_DIR}/stg-heap/include" \
	-g -O0 --coverage $${src} "$${CI_PROJECT_DIR}/unit-tests/Testing.cpp" -o "$${CI_PROJECT_DIR}/bin/test/coverable.out"
	$${CI_PROJECT_DIR}/bin/test/coverable.out
	@cd ../..
	gcovr -e unit-tests/ -e googletest/ --txt -r stg-heap/source/ --cobertura bin/test-coverage.xml --root $${CI_PROJECT_DIR} bin/test/

#run-tests: SHELL:=/bin/bash
#run-tests: build-tests
#	echo "PERFORMING: ${@}..."
	#make sure the variable representing the root of the project is assigned even if not being run via gitlab CI
#	@echo $${CI_PROJECT_DIR:=$$(pwd)}
#	./bin/linux/build/coverable.out
#	gcovr --txt -r stg-heap/source/ --cobertura bin/test-coverage.xml --root $${CI_PROJECT_DIR} bin/linux/build/
#build-coverage: clean
#	echo "PERFORMING: ${@}..."
#	mkdir -p bin
#	g++ $(src) -I"stg-heap/include" --coverage -g -O0 -o bin/coverable.out
#run-coverage: build-coverage
#	echo "PERFORMING: ${@}..."
#	./bin/coverable.out
#	gcovr --txt -r stg-heap/source/ --cobertura bin/test-coverage.xml --root ${CI_PROJECT_DIR} bin/
build-releases: build-release-linux
	echo "Building the windows and linux releases..."
build-release-linux: SHELL:=/bin/bash
build-release-linux: clean
	@echo "PERFORMING: ${@}..."
	#make sure the variable representing the root of the project is assigned even if not being run via gitlab CI
	@echo $${CI_PROJECT_DIR:=$$(pwd)}
	#make directories & move the include folder
	@mkdir -p bin/linux/build
	@mkdir -p bin/linux/intermediate
	@cp -r stg-heap/include bin/linux/build/include
	@cd bin/linux/intermediate
	#capture source files & compile
	@src=$$(find $${CI_PROJECT_DIR}/stg-heap/source/ -type f -name "*.cpp")
	@g++ -c -fPIC -I"$${CI_PROJECT_DIR}/stg-heap/include" -shared -O2 $${src}
	#capture object files and archive, move to final location
	@objs=$$(find $${CI_PROJECT_DIR}/bin/linux/intermediate -type f -name "*.o")
	@ar rcs libstg-heap.a $${objs}
	@cp libstg-heap.a ../build/
	#usage example: g++ Testing.cpp -I"stg-heap/include" -L. -lstg-heap -o test.out
clean:
	echo "PERFORMING: ${@}..."
	rm -rf bin
