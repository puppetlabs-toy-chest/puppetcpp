set(LIBPUPPET_VERSION_MAJOR 0)
set(LIBPUPPET_VERSION_MINOR 1)
set(LIBPUPPET_VERSION_PATCH 0)

include_directories(
    include/
    ${Boost_INCLUDE_DIRS}
    ${Facter_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${UTF8CPP_INCLUDE_DIRS}
)

# Set common sources
set(PUPPET_COMMON_SOURCES
    src/ast/access_expression.cc
    src/ast/array.cc
    src/ast/bare_word.cc
    src/ast/boolean.cc
    src/ast/case_expression.cc
    src/ast/class_definition_expression.cc
    src/ast/collection_expression.cc
    src/ast/defaulted.cc
    src/ast/defined_type_expression.cc
    src/ast/expression.cc
    src/ast/function_call_expression.cc
    src/ast/hash.cc
    src/ast/if_expression.cc
    src/ast/lambda.cc
    src/ast/method_call_expression.cc
    src/ast/name.cc
    src/ast/node_definition_expression.cc
    src/ast/number.cc
    src/ast/parameter.cc
    src/ast/postfix_expression.cc
    src/ast/regex.cc
    src/ast/selector_expression.cc
    src/ast/syntax_tree.cc
    src/ast/undef.cc
    src/ast/resource_defaults_expression.cc
    src/ast/resource_expression.cc
    src/ast/resource_override_expression.cc
    src/ast/string.cc
    src/ast/type.cc
    src/ast/unless_expression.cc
    src/ast/variable.cc
    src/lexer/lexer.cc
    src/lexer/number_token.cc
    src/lexer/position.cc
    src/lexer/token_id.cc
    src/logging/logger.cc
    src/compiler/context.cc
    src/compiler/environment.cc
    src/compiler/exceptions.cc
    src/compiler/module.cc
    src/compiler/node.cc
    src/compiler/parser.cc
    src/compiler/settings.cc
    src/facts/facter.cc
    src/facts/yaml.cc
    src/runtime/collectors/collector.cc
    src/runtime/collectors/list_collector.cc
    src/runtime/collectors/query_collector.cc
    src/runtime/collectors/query_evaluator.cc
    src/runtime/evaluators/access.cc
    src/runtime/evaluators/basic.cc
    src/runtime/evaluators/catalog.cc
    src/runtime/evaluators/control_flow.cc
    src/runtime/evaluators/postfix.cc
    src/runtime/evaluators/primary.cc
    src/runtime/functions/assert_type.cc
    src/runtime/functions/declare.cc
    src/runtime/functions/defined.cc
    src/runtime/functions/each.cc
    src/runtime/functions/fail.cc
    src/runtime/functions/filter.cc
    src/runtime/functions/logging.cc
    src/runtime/functions/map.cc
    src/runtime/functions/realize.cc
    src/runtime/functions/shellquote.cc
    src/runtime/functions/split.cc
    src/runtime/functions/versioncmp.cc
    src/runtime/functions/with.cc
    src/runtime/operators/assignment.cc
    src/runtime/operators/binary_context.cc
    src/runtime/operators/divide.cc
    src/runtime/operators/equals.cc
    src/runtime/operators/greater.cc
    src/runtime/operators/greater_equal.cc
    src/runtime/operators/in.cc
    src/runtime/operators/left_shift.cc
    src/runtime/operators/less.cc
    src/runtime/operators/less_equal.cc
    src/runtime/operators/logical_and.cc
    src/runtime/operators/logical_not.cc
    src/runtime/operators/logical_or.cc
    src/runtime/operators/match.cc
    src/runtime/operators/minus.cc
    src/runtime/operators/modulo.cc
    src/runtime/operators/multiply.cc
    src/runtime/operators/negate.cc
    src/runtime/operators/not_equals.cc
    src/runtime/operators/not_match.cc
    src/runtime/operators/plus.cc
    src/runtime/operators/relationship.cc
    src/runtime/operators/right_shift.cc
    src/runtime/operators/splat.cc
    src/runtime/operators/unary_context.cc
    src/runtime/types/any.cc
    src/runtime/types/boolean.cc
    src/runtime/types/callable.cc
    src/runtime/types/collection.cc
    src/runtime/types/data.cc
    src/runtime/types/defaulted.cc
    src/runtime/types/enumeration.cc
    src/runtime/types/floating.cc
    src/runtime/types/integer.cc
    src/runtime/types/numeric.cc
    src/runtime/types/pattern.cc
    src/runtime/types/regexp.cc
    src/runtime/types/scalar.cc
    src/runtime/types/string.cc
    src/runtime/types/type.cc
    src/runtime/types/undef.cc
    src/runtime/values/defaulted.cc
    src/runtime/values/regex.cc
    src/runtime/values/undef.cc
    src/runtime/values/value.cc
    src/runtime/catalog.cc
    src/runtime/context.cc
    src/runtime/definition_scanner.cc
    src/runtime/dispatcher.cc
    src/runtime/executor.cc
    src/runtime/expression_evaluator.cc
    src/runtime/scope.cc
    src/runtime/string_interpolator.cc
)

# Set platform-specific sources
if (UNIX)
    set(PUPPET_PLATFORM_SOURCES
        src/compiler/posix/settings.cc
    )
elseif(WIN32)
    set(PUPPET_PLATFORM_SOURCES
    )
endif()

# Add the executable for generating the static lexer
add_executable(generate_static_lexer src/lexer/generate_static_lexer.cc src/lexer/number_token.cc include/puppet/lexer/token_id.hpp src/lexer/token_id.cc src/lexer/position.cc)
target_link_libraries(generate_static_lexer ${Boost_LIBRARIES})

# Add the command to generate the static lexer
add_custom_command(COMMAND generate_static_lexer DEPENDS generate_static_lexer WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/lexer OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/lexer/static_lexer.hpp)
add_custom_target(generate_files ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/lexer/static_lexer.hpp)

add_library(puppet SHARED ${PUPPET_COMMON_SOURCES} ${PUPPET_PLATFORM_SOURCES})
add_dependencies(puppet generate_files)

set_target_properties(puppet PROPERTIES VERSION "${LIBPUPPET_VERSION_MAJOR}.${LIBPUPPET_VERSION_MINOR}.${LIBPUPPET_VERSION_PATCH}" COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY_UNITY")

target_link_libraries(puppet
    ${Boost_LIBRARIES}
    ${Facter_LIBRARY}
    ${YAMLCPP_LIBRARIES}
)

install(TARGETS puppet DESTINATION lib)

cotire(generate_static_lexer)
cotire(puppet)

add_subdirectory(tests)
