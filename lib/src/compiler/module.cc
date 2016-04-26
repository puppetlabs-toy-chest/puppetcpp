#include <puppet/compiler/module.hpp>
#include <puppet/compiler/environment.hpp>
#include <puppet/cast.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace sys = boost::system;

using namespace std;

namespace puppet { namespace compiler {

    module::module(compiler::environment& environment, string directory, string name) :
        finder(rvalue_cast(directory)),
        _environment(environment),
        _name(rvalue_cast(name))
    {
    }

    compiler::environment const& module::environment() const
    {
        return _environment;
    }

    string const& module::name() const
    {
        return _name;
    }

    bool module::is_valid_name(string const& name)
    {
        static const utility::regex valid_name{R"(^[a-z][a-z0-9_]*$)"};
        return name != "environment" && valid_name.match(name);
    }

}}  // namespace puppet::compiler
