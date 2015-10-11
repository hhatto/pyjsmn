install:
	make clean
	make jsmnbuild
	easy_install -ZU .

setup: clean freshjsmn jsmnbuild

freshjsmn:
	rm -rf jsmn
	git clone https://github.com/zserge/jsmn.git

test:
	python tests/test_pyjsmn.py

build: jsmnbuild pyjsmnbuild

jsmnbuild:
	cd jsmn && make

pyjsmnbuild:
	python setup.py build

clean:
	rm -rf build *.egg-info dist temp
	rm -rf tests/*.pyc

pypireg:
	python setup.py register
	python setup.py sdist upload
