#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <regex>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    values::value versioncmp::operator()(function_call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        auto count = arguments.size();
        if (count != 2) {
            throw evaluation_exception((boost::format("expected 2 arguments to '%1%' function but %2% were given.") % context.name() % count).str(), count > 2 ? context.argument_context(2) : context.name());
        }

        // Both arguments should be Strings
        auto version_a = arguments[0]->as<string>();
        if (!version_a) {
            throw evaluation_exception((boost::format("expected %1% for first argument but found %2%.") % types::string::name() % arguments[0]->get_type()).str(), context.argument_context(0));
        }
        auto version_b = arguments[1]->as<string>();
        if (!version_b) {
            throw evaluation_exception((boost::format("expected %1% for second argument but found %2%.") % types::string::name() % arguments[1]->get_type()).str(), context.argument_context(1));
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
            } else if (regex_match(a, digit_regex) && regex_match(b, digit_regex) && a[0] != '0' && b[0] != '0') {
                return static_cast<int64_t>(stoi(a) - stoi(b));
            } else {
                return static_cast<int64_t>(boost::ilexicographical_compare(a, b) ? -1 : 1);
            }
        }
        return static_cast<int64_t>(version_a->compare(*version_b));
    }

}}}}  // namespace puppet::compiler::evaluation::functions
