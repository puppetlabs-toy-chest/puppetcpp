#include <catch.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/filesystem.hpp>
#include <dtl/dtl.hpp>
#include <cstdlib>

using namespace std;
using namespace puppet::compiler::lexer;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::parser;

namespace ast = puppet::compiler::ast;
namespace x3 = boost::spirit::x3;
namespace fs = boost::filesystem;
namespace sys = boost::system;

static vector<std::string> read_lines(istream& stream)
{
    vector<std::string> lines;
    std::string line;

    while (getline(stream, line)) {
        lines.emplace_back(puppet::rvalue_cast(line));
    }
    return lines;
}

static std::string calculate_difference(syntax_tree const& tree, vector<std::string> const& baseline)
{
    // Read the lines of the YAML output of the syntax tree
    stringstream output;
    tree.write(format::yaml, output, false /* no path */);
    auto lines = read_lines(output);

    dtl::Diff<std::string> diff{baseline, lines};
    diff.onHuge();
    diff.compose();
    diff.composeUnifiedHunks();

    stringstream diffstream;
    diff.printUnifiedFormat(diffstream);
    return diffstream.str();
}

SCENARIO("parsing files", "[parser]")
{
    bool generate = getenv("PUPPET_GENERATE_BASELINE");
    if (generate) {
        WARN("generating new baseline files: please check the files for correctness.");
    }

    sys::error_code ec;
    auto begin = fs::directory_iterator{FIXTURES_DIR "compiler/parser/good", ec};
    REQUIRE_FALSE(ec);

    auto end = fs::directory_iterator{};
    for (; begin != end; ++begin) {
        auto& path = begin->path();

        ec.clear();
        auto status = begin->status(ec);
        if (ec || (status.type() != fs::regular_file)) {
            continue;
        }

        bool is_epp = path.extension() == ".epp";

        if (path.extension() != ".pp" && !is_epp) {
            if (path.extension() != ".baseline") {
                WARN("ignoring file found in fixtures directory: " << path);
            }
            continue;
        }

        auto baseline_path = path;
        baseline_path.replace_extension(".baseline");

        try {
            if (generate) {
                WARN("generating baseline file " << baseline_path);
                auto tree = parse_file(path.string(), nullptr, is_epp);
                ofstream stream(baseline_path.string());
                REQUIRE(stream);
                tree->write(format::yaml, stream, false /* don't include paths */);
                stream << '\n';
            }

            static auto const dummy_module = reinterpret_cast<puppet::compiler::module*>(0x1234);

            CAPTURE(path);
            CAPTURE(baseline_path);

            ifstream baseline{baseline_path.string()};
            REQUIRE(baseline);
            auto lines = read_lines(baseline);

            // First parse the file
            auto tree = parse_file(path.string(), dummy_module, is_epp);
            REQUIRE(tree);
            REQUIRE(tree->module() == dummy_module);
            REQUIRE(tree->path() == path.string());
            REQUIRE(tree->shared_path());
            REQUIRE(tree->source().empty());
            auto difference = calculate_difference(*tree, lines);
            CAPTURE(difference);
            REQUIRE(difference.empty());

            // Next read the file as a string and parse the string
            ifstream file(path.string());
            REQUIRE(file);
            ostringstream buffer;
            buffer << file.rdbuf();
            auto source = buffer.str();
            tree = parse_string(source, path.string(), dummy_module, is_epp);
            REQUIRE(tree);
            REQUIRE(tree->module() == dummy_module);
            REQUIRE(tree->path() == path.string());
            REQUIRE(tree->shared_path());
            REQUIRE(tree->source() == source);
            difference = calculate_difference(*tree, lines);
            CAPTURE(difference);
            REQUIRE(difference.empty());
        } catch (puppet::compiler::parse_exception const& ex) {
            std::string text;
            size_t column;
            ifstream stream(path.string());
            tie(text, column) = get_text_and_column(stream, ex.range().begin().offset());
            FAIL("parse error: " << path << ":" << ex.range().begin().line() << ":" << column << ": " << ex.what() << "\ntext: " << text);
        }
    }
}
