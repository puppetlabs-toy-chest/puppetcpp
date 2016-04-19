include(FindDependency)

find_dependency(Editline DISPLAY "editline" HEADERS "editline/readline.h" LIBRARIES "edit")

include(FeatureSummary)
set_package_properties(Editline PROPERTIES DESCRIPTION "A library for use by applications that allow users to edit command lines as they are typed in" URL "http://thrysoee.dk/editline")
set_package_properties(Editline PROPERTIES TYPE OPTIONAL PURPOSE "Enables a better experience for the 'repl' command.")
