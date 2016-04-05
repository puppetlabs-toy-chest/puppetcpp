set(LIBPUPPET_VERSION_MAJOR 0)
set(LIBPUPPET_VERSION_MINOR 1)
set(LIBPUPPET_VERSION_PATCH 0)

include_directories(
    include/
    ${Boost_INCLUDE_DIRS}
    ${Facter_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${UTF8CPP_INCLUDE_DIRS}
    ${Readline_INCLUDE_DIR}
)

# Set common sources
set(PUPPET_COMMON_SOURCES
    src/compiler/ast/ast.cc
    src/compiler/evaluation/collectors/collector.cc
    src/compiler/evaluation/collectors/list_collector.cc
    src/compiler/evaluation/collectors/query_collector.cc
    src/compiler/evaluation/collectors/query_evaluator.cc
    src/compiler/evaluation/functions/alert.cc
    src/compiler/evaluation/functions/assert_type.cc
    src/compiler/evaluation/functions/call_context.cc
    src/compiler/evaluation/functions/contain.cc
    src/compiler/evaluation/functions/crit.cc
    src/compiler/evaluation/functions/debug.cc
    src/compiler/evaluation/functions/defined.cc
    src/compiler/evaluation/functions/descriptor.cc
    src/compiler/evaluation/functions/each.cc
    src/compiler/evaluation/functions/emerg.cc
    src/compiler/evaluation/functions/err.cc
    src/compiler/evaluation/functions/fail.cc
    src/compiler/evaluation/functions/filter.cc
    src/compiler/evaluation/functions/include.cc
    src/compiler/evaluation/functions/info.cc
    src/compiler/evaluation/functions/inline_epp.cc
    src/compiler/evaluation/functions/map.cc
    src/compiler/evaluation/functions/notice.cc
    src/compiler/evaluation/functions/realize.cc
    src/compiler/evaluation/functions/reduce.cc
    src/compiler/evaluation/functions/require.cc
    src/compiler/evaluation/functions/split.cc
    src/compiler/evaluation/functions/tag.cc
    src/compiler/evaluation/functions/tagged.cc
    src/compiler/evaluation/functions/versioncmp.cc
    src/compiler/evaluation/functions/warning.cc
    src/compiler/evaluation/functions/with.cc
    src/compiler/evaluation/operators/binary/assignment.cc
    src/compiler/evaluation/operators/binary/call_context.cc
    src/compiler/evaluation/operators/binary/descriptor.cc
    src/compiler/evaluation/operators/binary/divide.cc
    src/compiler/evaluation/operators/binary/equals.cc
    src/compiler/evaluation/operators/binary/greater.cc
    src/compiler/evaluation/operators/binary/greater_equal.cc
    src/compiler/evaluation/operators/binary/in.cc
    src/compiler/evaluation/operators/binary/in_edge.cc
    src/compiler/evaluation/operators/binary/in_edge_subscribe.cc
    src/compiler/evaluation/operators/binary/left_shift.cc
    src/compiler/evaluation/operators/binary/less.cc
    src/compiler/evaluation/operators/binary/less_equal.cc
    src/compiler/evaluation/operators/binary/logical_and.cc
    src/compiler/evaluation/operators/binary/logical_or.cc
    src/compiler/evaluation/operators/binary/match.cc
    src/compiler/evaluation/operators/binary/minus.cc
    src/compiler/evaluation/operators/binary/modulo.cc
    src/compiler/evaluation/operators/binary/multiply.cc
    src/compiler/evaluation/operators/binary/not_equals.cc
    src/compiler/evaluation/operators/binary/not_match.cc
    src/compiler/evaluation/operators/binary/out_edge.cc
    src/compiler/evaluation/operators/binary/out_edge_subscribe.cc
    src/compiler/evaluation/operators/binary/plus.cc
    src/compiler/evaluation/operators/binary/right_shift.cc
    src/compiler/evaluation/operators/unary/call_context.cc
    src/compiler/evaluation/operators/unary/descriptor.cc
    src/compiler/evaluation/operators/unary/logical_not.cc
    src/compiler/evaluation/operators/unary/negate.cc
    src/compiler/evaluation/operators/unary/splat.cc
    src/compiler/evaluation/access_evaluator.cc
    src/compiler/evaluation/context.cc
    src/compiler/evaluation/dispatcher.cc
    src/compiler/evaluation/evaluator.cc
    src/compiler/evaluation/postfix_evaluator.cc
    src/compiler/evaluation/repl.cc
    src/compiler/evaluation/scope.cc
    src/compiler/evaluation/stack_frame.cc
    src/compiler/lexer/lexer.cc
    src/compiler/lexer/position.cc
    src/compiler/lexer/tokens.cc
    src/compiler/parser/parser.cc
    src/compiler/validation/type_validator.cc
    src/compiler/attribute.cc
    src/compiler/catalog.cc
    src/compiler/environment.cc
    src/compiler/exceptions.cc
    src/compiler/finder.cc
    src/compiler/module.cc
    src/compiler/node.cc
    src/compiler/registry.cc
    src/compiler/resource.cc
    src/compiler/scanner.cc
    src/compiler/settings.cc
    src/facts/facter.cc
    src/facts/yaml.cc
    src/logging/logger.cc
    src/options/commands/compile.cc
    src/options/commands/help.cc
    src/options/commands/parse.cc
    src/options/commands/repl.cc
    src/options/commands/version.cc
    src/options/command.cc
    src/options/executor.cc
    src/options/parser.cc
    src/runtime/types/alias.cc
    src/runtime/types/any.cc
    src/runtime/types/array.cc
    src/runtime/types/boolean.cc
    src/runtime/types/callable.cc
    src/runtime/types/catalog_entry.cc
    src/runtime/types/class.cc
    src/runtime/types/collection.cc
    src/runtime/types/data.cc
    src/runtime/types/defaulted.cc
    src/runtime/types/enumeration.cc
    src/runtime/types/floating.cc
    src/runtime/types/hash.cc
    src/runtime/types/integer.cc
    src/runtime/types/iterable.cc
    src/runtime/types/iterator.cc
    src/runtime/types/not_undef.cc
    src/runtime/types/numeric.cc
    src/runtime/types/optional.cc
    src/runtime/types/pattern.cc
    src/runtime/types/regexp.cc
    src/runtime/types/resource.cc
    src/runtime/types/runtime.cc
    src/runtime/types/scalar.cc
    src/runtime/types/string.cc
    src/runtime/types/struct.cc
    src/runtime/types/tuple.cc
    src/runtime/types/type.cc
    src/runtime/types/undef.cc
    src/runtime/types/variant.cc
    src/runtime/values/array.cc
    src/runtime/values/defaulted.cc
    src/runtime/values/hash.cc
    src/runtime/values/iterator.cc
    src/runtime/values/regex.cc
    src/runtime/values/type.cc
    src/runtime/values/undef.cc
    src/runtime/values/value.cc
    src/runtime/values/variable.cc
    src/runtime/values/wrapper.cc
    src/utility/filesystem/helpers.cc
)

# Set platform-specific sources
if (UNIX)
    set(PUPPET_PLATFORM_SOURCES
        src/compiler/posix/settings.cc
        src/utility/filesystem/posix/helpers.cc
    )
elseif(WIN32)
    set(PUPPET_PLATFORM_SOURCES
    )
endif()

# Add the executable for generating the static lexer
add_executable(
    generate_static_lexer
    src/compiler/lexer/generate_static_lexer.cc
    src/compiler/lexer/lexer.cc
    src/compiler/lexer/position.cc
    src/compiler/lexer/tokens.cc
)
target_link_libraries(generate_static_lexer ${Boost_LIBRARIES})

# Add the command to generate the static lexer
add_custom_command(COMMAND generate_static_lexer DEPENDS generate_static_lexer WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/compiler/lexer OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/compiler/lexer/static_lexer.hpp)
add_custom_target(generate_files ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/puppet/compiler/lexer/static_lexer.hpp)

add_library(puppet SHARED ${PUPPET_COMMON_SOURCES} ${PUPPET_PLATFORM_SOURCES})
add_dependencies(puppet generate_files)

set_target_properties(puppet PROPERTIES VERSION "${LIBPUPPET_VERSION_MAJOR}.${LIBPUPPET_VERSION_MINOR}.${LIBPUPPET_VERSION_PATCH}" COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY_UNITY")

target_link_libraries(puppet
    ${Boost_LIBRARIES}
    ${Facter_LIBRARY}
    ${YAMLCPP_LIBRARIES}
    ${Readline_LIBRARY}
)

install(TARGETS puppet DESTINATION lib)

add_subdirectory(tests)
