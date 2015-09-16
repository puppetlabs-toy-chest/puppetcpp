Puppet Compiler in C++
======================

This is a (very early) attempt to write a Puppet 4 compiler in C++11.

Parser status:

* [x] Puppet 4 compliant lexer
* [x] Puppet 4 compliant parser
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
* [ ] resource defaults expressions
* [x] resource override expressions
* [x] class definition expressions
* [x] defined type expressions
* [x] node definition expressions
* [ ] resource collection expressions
* [ ] exported resource collection expressions 
* [x] loading classes from modules
* [x] loading defined types from modules
* [x] access expressions
* [x] global scope
* [x] local scope
* [x] node scope
* [x] string interpolation
* [ ] external data lookups (i.e. hiera)
* [ ] EPP support

Type system implemented:

* [x] Any
* [x] Array
* [x] Boolean
* [ ] Callable
* [x] CatalogEntry
* [x] Collection
* [x] Data
* [x] Default
* [x] Enum
* [x] Float
* [x] Hash
* [x] Integer
* [x] Class
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
* [ ] defined
* [ ] digest
* [x] each
* [x] emerg
* [ ] epp
* [x] err
* [x] fail
* [ ] file
* [x] filter
* [ ] fqdn_rand
* [ ] generate
* [ ] hiera
* [ ] hiera_array
* [ ] hiera_hash
* [ ] hiera_include
* [x] include
* [x] info
* [ ] inline_epp
* [ ] inline_template
* [ ] lookup
* [ ] map
* [ ] match
* [ ] md5
* [x] notice
* [x] realize
* [ ] reduce
* [ ] regsubst
* [x] require
* [ ] scanf
* [ ] search
* [ ] sha1
* [ ] shellquote
* [ ] slice
* [x] split
* [ ] sprintf
* [ ] tag
* [ ] tagged
* [ ] template
* [x] versioncmp
* [x] warning
* [x] with

Catalog compiling status:

* [x] Facts from files and Facter
* [ ] JSON catalog generation (TODO: populate tags)

Build Requirements
------------------

* OSX or Linux
* GCC >= 5.0 or Clang >= 3.4 (with libc++)
* CMake >= 3.0
* Boost Libraries >= 1.57
* [Facter](https://github.com/puppetlabs/facter) >= 3.0 
* yaml-cpp >= 0.5.1 

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

You can run puppetcpp from its output directory:

`$ release/bin/puppetcpp <manifest>`

For a debug build:

`$ debug/bin/puppetcpp <manifest>`

Use the `--help` option for more options

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
