#include <puppet/ast/variable.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <regex>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    variable::variable()
    {
    }

    variable::variable(lexer::position position, string name) :
        _position(rvalue_cast(position)),
        _name(rvalue_cast(name))
    {
        validate_name();
    }

    string const& variable::name() const
    {
        return _name;
    }

    lexer::position const& variable::position() const
    {
        return _position;
    }

    void variable::validate_name() const
    {
        static const char* valid_variable_pattern = "0|[1-9]\\d*|((::)?[a-z]\\w*)*(::)?[a-z_]\\w*";
        static const std::regex valid_variable_regex(valid_variable_pattern);

        // Ensure the parameter name is valid
        if (!regex_match(_name, valid_variable_regex)) {
            throw puppet::compiler::parse_exception(_position, (boost::format("variable name '%1%' is not a valid variable name: the name must conform to /%2%/.") % _name % valid_variable_pattern).str());
        }
    }

    ostream& operator<<(ostream& os, variable const& var)
    {
        os << "$" << var.name();
        return os;
    }

}}  // namespace puppet::ast
