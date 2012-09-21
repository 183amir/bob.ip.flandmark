#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Andre Anjos <andre.anjos@idiap.ch>
# Fri 21 Sep 2012 09:19:39 CEST 

"""Tests for flandmark python bindings
"""

import os, sys
import unittest
import bob
import pkg_resources
from nose.plugins.skip import SkipTest

from . import Localizer

def F(name, f):
  """Returns the test file on the "data" subdirectory"""
  return pkg_resources.resource_filename(name, os.path.join('data', f))

INPUT_VIDEO = F('bob.io.test', 'test.mov')

class FlandmarkTest(unittest.TestCase):
  """Performs various tests on the Flandmark package"""

  def test01_video(self):

    op = Localizer("flandmark/haarcascade_frontalface_alt.xml",
        "flandmark/flandmark_model.dat")

    v = bob.io.VideoReader(INPUT_VIDEO)

    for f in v:
      self.assertTrue(op(f))

  def xtest02_broken(self):

    op = Localizer("flandmark/haarcascade_frontalface_alt.xml",
        "flandmark/flandmark_model.dat")

    for i, f in enumerate(bob.io.VideoReader(INPUT_VIDEO)):
      print v
      self.assertTrue(op(v))
