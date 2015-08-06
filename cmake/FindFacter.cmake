include(FindDependency)

find_dependency(Facter DISPLAY "facter" HEADERS "facter" LIBRARIES facter)

include(FeatureSummary)
set_package_properties(Facter PROPERTIES DESCRIPTION "A library for collecting system facts" URL "https://github.com/puppetlabs/facter")
set_package_properties(Facter PROPERTIES TYPE REQUIRED PURPOSE "Used to gather node facts.")
