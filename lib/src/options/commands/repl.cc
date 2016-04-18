#include <puppet/options/commands/repl.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/compiler/evaluation/repl.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>

#ifdef USE_Editline
#include <editline/readline.h>
#endif

using namespace std;
using namespace puppet::compiler;
using namespace puppet::compiler::lexer;
using namespace puppet::utility::filesystem;
namespace x3 = boost::spirit::x3;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace sys = boost::system;

static void output_result(puppet::runtime::values::value const& result)
{
    cout << " => " << result << endl;
}

#ifdef USE_Editline
static void repl_loop(evaluation::context& context)
{
    auto history_file = home_directory();
    if (!history_file.empty()) {
        history_file = (fs::path{ history_file } / ".puppetrepl_history").string();
    }

    // Enable history
    using_history();

    if (!history_file.empty()) {
        read_history(history_file.c_str());
    }

    // Disable tab completion
    rl_bind_key('\t', rl_insert);

    // Create a REPL and loop
    evaluation::repl repl{
        context,
        [&](compilation_exception const& ex) {
            auto& logger = context.node().logger();
            LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), ex.what());
        }
    };

    while (auto line = readline(repl.prompt().c_str())) {
        if (line && repl.line() == 1 && strcmp(line, "exit") == 0) {
            free(line);
            break;
        }
        auto result = repl.evaluate(line);
        free(line);
        if (result) {
            output_result(result->value);
            add_history(result->source.c_str());
        }
    }

    // Writre the history back out
    if (!history_file.empty()) {
        write_history(history_file.c_str());
    }
}
#else
static void repl_loop(evaluation::context& context)
{
    // Create a REPL and loop
    evaluation::repl repl{
        context,
        [&](compilation_exception const& ex) {
            auto& logger = context.node().logger();
            LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), ex.what());
        }
    };

    string line;
    cout << repl.prompt() << flush;
    while (getline(cin, line)) {
        if (repl.line() == 1 && line == "exit") {
            break;
        }
        auto result = repl.evaluate(line);
        if (result) {
            output_result(result->value);
        }
        cout << repl.prompt() << flush;
    }
}
#endif

namespace puppet { namespace options { namespace commands {

    char const* repl::name() const
    {
        return "repl";
    }

    char const* repl::description() const
    {
        return "Runs an interactive Puppet shell.";
    }

    char const* repl::summary() const
    {
        // TODO: create
        return
            "Runs the read-evel-print-loop (REPL) shell for the Puppet language. The shell is capable of interactively "
            "evaluating Puppet code as if being evaluated from a manifest file."
            " <p> "
            "The REPL shell incrementally builds a resource catalog that can optionally be output after the shell is exited."
            " <p> "
            "To exit the shell, type 'exit' and hit <ENTER>."
            ;
    }

    char const* repl::arguments() const
    {
        return "";
    }

    po::options_description repl::create_options() const
    {
        // Keep this list sorted alphabetically on full option name
        po::options_description options("");
        options.add_options()
            (CODE_DIRECTORY_OPTION, po::value<string>(), CODE_DIRECTORY_DESCRIPTION)
            (COLOR_OPTION, COLOR_DESCRIPTION)
            (DEBUG_OPTION_FULL, DEBUG_DESCRIPTION)
            (ENVIRONMENT_OPTION_FULL, po::value<string>()->default_value("production"), ENVIRONMENT_DESCRIPTION)
            (ENVIRONMENT_PATH_OPTION, po::value<string>(), ENVIRONMENT_PATH_DESCRIPTION)
            (FACTS_OPTION_FULL, po::value<string>(), FACTS_DESCRIPTION)
            (GRAPH_FILE_OPTION_FULL, po::value<string>(), GRAPH_FILE_DESCRIPTION)
            (HELP_OPTION, HELP_DESCRIPTION)
            (command::LOG_LEVEL_OPTION_FULL, po::value<string>()->default_value("notice"), command::LOG_LEVEL_DESCRIPTION)
            (MODULE_PATH_OPTION, po::value<string>(), MODULE_PATH_DESCRIPTION)
            (NODE_OPTION_FULL, po::value<string>(), NODE_DESCRIPTION)
            (NO_COLOR_OPTION, NO_COLOR_DESCRIPTION)
            (OUTPUT_OPTION_FULL, po::value<string>(), OUTPUT_DESCRIPTION)
            (VERBOSE_OPTION, VERBOSE_DESCRIPTION)
            ;
        return options;
    }

    po::options_description repl::create_hidden_options() const
    {
        return command::create_hidden_options();
    }

    po::positional_options_description repl::create_positional_options() const
    {
        return command::create_positional_options();
    }

    executor repl::create_executor(po::variables_map const& options) const
    {
        if (options.count(HELP_OPTION)) {
            return parser().parse({ HELP_OPTION, name() });
        }

        // Get the options
        auto level = command::get_level(options);
        auto colorization = get_colorization(options);
        auto facts = get_facts(options);
        auto node_name = get_node(options, *facts);
        auto settings = create_settings(options);
        auto output_file = get_output_file(options);
        auto graph_file = get_graph_file(options);

        // Move the options into the lambda capture
        return {
            *this,
            [
                level,
                colorization,
                facts = rvalue_cast(facts),
                node_name = rvalue_cast(node_name),
                settings = rvalue_cast(settings),
                output_file = rvalue_cast(output_file),
                graph_file = rvalue_cast(graph_file),
                this
            ] () {
                logging::console_logger logger;
                logger.level(level);

                // TODO: support color/no-color options

                try {
                    auto environment = compiler::environment::create(logger, settings);
                    environment->dispatcher().add_builtins();
                    compiler::node node{ logger, node_name, rvalue_cast(environment), facts };
                    compiler::catalog catalog{ node.name(), node.environment().name() };
                    auto context = node.create_context(catalog);

                    // Create the 'repl' stack frame
                    evaluation::scoped_stack_frame frame{ context, evaluation::stack_frame{ "<repl>", context.top_scope(), false }};

                    repl_loop(context);

                    try {
                        context.finalize();
                        catalog.populate_graph();

                        if (!graph_file.empty()) {
                            ofstream file{ graph_file };
                            if (!file) {
                                LOG(error, "cannot open '%1%' for writing.", graph_file);
                            } else {
                                LOG(notice, "writing dependency graph to '%1%'.", graph_file);
                                catalog.write_graph(file);
                            }
                        }

                        catalog.detect_cycles();

                        if (!output_file.empty()) {
                            ofstream output{ output_file };
                            if (!output) {
                                LOG(error, "cannot open '%1%' for writing.", output_file);
                            } else {
                                LOG(notice, "writing catalog to '%1%'.", output_file);
                                catalog.write(output);
                            }
                        }
                    } catch (evaluation_exception const& ex) {
                        compilation_exception info{ ex };
                        LOG(error, info.line(), info.column(), info.length(), info.text(), info.path(), info.what());
                    } catch (resource_cycle_exception const& ex) {
                        LOG(error, ex.what());
                    }
                } catch (compilation_exception const& ex) {
                    throw option_exception(ex.what(), this);
                }
                return EXIT_SUCCESS;
            }
        };
    }

}}}  // namespace puppet::options::commands
