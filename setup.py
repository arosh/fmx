from distutils.core import setup, Extension

fmx_module = Extension(
        'fmx',
        sources=['module.cpp'],
        libraries=['boost_python', 'msgpack'],
        extra_compile_args=['-std=c++11', '-march=native'],
        define_macros=[('PYTHON_MODULE', None)])

setup(ext_modules=[fmx_module])
