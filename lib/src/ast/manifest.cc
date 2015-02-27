#include <puppet/ast/manifest.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    manifest::manifest()
    {
    }

    manifest::manifest(optional<vector<expression>> body) :
        _body(std::move(body))
    {
    }

    optional<vector<expression>> const& manifest::body() const
    {
        return _body;
    }

    optional<vector<expression>>& manifest::body()
    {
        return _body;
    }

    ostream& operator<<(ostream& os, ast::manifest const& manifest)
    {
        pretty_print(os, manifest.body(), "; ");
        return os;
    }

}}  // namespace puppet::ast
