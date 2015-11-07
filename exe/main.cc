#include <puppet/compiler/settings.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/facts/yaml.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

using namespace std;
using namespace puppet;
using namespace puppet::logging;
using namespace puppet::facts;
using namespace puppet::runtime;
using namespace puppet::compiler;
namespace fs = boost::filesystem;

int main(int argc, char const* argv[])
{
    console_logger logger;

    try {
        compiler::settings settings{argc, argv};

        if (settings.show_version()) {
            // TODO: output an actual version
            cout << "0.1.0" << endl;
            return EXIT_SUCCESS;
        }
        if (settings.show_help()) {
            settings.print_usage();
            return EXIT_SUCCESS;
        }

        logger.level(settings.log_level());

        // Log some useful information for debugging purposes
        LOG(debug, "using directory '%1%' as the code directory.", settings.code_directory());
        LOG(debug, "using directory '%1%' as the environment directory.", settings.environment_directory());
        for (auto const& directory : settings.module_directories()) {
            LOG(debug, "using directory '%1%' to search for global modules.", directory);
        }

        // Construct an environment and load the modules
        auto environment = make_shared<compiler::environment>(logger, settings, settings.environment(), settings.environment_directory());

        // Construct a node
        node node{logger, settings.node_name(), rvalue_cast(environment), settings.facts()};

        // Open the output file for writing
        auto output_file = (fs::current_path() / settings.output_file()).string();
        ofstream output(output_file);
        if (!output) {
            throw settings_exception((boost::format("cannot open '%1%' for writing.") % output_file).str());
        }

        try {
            LOG(notice, "compiling for node '%1%' with environment '%2%'.", settings.node_name(), settings.environment());

            // Compile the node
            auto catalog = node.compile();

            // Write the graph file if given one
            if (!settings.graph_file().empty()) {
                auto graph_file = (fs::current_path() / settings.graph_file()).string();
                ofstream file(graph_file);
                if (!file) {
                    LOG(error, "cannot open '%1%' for writing.", graph_file);
                } else {
                    LOG(notice, "writing dependency graph to '%1%'.", graph_file);
                    catalog.write_graph(file);
                }
            }

            // Detect dependency cycles
            catalog.detect_cycles();

            // Write the catalog
            LOG(notice, "writing catalog to '%1%'.", output_file);
            catalog.write(output);

        } catch (compilation_exception const& ex) {
            LOG(error, ex.line(), ex.column(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
        } catch (resource_cycle_exception const& ex) {
            LOG(error, ex.what());
        }
    } catch (yaml_parse_exception const& ex) {
        LOG(error, ex.line(), ex.column(), ex.text(), ex.path(), ex.what());
    } catch (settings_exception const& ex) {
        LOG(error, ex.what());
        LOG(notice, "use 'puppetcpp --help' for help.");
        return EXIT_FAILURE;
    } catch (exception const& ex) {
        LOG(critical, "unhandled exception: %1%", ex.what());
    }

    auto errors = logger.errors();
    auto warnings = logger.warnings();

    LOG(notice, "compilation %1% with %2% %3% and %4% %5%.",
        (errors > 0 ? "failed" : "succeeded"),
        errors,
        (errors != 1 ? "errors" : "error"),
        warnings,
        (warnings != 1 ? "warnings" : "warning")
    );
    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
