from setuptools import setup, Extension
import os

here = os.path.dirname(os.path.abspath(__file__))
nuke_include = "C:/Program Files/Nuke17.1v2/include/include"

module = Extension('roto_native',
                    sources = ['roto_native.cpp'],
                    include_dirs = [nuke_include],
                    library_dirs = [here],
                    libraries = ['python311'],
                    extra_compile_args=['/O2'])

setup(name = 'roto_native',
      version = '1.0',
      description = 'C++ Accelerator for Roto Simplifier',
      ext_modules = [module])
