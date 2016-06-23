#include <catch.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/filesystem.hpp>
#include <dtl/dtl.hpp>
#include <cstdlib>
#include <regex>

using namespace std;
using namespace puppet::compiler::lexer;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::parser;

namespace ast = puppet::compiler::ast;
namespace x3 = boost::spirit::x3;
namespace fs = boost::filesystem;
namespace sys = boost::system;

struct test_logger : puppet::logging::stream_logger
{
    test_logger(ostream& stream) :
        _stream(stream)
    {
    }

 protected:
    ostream& get_stream(puppet::logging::level level) const override
    {
        return _stream;
    }

    void colorize(puppet::logging::level level) const override
    {
    }

    void reset(puppet::logging::level level) const override
    {
    }

 private:
    ostream& _stream;
};

static std::string normalize(std::string const& output)
{
    // Remove references to the fixture path
    static const std::regex path_regex{ (fs::path{FIXTURES_DIR} / "compiler" / "parser").string() + fs::path::preferred_separator };
    return regex_replace(output, path_regex, "");
}

static vector<std::string> read_lines(istream& stream)
{
    vector<std::string> lines;
    std::string line;

    while (getline(stream, line)) {
        lines.emplace_back(puppet::rvalue_cast(line));
    }
    return lines;
}

static std::string calculate_difference(stringstream& buffer, vector<std::string> const& baseline)
{
    auto lines = read_lines(buffer);

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
    auto begin = fs::directory_iterator{FIXTURES_DIR "compiler/parser", ec};
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

        if (generate) {
            WARN("generating baseline file " << baseline_path);

            ostringstream buffer;
            test_logger logger{ buffer };
            try {
                auto tree = parse_file(logger, path.string(), nullptr, is_epp);
                tree->validate();
                tree->write(format::yaml, buffer);
                buffer << '\n';
            } catch (puppet::compiler::parse_exception const& ex) {
                puppet::compiler::compilation_exception exception{ ex, path.string() };
                LOG(error, exception.line(), exception.column(), exception.length(), exception.text(), exception.path(), exception.what());
            }

            ofstream output{ baseline_path.string() };
            REQUIRE(output);
            output << normalize(buffer.str());
        }

        static auto const dummy_module = reinterpret_cast<puppet::compiler::module*>(0x1234);

        CAPTURE(path);
        CAPTURE(baseline_path);

        ifstream baseline{baseline_path.string()};
        REQUIRE(baseline);
        auto baseline_lines = read_lines(baseline);

        // First parse the file
        {
            stringstream buffer;
            test_logger logger{ buffer };

            try {
                auto tree = parse_file(logger, path.string(), dummy_module, is_epp);
                tree->validate();
                REQUIRE(tree);
                REQUIRE(tree->module() == dummy_module);
                REQUIRE(tree->path() == path.string());
                REQUIRE(tree->shared_path());
                REQUIRE(tree->source().empty());
                tree->write(format::yaml, buffer);
            } catch (puppet::compiler::parse_exception const& ex) {
                puppet::compiler::compilation_exception exception{ ex, path.string() };
                LOG(error, exception.line(), exception.column(), exception.length(), exception.text(), exception.path(), exception.what());
            }

            buffer.str(normalize(buffer.str()));
            auto difference = calculate_difference(buffer, baseline_lines);
            CAPTURE(difference);
            REQUIRE(difference.empty());
        }

        // Next read the file as a string and parse the string
        {
            std::string source;
            {
                ifstream file(path.string());
                REQUIRE(file);
                ostringstream buffer;
                buffer << file.rdbuf();
                source = buffer.str();
            }

            stringstream buffer;
            test_logger logger{ buffer };

            try {
                auto tree = parse_string(logger, source, path.string(), dummy_module, is_epp);
                tree->validate();
                REQUIRE(tree);
                REQUIRE(tree->module() == dummy_module);
                REQUIRE(tree->path() == path.string());
                REQUIRE(tree->shared_path());
                REQUIRE(tree->source() == source);
                tree->write(format::yaml, buffer);
            } catch (puppet::compiler::parse_exception const& ex) {
                puppet::compiler::compilation_exception exception{ ex, path.string(), source };
                LOG(error, exception.line(), exception.column(), exception.length(), exception.text(), exception.path(), exception.what());
            }

            buffer.str(normalize(buffer.str()));
            auto difference = calculate_difference(buffer, baseline_lines);
            CAPTURE(difference);
            REQUIRE(difference.empty());
        }
    }
}
