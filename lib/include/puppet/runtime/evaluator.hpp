/**
 * @file
 * Declares the Puppet language evaluator.
 */
#pragma once

#include "../ast/manifest.hpp"
#include "../lexer/lexer.hpp"
#include <puppet/runtime/expression_evaluator.hpp>

namespace puppet { namespace runtime {

    /**
     * Represents the Puppet language evaluator.
     */
    struct evaluator
    {
        /**
         * Evaluates the given AST manifest.
         * @tparam ErrorReporter The error reporter type.
         * @param reporter The error reporter to use during evaluation.
         * @param manifest The manifest to evaluate.
         * @param path The path to the manifest file.
         * @param input The manifest file's input stream.
         */
        template <typename ErrorReporter>
        void evaluate(ErrorReporter& reporter, ast::manifest& manifest, std::string const& path, std::ifstream& input)
        {
            if (!manifest.body()) {
                return;
            }

            try {
                context ctx;
                expression_evaluator expr_evaluator(ctx);
                for (auto& expression : *manifest.body()) {
                    expr_evaluator.evaluate(expression);
                }
            } catch (evaluation_exception const& ex) {
                std::string line;
                size_t column;
                tie(line, column) = lexer::get_line_and_column(input, std::get<0>(ex.position()));
                reporter.error_with_location(path, line, std::get<1>(ex.position()), column, ex.what());
            }
        }
    };

}}  // namespace puppet::runtime
