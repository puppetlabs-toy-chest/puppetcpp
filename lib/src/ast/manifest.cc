#include <puppet/ast/manifest.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    manifest::manifest()
    {
    }

    manifest::manifest(optional<vector<expression>> body, lexer::position end) :
        body(rvalue_cast(body)),
        end(rvalue_cast(end))
    {
    }

    ostream& operator<<(ostream& os, ast::manifest const& manifest)
    {
        pretty_print(os, manifest.body, "; ");
        return os;
    }

}}  // namespace puppet::ast
