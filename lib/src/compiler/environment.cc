#include <puppet/compiler/environment.hpp>

using namespace std;

namespace puppet { namespace compiler {

    environment::environment(string name, string base) :
        _name(std::move(name)),
        _base(std::move(base))
    {
    }

    string const& environment::name() const
    {
        return _name;
    }

    string const& environment::base() const
    {
        return _base;
    }

}}  // namespace puppet::compiler
