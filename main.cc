// The static lexer header must be included prior to including the lexer header
#include "lexer/static_lexer.hpp"
#include "lexer/lexer.hpp"
#include "parser/grammar.hpp"
#include "ast/manifest.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::parser;
namespace ast = puppet::ast;
using namespace boost::spirit;

void error(ifstream& file, string const& filename, token_position const& position, string const& message = std::string())
{
    string line;
    size_t column;

    tie(line, column) = get_line_and_column(file, std::get<0>(position));

    cerr << filename << ":" << std::get<1>(position) << ":" << column << ": error: ";
    if (!message.empty()) {
        cerr << message << "\n";
    } else {
        auto current = line[column - 1];
        if (current == '\'' || current == '\"') {
            cerr << "unclosed quote";
        } else {
            cerr << "unexpected character ";
            if (isprint(current)) {
                cerr << '\'' << current << '\'';
            } else {
                cerr << "0x" << setw(2) << setfill('0') << static_cast<int>(current);
            }
        }
        cerr << ".\n";
    }

    if (line.empty()) {
        return;
    }

    cerr << "    " << line << '\n';
    cerr << setfill(' ') << setw(column + 5) << "^\n";
}

struct expectation_info_printer
{
    expectation_info_printer(ostream& os) :
        _os(os),
        _next(false)
    {
    }

    void element(utf8_string const& tag, utf8_string const& value, int depth)
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

private:
    ostream& _os;
    bool _next;
    std::stack<int> _depths;
};

template <typename Iterator>
void expectation_error(ifstream& file, string const& filename, qi::expectation_failure<Iterator> const& ex)
{
    // Expectation failure
    ostringstream ss;
    ss << "expected ";

    expectation_info_printer printer(ss);
    boost::apply_visitor(basic_info_walker<expectation_info_printer>(printer, ex.what_.tag, 0), ex.what_.value);

    ss << " but found " << static_cast<token_id>(ex.first->id()) << ".";

    error(file, filename, get_position(file, *ex.first), ss.str());
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "usage: puppetcpp <source_file>" << endl;
        return EXIT_FAILURE;
    }

    ifstream in(argv[1]);
    if (!in) {
        cerr << "error: could not open source file \"" << argv[1] << "\"." << endl;
        return EXIT_FAILURE;
    }

    auto input_begin = lex_begin(in);
    auto input_end = lex_end(in);

    // Define the lexer type to use so we can easily change between static and dynamic lexers
    typedef file_static_lexer lexer_type;

    try {
        lexer_type lexer;
        auto token_begin = lexer.begin(input_begin, input_end);
        auto token_end = lexer.end();

        // Parse the input into a manifest
        ast::manifest manifest;
        if (boost::spirit::qi::parse(token_begin, token_end, grammar<typename lexer_type::iterator_type>(lexer), manifest) && (token_begin == token_end || token_begin->id() == boost::lexer::npos)) {
            cout << "file compiled successfully." << "\n";
            cout << "manifest:\n" << manifest << endl;
            return EXIT_SUCCESS;
        }

        // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
        if (token_begin != token_end && token_is_valid(*token_begin)) {
            ostringstream ss;
            ss << "unexpected " << static_cast<token_id>(token_begin->id()) << ".";
            error(in, argv[1], get_position(in, *token_begin), ss.str());
        } else {
            // Otherwise, the failure is in the input stream
            error(in, argv[1], input_begin.position());
        }
        return EXIT_FAILURE;
    } catch (lexer_exception<decltype(input_begin)>& ex) {
        // Handle lexing failure
        error(in, argv[1], ex.location().position(), ex.what());
        return EXIT_FAILURE;
    } catch (qi::expectation_failure<lexer_type::iterator_type> const& ex) {
        // Handle grammar expectation failure
        expectation_error(in, argv[1], ex);
        return EXIT_FAILURE;
    }
}
