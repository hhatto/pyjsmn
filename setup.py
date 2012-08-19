from distutils.core import setup, Extension


setup(
    name='pyjsmn',
    version='0.1',
    description='Python extension for jsmn(JSON parser).',
    license='Expat License',
    author='Hideo Hattori',
    author_email='hhatto.jp@gmail.com',
    url='https://github.com/hhatto/pyjsmn',
    ext_modules=[
        Extension('pyjsmn',
                  sources=['pyjsmn/_jsmn.c'],
                  include_dirs=['./jsmn/'],
                  library_dirs=['./jsmn/'],
                  libraries=['jsmn'])]
)
