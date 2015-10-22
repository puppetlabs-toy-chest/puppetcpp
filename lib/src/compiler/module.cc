#include <puppet/compiler/module.hpp>
#include <puppet/compiler/environment.hpp>
#include <puppet/cast.hpp>
#include <boost/filesystem.hpp>
#include <regex>

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
        static const regex valid_name{"^[a-z][a-z0-9_]*$"};
        static const vector<string> invalid_names = {
            "and",
            "attr",
            "case",
            "class",
            "default",
            "define",
            "else",
            "elsif",
            "false",
            "function",
            "if",
            "in",
            "inherits",
            "node",
            "or",
            "private",
            "true",
            "type",
            "undef",
            "unless"
        };

        if (!regex_match(name, valid_name)) {
            return false;
        }

        if (std::find(invalid_names.begin(), invalid_names.end(), name) != invalid_names.end()) {
            return false;
        }
        return true;
    }

}}  // namespace puppet::compiler
