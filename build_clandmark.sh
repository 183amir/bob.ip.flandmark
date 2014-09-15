#!/bin/bash

if [ ! -d __build ]; then
  mkdir __build
fi

cd __build
cmake ../clandmark -DBUILD_CPP_EXAMPLES=0 -DBUILD_MATLAB_BINDINGS=0 -DBUILD_PYTHON_BINDINGS=0 -DBUILD_SHARED_LIBS=1 -DDOUBLE_PRECISION=1 -DCMAKE_BUILD_TYPE=RELEASE
make
