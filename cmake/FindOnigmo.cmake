include(FindDependency)

find_dependency(Onigmo DISPLAY "Onigmo" HEADERS "onigmo.h" LIBRARIES libonigmo.so libonigmo.dylib libonigmo.dll)

include(FeatureSummary)
set_package_properties(Onigmo PROPERTIES DESCRIPTION "A regular expressions library, used in current Ruby versions, that was forked from Oniguruma" URL "https://github.com/k-takata/Onigmo")
set_package_properties(Onigmo PROPERTIES TYPE REQUIRED PURPOSE "Used to provide compatibility with Puppet regular expressions which rely on Ruby.")
