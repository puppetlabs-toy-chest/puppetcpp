#include <puppet/parser/parser.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace boost::spirit;

namespace puppet { namespace parser {

    parse_exception::parse_exception(token_position position, string const& message) :
        runtime_error(message),
        _position(std::move(position))
    {
    }

    token_position const& parse_exception::position() const
    {
        return _position;
    }

    ast::manifest parser::parse(ifstream& input, bool interpolation)
    {
        file_static_lexer lexer;
        auto begin = lex_begin(input);
        auto end = lex_end(input);
        return parse(lexer, input, begin, end, interpolation);
    }

    ast::manifest parser::parse(string const& input, bool interpolation)
    {
        string_static_lexer lexer;
        auto begin = lex_begin(input);
        auto end = lex_end(input);
        return parse(lexer, input, begin, end, interpolation);
    }

    ast::manifest parser::parse(lexer_string_iterator& begin, lexer_string_iterator const& end, bool interpolation)
    {
        string_static_lexer lexer;
        auto range = boost::make_iterator_range(begin, end);
        return parse(lexer, range, begin, end, interpolation);
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

}}  // namespace puppet::parser