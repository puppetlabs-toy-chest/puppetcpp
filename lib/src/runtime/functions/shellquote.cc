#include <puppet/runtime/functions/shellquote.hpp>


using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {
    // FIXME: How should this handle unicode? The original ruby only handles
    // the printable ascii range.

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
        // EWWWW!!
        const string safe = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0-9@%_+=:,./-";
        const string dangerous =  "!\"`$\\";
        auto& evaluator = context.evaluator();
        vector<const string> result;

        auto& arguments = context.arguments();
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto word = as<string>(arguments[i]);
            if (!word) {
                throw evaluator.create_exception(context.position(i), (boost::format("expected %1% for first argument but found %2%.") % types::string::name() % get_type(arguments[i])).str());
            }
            if (word->length() != 0 && _count_chars(*word, safe) == word->length()) {
                result.push_back(*word);
            } else if (_count_chars(*word, dangerous) == 0) {
                result.push_back((boost::format("\"%1%\"") % *word).str());
            } else if (_count_chars(*word, "'") == 0) {
                result.push_back((boost::format("'%1%'") % *word).str());
            } else {
                string r = "\"";
                for (auto &c : *word) {
                    if (dangerous.find(c) != string::npos) {
                        r += "\\";
                    }
                    r += c;
                }
                r += "\"";
                result.push_back(r);
            }
        }
        // There's probably a better way to do this too.
        string ret;
        for (auto &str : result) {
            ret += str;
            ret += " ";
        }
        return ret;
    }

    // O(N*M) :( Is there some sort of cheap hash function which
    // Also, there's probably functional C++11 wizardry which could make this
    // a 1 liner.
    size_t shellquote::_count_chars(const string &word, const string &set) const {
        int count = 0;
        for (auto const &chr : word) {
            for (auto const &elem : set) {
                if (chr == elem) {
                    count++;
                }
            }
        }
        return count;
    }

}}}  // namespace puppet::runtime::functions
