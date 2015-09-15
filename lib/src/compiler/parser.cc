#include <puppet/compiler/parser.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace boost::spirit;

namespace puppet { namespace compiler {

    ast::syntax_tree parser::parse(ifstream& input)
    {
        file_static_lexer lexer;
        compiler::grammar<file_static_lexer> grammar{lexer};

        auto begin = lex_begin(input);
        auto end = lex_end(input);
        return parse(lexer, grammar, input, begin, end);
    }

    ast::syntax_tree parser::parse(string const& input)
    {
        string_static_lexer lexer;
        compiler::grammar<string_static_lexer> grammar{lexer};

        auto begin = lex_begin(input);
        auto end = lex_end(input);
        return parse(lexer, grammar, input, begin, end);
    }

    ast::syntax_tree parser::parse(lexer_string_iterator& begin, lexer_string_iterator const& end)
    {
        string_static_lexer lexer;
        compiler::grammar<string_static_lexer> grammar{lexer, true};

        auto range = boost::make_iterator_range(begin, end);
        return parse(lexer, grammar, range, begin, end);
    }

    parser::expectation_info_printer::expectation_info_printer(ostream& os) :
        _os(os),
        _next(false)
    {
    }

    void parser::expectation_info_printer::element(utf8_string const& tag, utf8_string const& value, int depth)
    {
        if (!_depths.empty()) {
            if (depth > _depths.top()) {
                if (!_next) {
                    return;
                }
            } else if (depth == _depths.top()) {
                _depths.pop();
            }
        }
        _next = false;

        if (tag == "eoi") {
            _os << "end of input";
        } else if (tag == "list") {
            _os << "list of ";
            _depths.push(depth);
            _next = true;
        } else if (tag == "expect") {
            _os << "at least one ";
            _depths.push(depth);
            _next = true;
        } else if (tag == "token" || tag == "raw_token") {
            _os << value;
        } else {
            if (!tag.empty()) {
                _os << tag;
                if (!value.empty()) {
                    _os << ' ';
                }
            }
            _os << value;
        }
    }

}}  // namespace puppet::compiler