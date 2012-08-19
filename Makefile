build:
	cd jsmn && make
	python setup.py build

clean:
	rm -rf build *.egg-info temp
	cd jsmn && make clean
