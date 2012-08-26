build: jsmnbuild pyjsmnbuild

jsmnbuild:
	cd jsmn && make

pyjsmnbuild:
	python setup.py build

clean:
	rm -rf build *.egg-info temp
	cd jsmn && make clean
