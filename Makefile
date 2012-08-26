install:
	make clean
	make jsmnbuild
	easy_install -ZU .

test:
	python tests/test_pyjsmn.py

build: jsmnbuild pyjsmnbuild

jsmnbuild:
	cd jsmn && make

pyjsmnbuild:
	python setup.py build

clean:
	rm -rf build *.egg-info temp
	cd jsmn && make clean
	rm -rf tests/*.pyc
