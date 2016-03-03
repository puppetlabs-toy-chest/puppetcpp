#include <catch.hpp>
#include <puppet/compiler/environment.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace puppet;
using namespace puppet::compiler;
namespace fs = boost::filesystem;
namespace sys = boost::system;

SCENARIO("environment with only manifests", "[environment]")
{
    puppet::logging::console_logger logger;

    const string environment_name = "manifests_only";

    fs::path environments_dir = fs::path{FIXTURES_DIR} / "compiler" / "environments";
    fs::path environment_dir  = environments_dir / environment_name;
    fs::path manifests_dir  = environment_dir / "manifests";

    compiler::settings settings;
    settings.set(settings::environment_path, environments_dir.string());
    settings.set(settings::environment, environment_name);
    settings.set(settings::base_module_path, "");

    auto environment = puppet::compiler::environment::create(logger, settings);
    REQUIRE(environment->name() == environment_name);
    REQUIRE(environment->directory() == environment_dir.string());

    auto& modules = environment->modules();
    REQUIRE(modules.empty());

    vector<string> manifests;
    environment->each_file(find_type::manifest, [&](auto& path) {
        manifests.emplace_back(path);
        return true;
    });
    REQUIRE(manifests.size() == 2);
    REQUIRE(manifests[0] == (manifests_dir / "bar.pp").string());
    REQUIRE(manifests[1] == (manifests_dir / "foo.pp").string());
}

SCENARIO("environment with modules", "[environment]")
{
    puppet::logging::console_logger logger;

    const string environment_name = "has_modules";

    fs::path environments_dir = fs::path{FIXTURES_DIR} / "compiler" / "environments";
    fs::path environment_dir  = environments_dir / environment_name;
    fs::path modules_dir  = environment_dir / "modules";
    fs::path manifests_dir  = environment_dir / "manifests";

    compiler::settings settings;
    settings.set(settings::environment_path, environments_dir.string());
    settings.set(settings::environment, environment_name);
    settings.set(settings::base_module_path, "");

    auto environment = puppet::compiler::environment::create(logger, settings);
    REQUIRE(environment->name() == environment_name);
    REQUIRE(environment->directory() == environment_dir.string());

    auto& modules = environment->modules();
    REQUIRE(modules.size() == 3);
    REQUIRE(modules[0].name() == "bar");
    REQUIRE(modules[0].directory() == (modules_dir / "bar").string());
    REQUIRE(modules[1].name() == "baz");
    REQUIRE(modules[1].directory() == (modules_dir / "baz").string());
    REQUIRE(modules[2].name() == "foo");
    REQUIRE(modules[2].directory() == (modules_dir / "foo").string());

    vector<string> manifests;
    environment->each_file(find_type::manifest, [&](auto& path) {
        manifests.emplace_back(path);
        return true;
    });
    REQUIRE(manifests.size() == 1);
    REQUIRE(manifests[0] == (manifests_dir / "site.pp").string());
}

SCENARIO("environment with configuration file", "[environment]")
{
    puppet::logging::console_logger logger;

    const string environment_name = "configuration";

    fs::path environments_dir = fs::path{FIXTURES_DIR} / "compiler" / "environments";
    fs::path environment_dir  = environments_dir / environment_name;
    fs::path modules_dir  = environment_dir / "dist";

    compiler::settings settings;
    settings.set(settings::environment_path, environments_dir.string());
    settings.set(settings::environment, environment_name);
    settings.set(settings::base_module_path, "");

    auto environment = puppet::compiler::environment::create(logger, settings);
    REQUIRE(environment->name() == environment_name);
    REQUIRE(environment->directory() == environment_dir.string());

    auto& modules = environment->modules();
    REQUIRE(modules.size() == 3);
    REQUIRE(modules[0].name() == "foo");
    REQUIRE(modules[0].directory() == (modules_dir / "foo").string());
    REQUIRE(modules[1].name() == "foobar");
    REQUIRE(modules[1].directory() == (modules_dir / "foobar").string());
    REQUIRE(modules[2].name() == "zed");
    REQUIRE(modules[2].directory() == (modules_dir / "zed").string());

    vector<string> manifests;
    environment->each_file(find_type::manifest, [&](auto& path) {
        manifests.emplace_back(path);
        return true;
    });
    REQUIRE(manifests.size() == 1);
    REQUIRE(manifests[0] == (environment_dir / "site.pp").string());
}
