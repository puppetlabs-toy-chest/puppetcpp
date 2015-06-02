#include <puppet/ast/syntax_tree.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    syntax_tree::syntax_tree(std::string path, optional<vector<expression>> body, lexer::position end) :
        _path(rvalue_cast(path)),
        _body(rvalue_cast(body)),
        _end(rvalue_cast(end))
    {
    }

    std::string const& syntax_tree::path() const
    {
        return _path;
    }

    optional<vector<expression>> const& syntax_tree::body() const
    {
        return _body;
    }

    lexer::position const& syntax_tree::end() const
    {
        return _end;
    }

    ostream& operator<<(ostream& os, ast::syntax_tree const& tree)
    {
        pretty_print(os, tree.body(), "; ");
        return os;
    }

}}  // namespace puppet::ast
