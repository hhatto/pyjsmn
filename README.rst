pyjsmn
======
.. image:: https://secure.travis-ci.org/hhatto/pyjsmn.png?branch=master
   :target: https://secure.travis-ci.org/hhatto/pyjsmn
   :alt: Build status


About
-----
pyjsmn is a wrapper for `JSMN`_ (minimalistic JSON parser).
pyjsmn is not support encoding method(obj->str).

.. _`JSMN`: http://zserge.bitbucket.org/jsmn.html


Installation
------------
from pip::

    $ pip install pyjsmn

from easy_install::

    easy_install -ZU pyjsmn


Requirements
------------
Python2.7.


Usage
-----

basic usage::

    >>> import pyjsmn
    >>> pyjsmn.loads('[1, 2, {"test": "hoge"}]')
    >>> [1, 2, {"test": "hoge"}]


Links
-----
* PyPI_
* GitHub_

.. _PyPI: http://pypi.python.org/pypi/pyjsmn/
.. _GitHub: https://github.com/hhatto/pyjsmn
.. _`Travis-CI`: https://secure.travis-ci.org/hhatto/pyjsmn
