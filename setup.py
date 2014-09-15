#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Andre Anjos <andre.anjos@idiap.ch>
# Thu 20 Sep 2012 14:43:19 CEST

"""Bindings for flandmark
"""

bob_packages = ['bob.core', 'bob.io.base']

from setuptools import setup, find_packages, dist
dist.Distribution(dict(setup_requires=['bob.blitz'] + bob_packages))
from bob.blitz.extension import Extension, build_ext
import os

this_dir = os.path.dirname(os.path.realpath(__file__))
package_dir = os.path.join(this_dir, 'bob', 'ip', 'flandmark')

version = '2.1.0a0'

packages = ['boost', 'opencv>=2.0']
boost_modules = ['system']
macros = [('DOUBLE_PRECISION', '1')]



def compile_cmake(build_dir):
  """Compiles the external clandmark and flandmark libraries using the original cmake files."""
  import subprocess
  # compile the external code using CMake
  clandmark_dir = os.path.join(this_dir, 'clandmark')
  # configure cmake
  command = ['cmake', clandmark_dir, '-DBUILD_CPP_EXAMPLES=0', '-DBUILD_MATLAB_BINDINGS=0', '-DBUILD_PYTHON_BINDINGS=0', '-DBUILD_SHARED_LIBS=1', '-DDOUBLE_PRECISION=1', '-DCMAKE_BUILD_TYPE=RELEASE']
  subprocess.call(command, cwd=build_dir)
  # run make
  env = {'VERBOSE' : '1'}
  env.update(os.environ)
  subprocess.call(['make'], cwd=build_dir, env=env)
  # return the list of generated libraries
  return  [os.path.join(build_dir, 'libclandmark', l) for l in os.listdir(os.path.join(build_dir, 'libclandmark')) if '.so' in l]


# finally, call setup
setup(

    name="bob.ip.flandmark",
    version=version,
    description="Python bindings to the flandmark keypoint localization library",
    license="GPLv3",
    author='Andre Anjos',
    author_email='andre.anjos@idiap.ch',
    long_description=open('README.rst').read(),
    url='https://github.com/bioidiap/bob.ip.flandmark',

    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,

    install_requires=[
      'setuptools',
      'bob.blitz',
      'bob.core',
      'bob.io.base',
      'bob.io.image', #for tests
      'bob.ip.color', #for tests
      'bob.ip.draw', #for doc generation
      'matplotlib', #for doc generation
    ],

    namespace_packages=[
      "bob",
      "bob.ip",
    ],

    ext_modules=[
      Extension("bob.ip.flandmark.version",
        [
          "bob/ip/flandmark/version.cpp",
        ],
        bob_packages = bob_packages,
        version = version,
        packages = packages,
        boost_modules = boost_modules,
      ),

      Extension("bob.ip.flandmark._library",
        [
          "bob/ip/flandmark/flandmark.cpp",
          "bob/ip/flandmark/main.cpp",
        ],
        bob_packages = bob_packages,

        version = version,
        include_dirs = include_dirs,
        packages = packages,
        boost_modules = boost_modules,
      ),
    ],

    cmdclass = {
      'build_ext': build_ext
    },

    classifiers = [
      'Development Status :: 4 - Development/Beta',
      'Intended Audience :: Developers',
      'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
      'Natural Language :: English',
      'Programming Language :: Python',
      'Topic :: Scientific/Engineering :: Artificial Intelligence',
      ],

    )
