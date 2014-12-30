#include "lexer.hpp"
#include <boost/spirit/include/lex_generate_static_lexertl.hpp>
#include <fstream>

using namespace std;
using namespace puppet::lexer;
using namespace boost::spirit;

int main(int argc, char* argv[])
{
    ofstream out("static_lexer.hpp");
    return lex::lexertl::generate_static_dfa(string_lexer(), out, "") ? 0 : -1;
}
