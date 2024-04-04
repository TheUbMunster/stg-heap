comp-for-coverage:
	g++ *.cpp --coverage -g -O0 -o coverable.out
coverage: comp-for-coverage
	./coverable.out
	gcovr
