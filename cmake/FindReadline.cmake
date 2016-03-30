include(FindDependency)

find_dependency(Readline DISPLAY "readline" HEADERS "readline/readline.h" LIBRARIES "readline")

include(FeatureSummary)
set_package_properties(Readline PROPERTIES DESCRIPTION "A library for use by applications that allow users to edit command lines as they are typed in." URL "https://cnswww.cns.cwru.edu/php/chet/readline/rltop.html")
set_package_properties(Readline PROPERTIES TYPE OPTIONAL PURPOSE "Enables a better experience for the 'repl' command.")
