include_directories(
    ../include/
    ${Boost_INCLUDE_DIRS}
    ${CATCH_INCLUDE_DIRS}
    ${DTL_INCLUDE_DIRS}
)

add_executable(puppet_test
    compiler/ast/ast.cc
    compiler/lexer/lexer.cc
    compiler/parser/parser.cc
    compiler/environment.cc
    options/commands/compile.cc
    options/commands/help.cc
    options/commands/version.cc
    options/parser.cc
    main.cc
)

add_definitions(
    -DFIXTURES_DIR="${CMAKE_CURRENT_LIST_DIR}/fixtures/"
)

set_target_properties(puppet_test PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
target_link_libraries(puppet_test puppet ${Boost_LIBRARIES})

cotire(puppet_test)

add_test(NAME "puppet\\ tests" COMMAND puppet_test)
