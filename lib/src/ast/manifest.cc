#include <puppet/ast/manifest.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    manifest::manifest()
    {
    }

    manifest::manifest(optional<vector<expression>> body, lexer::token_position end) :
        _body(std::move(body)),
        _end(std::move(end))
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

    lexer::token_position const& manifest::end() const
    {
        return _end;
    }

    ostream& operator<<(ostream& os, ast::manifest const& manifest)
    {
        pretty_print(os, manifest.body(), "; ");
        return os;
    }

}}  // namespace puppet::ast
