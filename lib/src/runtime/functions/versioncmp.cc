#include <puppet/runtime/functions/versioncmp.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {
    // Faithfully translated from the original lib/puppet/util/package.rb
    value versioncmp::operator()(call_context& context) const
    {
        auto& evaluator = context.evaluator();

        // Check the argument count
        auto& arguments = context.arguments();
        if (arguments.size() != 2) {
            throw evaluator.create_exception(arguments.size() > 2 ? context.position(2) : context.position(), (boost::format("expected 2 arguments to '%1%' function but %2% were given.") % context.name() % arguments.size()).str());
        }

        // Both arguments should be Strings
        auto version_a = as<string>(arguments[0]);
        if (!version_a) {
            throw evaluator.create_exception(context.position(0), (boost::format("expected %1% for first argument but found %2%.") % types::string::name() % get_type(arguments[0])).str());
        }
        auto version_b = as<string>(arguments[1]);
        if (!version_b) {
            throw evaluator.create_exception(context.position(1), (boost::format("expected %1% for first argument but found %2%.") % types::string::name() % get_type(arguments[1])).str());
        }

        // The regex used to split the version numbers into sub-sections
        static const std::regex version_regex("[-.]|\\d+|[^-.\\d]+");
        // The regex used to determine if all characters in a sub-section are numeric.
        static const std::regex digit_regex("^\\d+$");

        regex_iterator<string::const_iterator> iter_a(version_a->begin(), version_a->end(), version_regex);
        regex_iterator<string::const_iterator> iter_b(version_b->begin(), version_b->end(), version_regex);
        regex_iterator<string::const_iterator> end;
        for (;iter_a != end && iter_b != end; ++iter_a, ++iter_b) {
            string a = iter_a->str();
            string b = iter_b->str();
            if (a == b) {
                continue;
            } else if (a == "-" && b == "-") {
                continue;
            } else if (a == "-") {
                return static_cast<int64_t>(-1);
            } else if (b == "-") {
                return static_cast<int64_t>(1);
            } else if (a == "." && b == ".") {
                continue;
            } else if (a == ".") {
                return static_cast<int64_t>(-1);
            } else if (b == ".") {
                return static_cast<int64_t>(1);
            } else if (regex_match(a, digit_regex) &&
                       regex_match(b, digit_regex) &&
                       a[0] != '0' &&
                       b[0] != '0') {
                return static_cast<int64_t>(stoi(a) - stoi(b));
            } else {
                return static_cast<int64_t>(boost::ilexicographical_compare(a, b) ? -1 : 1);
            }
        }
        return static_cast<int64_t>(version_a->compare(*version_b));
    }
}}}  // namespace puppet::runtime::functions
