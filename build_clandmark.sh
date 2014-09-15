#!/bin/bash

if [ ! -d __build ]; then
  mkdir __build
fi

cd __build
cmake ../clandmark
make -B
