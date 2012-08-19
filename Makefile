build:
	cd jsmn && make
	python setup.py build

clean:
	rm -rf build
	cd jsmn && make clean
