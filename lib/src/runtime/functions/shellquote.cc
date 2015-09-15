#include <puppet/runtime/functions/shellquote.hpp>


using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {
    // TODO: Handle unicode.

    // Faithfully translated from the original lib/puppet/parser/functions/shellquote.rb
    /** Quote and concatenate arguments for use in Bourne shell.

     Each argument is quoted separately, and then all are concatenated
     with spaces.  If an argument is an array, each element of that
     array is interpolated within the rest of the arguments; this makes
     it possible to have an array of arguments and pass that array to
     shellquote instead of having to specify each argument
     individually in the call.
    */
    value shellquote::operator()(call_context& context) const
    {
        auto& evaluator = context.evaluator();
        auto& arguments = context.arguments();
        return _quotify(arguments, evaluator, context);
    }

    string shellquote::_quotify(const values::array& arguments, expression_evaluator& evaluator, call_context& context) const
    {
        static const string dangerous =  "!\"`$\\";

        string result;
        for (size_t i = 0; i < arguments.size(); ++i) {
            // Each argument may be a string or an array. If it is an array
            // (or worse, an array of arrays) then recursively quotify it.

            // First assume it's a string.
            auto word = as<string>(arguments[i]);
            if (!word) {
                // Okay, it's not a string, let's try an array.
                auto sub_array = as<values::array>(arguments[i]);
                if (!sub_array) {
                    // Crap, let's give up.
                    throw evaluator.create_exception(context.position(i), (boost::format("expected %1% or %2% for first argument but found %3%.") % types::string::name() % types::array::name() % get_type(arguments[i])).str());
                }
                // Okay quotify it and add it to the result.
                result += " " + _quotify(*sub_array, evaluator, context);
                continue;
            }
            if (word->find_first_of(dangerous) == string::npos) {
                result += " " + (boost::format("\"%1%\"") % *word).str();
            } else if (word->find_first_of("'") == string::npos) {
                result += " " + (boost::format("'%1%'") % *word).str();
            } else {
                result += "\"";
                for (auto &c : *word) {
                    if (dangerous.find(c) != string::npos) {
                        result += "\\";
                    }
                    result += c;
                }
                result += "\"";
            }
        }
        return result;
    }
}}}  // namespace puppet::runtime::functions
