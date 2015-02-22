Puppet Compiler in C++
======================

This is a (very early) attempt to write a Puppet 4 compiler in C++11.

Status:

* ~~Puppet 4 compliant lexer~~ [done]
* ~~Puppet 4 compliant parser~~ [done]
* ~~AST construction~~ [done]
* Puppet 4 evaluator [not started]
* Catalog compilation [not started]

Currently, `puppetcpp` will parse manifest files and perform syntax checking.
Semantic analysis is not performed, so the compiler will only error if the
input manifest does not conform to the Puppet 4 grammar.

Only the given manifest will be parsed; manifests that are included/required from the input manifest
will not be parsed as there currently isn't any evaluation of the AST.  This means that
functions called in the manifest are not being executed.

The output of `puppetcpp` is currently the representation of the AST that was parsed.
Eventually it will output a catalog.

Currently not implemented in the parser:

* Unicode escape sequences (\uXXXX)
* A parser for string interpolation (needed for AST evaluation)
* A parser for Embedded Puppet Templates, EPP (needed for AST evaluation)
* A check for ANSI color codes being supported (non-Windows and istty)

Build Requirements
------------------

* GCC >= 4.8 or Clang >= 3.4
* CMake >= 2.8.12
* Boost Libraries >= 1.57

Pre-Build
---------

All of the following examples start by assuming the current directory is the root of the repo.

Before building, use `cmake` to generate build files:

    $ mkdir release
    $ cd release
    $ cmake ..

To generate build files with debug information:

    $ mkdir debug
    $ cd debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..

Build
-----

To build puppetcpp, use 'make':

    $ cd release
    $ make

To build puppetcpp with debug information:

    $ cd debug
    $ make

Run
---

You can run puppetcpp from where it was built:

`$ release/puppetcpp <manifest>`

For a debug build:

`$ debug/puppetcpp <manifest>`

Test
----

Unit tests are not currently implemented.

Install
-------

An install target is not currently implemented.

Uninstall
---------

An install target is not currently implemented.

Documentation
-------------

To generate API documentation, install doxygen 1.8.7 or later.

    $ doxygen

To view the documentation, open `html/index.html` in a web browser.
