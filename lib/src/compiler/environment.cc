#include <puppet/compiler/environment.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler {

    environment::environment(string name, string base) :
        _name(rvalue_cast(name)),
        _base(rvalue_cast(base))
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
