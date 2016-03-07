#include <catch.hpp>
#include <puppet/compiler/evaluation/repl.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet;
using namespace puppet::compiler;
using namespace puppet::compiler::evaluation;

SCENARIO("using the repl evaluator", "[evaluation]")
{
    compiler::settings settings;
    logging::console_logger logger;

    auto environment = compiler::environment::create(logger, settings);
    environment->dispatcher().add_builtins();
    compiler::node node{ logger, "test", rvalue_cast(environment), nullptr };
    compiler::catalog catalog{ node.name(), node.environment().name() };
    auto context = node.create_context(catalog);

    // Create the 'repl' stack frame
    evaluation::scoped_stack_frame frame{ context, evaluation::stack_frame{ "<repl>", context.top_scope(), false }};

    boost::optional<compilation_exception> exception;
    evaluation::repl repl{
        context,
        [&](compilation_exception const& ex) {
            exception = ex;
        }
    };

    REQUIRE(repl.prompt() == "test:001:1> ");
    REQUIRE(repl.count() == 1);
    REQUIRE(repl.line() == 1);

    WHEN("given a simple statement") {
        auto result = repl.evaluate("1 + 1");
        THEN("it should evaluate to the expected value") {
            REQUIRE(result);
            REQUIRE(result->source == "1 + 1");
            REQUIRE(boost::lexical_cast<string>(result->value) == "2");
            REQUIRE(repl.prompt() == "test:002:1> ");
            REQUIRE(repl.count() == 2);
            REQUIRE(repl.line() == 1);
            REQUIRE_FALSE(exception);
        }
    }
    WHEN("given a multiline statement") {
        AND_WHEN("given the first line") {
            auto result = repl.evaluate("class foo");
            THEN("it should not yet be evaluated") {
                REQUIRE_FALSE(result);
                REQUIRE_FALSE(exception);
                REQUIRE(repl.prompt() == "test:001:2> ");
                REQUIRE(repl.count() == 1);
                REQUIRE(repl.line() == 2);
            }
            AND_WHEN("given the second line") {
                auto result = repl.evaluate("($param)");
                THEN("it should not yet be evaluated") {
                    REQUIRE_FALSE(result);
                    REQUIRE_FALSE(exception);
                    REQUIRE(repl.prompt() == "test:001:3> ");
                    REQUIRE(repl.count() == 1);
                    REQUIRE(repl.line() == 3);
                }
                AND_WHEN("given the third line") {
                    auto result = repl.evaluate("{");
                    THEN("it should not yet be evaluated") {
                        REQUIRE_FALSE(result);
                        REQUIRE_FALSE(exception);
                        REQUIRE(repl.prompt() == "test:001:4> ");
                        REQUIRE(repl.count() == 1);
                        REQUIRE(repl.line() == 4);
                    }
                    AND_WHEN("given the fourth line") {
                        auto result = repl.evaluate("notice $param");
                        THEN("it should not yet be evaluated") {
                            REQUIRE_FALSE(result);
                            REQUIRE_FALSE(exception);
                            REQUIRE(repl.prompt() == "test:001:5> ");
                            REQUIRE(repl.count() == 1);
                            REQUIRE(repl.line() == 5);
                        }
                        AND_WHEN("given the fifth and final line") {
                            auto result = repl.evaluate("}");
                            THEN("it should evaluate to the expected value") {
                                REQUIRE(result);
                                REQUIRE(result->source == "class foo\n($param)\n{\nnotice $param\n}");
                                REQUIRE(boost::lexical_cast<string>(result->value) == "Class[foo]");
                                REQUIRE(repl.prompt() == "test:002:1> ");
                                REQUIRE(repl.count() == 2);
                                REQUIRE(repl.line() == 1);
                                REQUIRE_FALSE(exception);
                            }
                        }
                    }
                }
            }
        }
    }
    WHEN("given a command with a syntax error") {
        auto result = repl.evaluate("class foo bar {}");
        THEN("a compilation error should be provided") {
            REQUIRE_FALSE(result);
            REQUIRE(repl.prompt() == "test:002:1> ");
            REQUIRE(repl.count() == 2);
            REQUIRE(repl.line() == 1);
            REQUIRE(exception);
            REQUIRE(exception->what() == string{"expected '{' but found name."});
            REQUIRE(exception->path() == "<repl>");
            REQUIRE(exception->line() == 1);
            REQUIRE(exception->column() == 11);
            REQUIRE(exception->length() == 3);
            REQUIRE(exception->text() == "class foo bar {}");
        }
    }
}
