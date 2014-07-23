.. vim: set fileencoding=utf-8 :
.. Andre Anjos <andre.anjos@idiap.ch>
.. Thu 17 Apr 16:59:12 2014 CEST

.. image:: https://travis-ci.org/bioidiap/bob.ip.flandmark.svg?branch=master
   :target: https://travis-ci.org/bioidiap/bob.ip.flandmark
.. image:: http://img.shields.io/badge/docs-latest-orange.png
   :target: https://www.idiap.ch/software/bob/docs/latest/bioidiap/bob.ip.flandmark/master/index.html
.. image:: https://coveralls.io/repos/bioidiap/bob.ip.flandmark/badge.png
   :target: https://coveralls.io/r/bioidiap/bob.ip.flandmark
.. image:: http://img.shields.io/github/tag/bioidiap/bob.ip.flandmark.png
   :target: https://github.com/bioidiap/bob.ip.flandmark
.. image:: http://img.shields.io/pypi/v/bob.ip.flandmark.png
   :target: https://pypi.python.org/pypi/bob.ip.flandmark
.. image:: http://img.shields.io/pypi/dm/bob.ip.flandmark.png
   :target: https://pypi.python.org/pypi/bob.ip.flandmark

==============================
 Python Bindings to Flandmark
==============================

This package is a simple Python wrapper to the (rather quick) open-source
facial landmark detector `Flandmark`_, **version 1.0.7** (or the github state
as of 10/february/2013). If you use this package, the author asks you to cite
the following paper::

  @inproceedings{Uricar-Franc-Hlavac-VISAPP-2012,
    author =      {U{\v{r}}i{\v{c}}{\'{a}}{\v{r}}, Michal and Franc, Vojt{\v{e}}ch and Hlav{\'{a}}{\v{c}}, V{\'{a}}clav},
    title =       {Detector of Facial Landmarks Learned by the Structured Output {SVM}},
    year =        {2012},
    pages =       {547-556},
    booktitle =   {VISAPP '12: Proceedings of the 7th International Conference on Computer Vision Theory and Applications},
    editor =      {Csurka, Gabriela and Braz, Jos{\'{e}}},
    publisher =   {SciTePress --- Science and Technology Publications},
    address =     {Portugal},
    volume =      {1},
    isbn =        {978-989-8565-03-7},
    book_pages =  {747},
    month =       {February},
    day =         {24-26},
    venue =       {Rome, Italy},
    keywords =    {Facial Landmark Detection, Structured Output Classification, Support Vector Machines, Deformable Part Models},
    prestige =    {important},
    authorship =  {50-40-10},
    status =      {published},
    project =     {FP7-ICT-247525 HUMAVIPS, PERG04-GA-2008-239455 SEMISOL, Czech Ministry of Education project 1M0567},
    www = {http://www.visapp.visigrapp.org},
  }

.. note::
  Since version 2.1, bob.ip.flandmark relies on the **new** implementation of Flandmark, which is now part of `CLandmark`_.
  The API has not changed, though, only the models are now in a (readable) XML format.


You should also cite `Bob`_, as a core framework, in which these bindings are
based on::

  @inproceedings{Anjos_ACMMM_2012,
    author = {A. Anjos AND L. El Shafey AND R. Wallace AND M. G\"unther AND C. McCool AND S. Marcel},
    title = {Bob: a free signal processing and machine learning toolbox for researchers},
    year = {2012},
    month = oct,
    booktitle = {20th ACM Conference on Multimedia Systems (ACMMM), Nara, Japan},
    publisher = {ACM Press},
    url = {http://publications.idiap.ch/downloads/papers/2012/Anjos_Bob_ACMMM12.pdf},
  }

Installation
------------

Install it through normal means, via PyPI or use ``zc.buildout`` to bootstrap
the package and run test units.

Documentation
-------------

The latest version of the documentation can be found `here <https://www.idiap.ch/software/bob/docs/latest/bioidiap/bob.ip.flandmark/master/index.html>`_.

Otherwise, you can generate the documentation for this package yourself, after installation, using Sphinx::

  $ sphinx-build -b html doc sphinx

This shall place in the directory ``sphinx``, the current version for the
documentation of the package.

Testing
-------

You can run a set of tests using the nose test runner::

  $ nosetests -sv

.. warning::

   If Bob <= 1.2.1 is installed on your python path, nose will automatically
   load the old version of the insulate plugin available in Bob, which will
   trigger the loading of incompatible shared libraries (from Bob itself), in
   to your working binary. This will cause a stack corruption. Either remove
   the centrally installed version of Bob, or build your own version of Python
   in which Bob <= 1.2.1 is not installed.

You can run our documentation tests using sphinx itself::

  $ sphinx-build -b doctest doc sphinx

You can test overall test coverage with::

  $ nosetests --with-coverage --cover-package=bob.ip.flandmark

The ``coverage`` egg must be installed for this to work properly.

Development
-----------

To develop this package, install using ``zc.buildout``, using the buildout
configuration found on the root of the package::

  $ python bootstrap.py
  ...
  $ ./bin/buildout

Tweak the options in ``buildout.cfg`` to disable/enable verbosity and debug
builds.

.. Place your references here:
.. _flandmark: http://cmp.felk.cvut.cz/~uricamic/flandmark/index.php
.. _bob: https://www.idiap.ch/software/bob
