Puppet Compiler in C++
======================

[![Build Status](https://travis-ci.org/puppetlabs/puppetcpp.svg?branch=master)](https://travis-ci.org/puppetlabs/puppetcpp) [![Code Coverage](https://codecov.io/gh/puppetlabs/puppetcpp/branch/master/graph/badge.svg)](https://codecov.io/gh/puppetlabs/puppetcpp)

[![Build History](https://buildstats.info/travisci/chart/puppetlabs/puppetcpp)](https://travis-ci.org/puppetlabs/puppetcpp)

This is a (very early) attempt to write a Puppet 4.x compiler in C++14.

Parser status:

* [x] Puppet 4.x compliant lexer
* [x] Puppet 4.x compliant parser
* [x] AST construction

Expression evaluator status:

* [x] literal expressions
* [x] variable assignment
* [x] + operator
* [x] - operator (binary)
* [x] - operator (unary)
* [x] * operator (binary)
* [x] * operator (unary)
* [x] / operator
* [x] % operator
* [x] << operator
* [x] >> operator
* [x] logical `and`
* [x] logical `or`
* [x] logical `not`
* [x] == operator
* [x] != operator
* [x] =~ operator
* [x] !~ operator
* [x] < operator
* [x] <= operator
* [x] > operator
* [x] >= operator
* [x] -> operator
* [x] ~> operator
* [x] <- operator
* [x] <~ operator
* [x] `in` operator
* [x] if expressions
* [x] unless expressions
* [x] selector expressions
* [x] case expressions
* [x] method call expressions
* [x] function call expressions
* [x] lambdas
* [x] resource expressions
* [x] resource metaparameters
* [x] virtual resource expressions
* [x] exported resource expressions
* [x] resource defaults expressions (see note below)
* [x] resource override expressions
* [x] class definition statements
* [x] defined type statements
* [x] node definition statements
* [x] resource collection statements
* [ ] exported resource collection expressions (NYI: importing resources)
* [x] break statements
* [ ] return statements
* [ ] next statements
* [x] loading classes from modules
* [x] loading defined types from modules
* [x] access expressions
* [x] global scope
* [x] local scope
* [x] node scope
* [x] string interpolation
* [x] Puppet type aliases
* [x] custom functions written in Puppet
* [ ] custom functions written in Ruby
* [ ] custom types written in Puppet
* [ ] custom types written in Ruby
* [ ] external data binding (i.e. hiera)
* [ ] module data functions in Ruby
* [ ] module data functions in Puppet
* [x] EPP support
* [ ] application orchestration (NYI: evaluation)

Note: resource default expressions use "static scoping" instead of "dynamic scoping"

Type system implemented:

* [x] Any
* [x] Array
* [x] Boolean
* [x] Callable
* [x] CatalogEntry
* [x] Collection
* [x] Data
* [x] Default
* [x] Enum
* [x] Float
* [x] Hash
* [x] Integer
* [x] Iterable
* [x] Iterator
* [x] Class
* [x] NotUndef
* [x] Numeric
* [x] Optional
* [x] Pattern
* [x] Regexp
* [x] Resource
* [x] Runtime
* [x] Scalar
* [x] String
* [x] Struct
* [x] Tuple
* [x] Type
* [x] Undef
* [x] Variant

Puppet functions implemented:

* [x] alert
* [x] assert_type
* [x] contain
* [ ] create_resources
* [x] crit
* [x] debug
* [x] defined
* [ ] digest
* [x] each
* [x] emerg
* [x] epp
* [x] err
* [x] fail
* [x] file
* [x] filter
* [ ] fqdn_rand
* [ ] generate
* [ ] hiera
* [ ] hiera_array
* [ ] hiera_hash
* [ ] hiera_include
* [x] include
* [x] info
* [x] inline_epp
* [ ] inline_template
* [ ] lookup
* [x] map
* [ ] match
* [ ] md5
* [ ] new
* [x] notice
* [x] realize
* [x] reduce
* [ ] regsubst
* [x] require
* [x] reverse_each
* [ ] scanf
* [ ] sha1
* [ ] shellquote
* [ ] slice
* [x] split
* [ ] sprintf
* [x] step
* [x] tag
* [x] tagged
* [ ] template
* [x] type
* [x] versioncmp
* [x] warning
* [x] with

Catalog compiling status:

* [x] Facts from files and Facter
* [x] JSON catalog generation

Running With Docker
-------------------

The `peterhuene/puppetcpp` image on Docker Hub contains a pre-built version of the prototype compiler.

Use the following docker command to run the compiler:

    $ docker run -it peterhuene/puppetcpp

This image is not yet automatically updated when commits are merged into master; it will be periodically updated when new features are merged.


Build Requirements
------------------

* OSX or Linux
* GCC >= 5.0 or Clang >= 3.4 (with libc++)
* [CMake](https://cmake.org/) >= 3.0
* [Boost Libraries](http://www.boost.org/) >= 1.60.0
* [ICU](http://site.icu-project.org/) >= 57.1
* [Onigmo](https://github.com/k-takata/Onigmo) >= 6.0.0
* [Facter](https://github.com/puppetlabs/facter) >= 3.0
* [yaml-cpp](https://github.com/jbeder/yaml-cpp) >= 0.5.1
* [Editline](http://thrysoee.dk/editline/) (optional - improves the REPL experience)

Pre-Build
---------

All of the following examples start by assuming the current directory is the root of the repo.

Before building, use `cmake` to generate build files:

    $ mkdir release
    $ cd release
    $ cmake ..

To generate a fully debug build use `-DCMAKE_BUILD_TYPE=Debug` when invoking `cmake`.

To speed up builds, it is recommended to use [ccache](https://ccache.samba.org/) as the "compiler":

    $ cd release
    $ CCACHE_SLOPPINESS=pch_defines,time_macros cmake .. -DCMAKE_C_COMPILER=$(which ccache) -DCMAKE_C_COMPILER_ARG1=cc -DCMAKE_CXX_COMPILER=$(which ccache) -DCMAKE_CXX_COMPILER_ARG1=c++

**Note: the environment variable `CCACHE_SLOPPINESS` must be set to `pch_defines,time_macros` for precompiled headers to work with ccache.**

Also consider using [Ninja](https://ninja-build.org/) as a replacement for GNU Make:

    $ cd release
    $ cmake .. -GNinja -DDISABLE_PCH=1 -DCMAKE_C_COMPILER=$(which ccache) -DCMAKE_C_COMPILER_ARG1=cc -DCMAKE_CXX_COMPILER=$(which ccache) -DCMAKE_CXX_COMPILER_ARG1=c++

Then use `ninja` instead of `make` in the examples below.

**Note: tracking precompiled header changes with ninja is [currently unsupported in CMake](https://cmake.org/Bug/view.php?id=13234) so it is recommended to add `-DDISABLE_PCH=1` when running cmake.**

Build
-----

Currently Boost 1.61 and later has a breaking change that prevents building this project; see the [PR to revert the change](https://github.com/boostorg/spirit/pull/219).

Boost can be patched by simply updating `include/boost/spirit/home/x3/operator/detail/sequence.hpp` to the previous version.
The previous version is available [here](https://raw.githubusercontent.com/peterhuene/spirit/fcf705ef9ea316dded54e14fb6ba9d01a27ec0e4/include/boost/spirit/home/x3/operator/detail/sequence.hpp).

To build puppetcpp, use 'make':

    $ cd release
    $ make

To build puppetcpp with debug information:

    $ cd debug
    $ make

Run
---

You can run puppetcpp from its output directory:

`$ release/bin/puppetcpp help`

For a debug build:

`$ debug/bin/puppetcpp help`

Test
----

You can run puppetcpp tests using the test target:

    $ cd release
    $ make test

For a debug build:

    $ cd debug
    $ make test

For verbose test output, run `ctest` instead of using the test target:

    $ cd release
    $ ctest -V

Install
-------

You can install puppetcpp into your system:

    $ cd release
    $ make && sudo make install

By default, puppetcpp will install files into `/usr/local/bin`, `/usr/local/lib`, and `/usr/local/include`.

To install to a different location, set the install prefix:

    $ cd release
    $ cmake -DCMAKE_INSTALL_PREFIX=~/puppetcpp ..
    $ make clean install

This would install puppetcpp into `~/puppetcpp/bin`, `~/puppetcpp/lib`, and `~/puppetcpp/include`.

Uninstall
---------

Run the following command to remove files that were previously installed:

    $ sudo xargs rm < release/install_manifest.txt

Documentation
-------------

To generate API documentation, install doxygen 1.8.7 or later.

    $ doxygen

To view the documentation, open `html/index.html` in a web browser.
