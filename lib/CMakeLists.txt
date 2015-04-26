set(LIBPUPPET_VERSION_MAJOR 0)
set(LIBPUPPET_VERSION_MINOR 1)
set(LIBPUPPET_VERSION_PATCH 0)

include_directories(
    include/
    ${Boost_INCLUDE_DIRS}
)

# Add the executable for generating the static lexer
add_executable(generate_static_lexer src/lexer/generate_static_lexer.cc src/lexer/number_token.cc include/puppet/lexer/token_id.hpp src/lexer/token_id.cc)
target_link_libraries(generate_static_lexer ${Boost_LIBRARIES})

# Add the command to generate the static lexer
add_custom_command(COMMAND generate_static_lexer DEPENDS generate_static_lexer WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/lexer OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/lexer/static_lexer.hpp)

add_library(puppet SHARED
    src/lexer/lexer.cc
    src/lexer/number_token.cc
    src/lexer/token_id.cc
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
    src/ast/manifest.cc
    src/ast/method_call_expression.cc
    src/ast/name.cc
    src/ast/node_definition_expression.cc
    src/ast/number.cc
    src/ast/parameter.cc
    src/ast/regex.cc
    src/ast/selector_expression.cc
    src/ast/undef.cc
    src/ast/resource_defaults_expression.cc
    src/ast/resource_expression.cc
    src/ast/resource_override_expression.cc
    src/ast/string.cc
    src/ast/type.cc
    src/ast/unless_expression.cc
    src/ast/variable.cc
    src/logging/logger.cc
    src/parser/parser.cc
    src/runtime/functions/fail.cc
    src/runtime/functions/logging.cc
    src/runtime/functions/split.cc
    src/runtime/functions/with.cc
    src/runtime/types/any.cc
    src/runtime/types/boolean.cc
    src/runtime/types/callable.cc
    src/runtime/types/catalog_entry.cc
    src/runtime/types/collection.cc
    src/runtime/types/data.cc
    src/runtime/types/defaulted.cc
    src/runtime/types/enumeration.cc
    src/runtime/types/floating.cc
    src/runtime/types/integer.cc
    src/runtime/types/klass.cc
    src/runtime/types/numeric.cc
    src/runtime/types/pattern.cc
    src/runtime/types/regexp.cc
    src/runtime/types/resource.cc
    src/runtime/types/scalar.cc
    src/runtime/types/string.cc
    src/runtime/types/type.cc
    src/runtime/types/undef.cc
    src/runtime/values/defaulted.cc
    src/runtime/values/regex.cc
    src/runtime/values/undef.cc
    src/runtime/values/value.cc
    src/runtime/context.cc
    src/runtime/dispatcher.cc
    src/runtime/expression_evaluator.cc
    src/runtime/operators.cc
    src/runtime/scope.cc
    src/runtime/string_interpolator.cc
    src/runtime/yielder.cc
)
add_dependencies(puppet generate_static_lexer)

set_target_properties(puppet PROPERTIES VERSION "${LIBPUPPET_VERSION_MAJOR}.${LIBPUPPET_VERSION_MINOR}.${LIBPUPPET_VERSION_PATCH}" COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY_UNITY")

target_link_libraries(puppet
    ${Boost_LIBRARIES}
)

install(TARGETS puppet DESTINATION lib)

cotire(generate_static_lexer)
cotire(puppet)

add_subdirectory(tests)
