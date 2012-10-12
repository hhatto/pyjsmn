from distutils.core import setup, Extension


setup(
    name='pyjsmn',
    version='0.2',
    description='Python extension for jsmn(JSON parser).',
    long_description=open('README.rst').read(),
    license='Expat License',
    author='Hideo Hattori',
    author_email='hhatto.jp@gmail.com',
    url='https://github.com/hhatto/pyjsmn',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
    ],
    keywords='json jsmn',
    ext_modules=[
        Extension('pyjsmn',
                  sources=['pyjsmn/_jsmn.c', './jsmn/jsmn.c'],
                  include_dirs=['./jsmn/'],
                  #extra_compile_args=["-DDEBUG"],
                  #extra_compile_args=["-DJSMN_STRICT"],
                  )]
)
