#include <catch.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/filesystem.hpp>
#include <dtl/dtl.hpp>
#include <cstdlib>
#include <regex>

using namespace std;
using namespace puppet;
using namespace puppet::compiler;
using namespace puppet::logging;
namespace fs = boost::filesystem;
namespace sys = boost::system;

struct test_logger : stream_logger
{
    test_logger(ostream& stream) :
        _stream(stream)
    {
    }

 protected:
    ostream& get_stream(logging::level level) const override
    {
        return _stream;
    }

    void colorize(logging::level level) const override
    {
    }

    void reset(logging::level level) const override
    {
    }

 private:
    ostream& _stream;
};

static string normalize(string const& output)
{
    // Remove catalog versions
    static const std::regex version_regex{ R"((\s*"version":\s*)\d+,)" };

    // Remove references to the fixture path
    static const std::regex path_regex{ (fs::path{FIXTURES_DIR} / "compiler" / "evaluation").string() + fs::path::preferred_separator };

    auto result = regex_replace(output, version_regex, "$01123456789");
    result = regex_replace(result, path_regex, "");
    return result;
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

SCENARIO("evaluating manifests", "[evaluation]")
{
    bool generate = getenv("PUPPET_GENERATE_BASELINE");
    if (generate) {
        WARN("generating new baseline files: please check the files for correctness.");
    }

    sys::error_code ec;
    auto begin = fs::directory_iterator{fs::path{FIXTURES_DIR} / "compiler" / "evaluation", ec};
    REQUIRE_FALSE(ec);

    auto end = fs::directory_iterator{};
    for (; begin != end; ++begin) {
        auto& path = begin->path();

        ec.clear();
        auto status = begin->status(ec);
        if (ec || (status.type() != fs::regular_file)) {
            continue;
        }

        if (path.extension() != ".pp") {
            if (path.extension() != ".baseline") {
                WARN("ignoring file found in fixtures directory: " << path);
            }
            continue;
        }

        auto baseline_path = path;
        baseline_path.replace_extension(".baseline");

        compiler::settings settings;

        if (generate) {
            WARN("generating baseline file " << baseline_path);

            ostringstream buffer;
            test_logger logger{ buffer };
            auto environment = compiler::environment::create(logger, settings);
            environment->dispatcher().add_builtins();
            compiler::node node{ logger, "test", environment, nullptr };

            try {
                auto catalog = node.compile({ path.string() });
                catalog.write(buffer);
                buffer << '\n';
            } catch (compilation_exception const& ex) {
                LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
                logger.log(ex.backtrace());
            }

            ofstream output{ baseline_path.string() };
            REQUIRE(output);
            output << normalize(buffer.str());
        }

        static auto const dummy_module = reinterpret_cast<puppet::compiler::module*>(0x1234);

        CAPTURE(path);
        CAPTURE(baseline_path);

        ifstream baseline{ baseline_path.string() };
        REQUIRE(baseline);
        auto lines = read_lines(baseline);

        stringstream buffer;
        test_logger logger{ buffer };
        auto environment = compiler::environment::create(logger, settings);
        environment->dispatcher().add_builtins();
        compiler::node node{ logger, "test", environment, nullptr };

        try {
            auto catalog = node.compile({ path.string() });
            catalog.write(buffer);
            buffer << '\n';
        } catch (compilation_exception const& ex) {
            LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
            logger.log(ex.backtrace());
        }

        buffer.str(normalize(buffer.str()));
        auto difference = calculate_difference(buffer, lines);
        CAPTURE(difference);
        REQUIRE(difference.empty());
    }
}
