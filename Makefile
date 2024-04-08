.ONESHELL: build-release-linux
.PHONY: clean

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
build-release: build-release-win build-release-linux
	echo "Building the windows and linux releases..."
#build-release-win: clean
#	echo "PERFORMING: ${@}..."
#	mkdir -p bin/win
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



#to select a different shell than sh for just this target, do this:
test: SHELL:=/bin/bash
test:
	@ps #atsign suppresses printing the command itself before it's executed
	-pwd #minus means keep going even if this command fails
	cd ..
	pwd
