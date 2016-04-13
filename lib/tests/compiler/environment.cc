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

void import(puppet::logging::logger& logger, puppet::compiler::environment& environment, find_type type, string const& name, bool expected = true)
{
    CAPTURE(name);

    if (type == find_type::function) {
        REQUIRE_FALSE(environment.dispatcher().find(name));
    } else if (type == find_type::type) {
        REQUIRE_FALSE(environment.registry().find_type_alias(name));
    } else {
        REQUIRE_FALSE((environment.registry().find_class(name) || environment.registry().find_defined_type(name)));
    }

    environment.import(logger, type, name);

    if (type == find_type::function) {
        auto function = environment.dispatcher().find(name);
        if (expected) {
            REQUIRE(function);
        } else {
            REQUIRE_FALSE(function);
        }
    } else if (type == find_type::type) {
        auto alias = environment.registry().find_type_alias(name);
        if (expected) {
            REQUIRE(alias);
        } else {
            REQUIRE_FALSE(alias);
        }
    } else {
        auto klass = environment.registry().find_class(name);
        auto defined_type = environment.registry().find_defined_type(name);
        if (expected) {
            REQUIRE((klass || defined_type));
        } else {
            REQUIRE_FALSE((klass || defined_type));
        }
    }
}

SCENARIO("environment with files to import", "[environment]")
{
    puppet::logging::console_logger logger;

    const string environment_name = "import";

    fs::path environments_dir = fs::path{FIXTURES_DIR} / "compiler" / "environments";
    fs::path environment_dir  = environments_dir / environment_name;

    compiler::settings settings;
    settings.set(settings::environment_path, environments_dir.string());
    settings.set(settings::environment, environment_name);

    auto environment = puppet::compiler::environment::create(logger, settings);

    // These attempt to parse invalid manifests which will raise an exception; the environment should not load manifests through import
    WHEN("importing using the environment namespace") {
        THEN("it should not raise an exception if requested to import a manifest") {
            import(logger, *environment, find_type::manifest, "environment", false);
            import(logger, *environment, find_type::manifest, "environment::foo", false);
        }
        THEN("it should import functions") {
            import(logger, *environment, find_type::function, "environment::foo");
            import(logger, *environment, find_type::function, "environment::bar::baz");
            import(logger, *environment, find_type::type, "environment::nope", false);
        }
        THEN("it should import types") {
            import(logger, *environment, find_type::type, "Environment::Foo");
            import(logger, *environment, find_type::type, "Environment::Bar::Baz");
            import(logger, *environment, find_type::type, "Environment::Nope", false);
        }
    }
    WHEN("importing using module 'bar'") {
        THEN("it should import classes or defined types") {
            import(logger, *environment, find_type::manifest, "bar");
            import(logger, *environment, find_type::manifest, "bar::baz");
            import(logger, *environment, find_type::manifest, "bar::baz::cake");
            import(logger, *environment, find_type::manifest, "bar::nope", false);
        }
        THEN("it should import functions") {
            import(logger, *environment, find_type::function, "bar::foo");
            import(logger, *environment, find_type::function, "bar::bar::baz");
            import(logger, *environment, find_type::function, "bar", false);
            import(logger, *environment, find_type::function, "bar::nope", false);
        }
        THEN("it should import types") {
            import(logger, *environment, find_type::type, "Bar::Baz");
            import(logger, *environment, find_type::type, "Bar::Jam::Cake");
            import(logger, *environment, find_type::type, "Bar", false);
            import(logger, *environment, find_type::type, "Bar::Nope", false);
        }
    }
    WHEN("importing using module 'foo'") {
        THEN("it should import classes or defined types") {
            import(logger, *environment, find_type::manifest, "foo::bar::baz");
            import(logger, *environment, find_type::manifest, "foo", false);
            import(logger, *environment, find_type::manifest, "foo::nope", false);
        }
        THEN("it should import functions") {
            import(logger, *environment, find_type::function, "foo::bar");
            import(logger, *environment, find_type::function, "foo::cake::is_a::lie");
            import(logger, *environment, find_type::function, "foo", false);
            import(logger, *environment, find_type::function, "foo::nope", false);
        }
        THEN("it should import types") {
            import(logger, *environment, find_type::type, "Foo::Bar");
            import(logger, *environment, find_type::type, "Foo::Baz::Wut");
            import(logger, *environment, find_type::type, "Foo", false);
            import(logger, *environment, find_type::type, "Foo::Nope", false);
        }
    }
}
