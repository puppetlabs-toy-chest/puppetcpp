#include <puppet/parser/parser.hpp>

using namespace std;
using namespace boost::spirit;

namespace puppet { namespace parser {

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