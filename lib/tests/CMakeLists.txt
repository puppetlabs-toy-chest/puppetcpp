include_directories(
    ../include/
    ${Boost_INCLUDE_DIRS}
    ${CATCH_INCLUDE_DIRS}
    ${DTL_INCLUDE_DIRS}
)

add_executable(puppet_test
    compiler/ast/ast.cc
    compiler/evaluation/evaluation.cc
    compiler/evaluation/repl.cc
    compiler/lexer/lexer.cc
    compiler/parser/parser.cc
    compiler/environment.cc
    options/commands/compile.cc
    options/commands/help.cc
    options/commands/parse.cc
    options/commands/repl.cc
    options/commands/version.cc
    options/parser.cc
    unicode/string.cc
    main.cc
)

add_definitions(
    -DFIXTURES_DIR="${CMAKE_CURRENT_LIST_DIR}/fixtures/"
)

# Disable the parenthesis warning for Catch macros
target_compile_options(puppet_test PUBLIC "-Wno-parentheses")
target_link_libraries(puppet_test puppet ${Boost_LIBRARIES} ${ICU_LIBRARIES})

cotire(puppet_test)

add_test(NAME "puppet\\ tests" COMMAND puppet_test)
