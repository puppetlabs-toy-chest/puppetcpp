#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor versioncmp::create_descriptor()
    {
        functions::descriptor descriptor{ "versioncmp" };

        descriptor.add("Callable[String, String]", [](call_context& context) {
            // Both arguments should be Strings
            auto& version_a = context.argument(0).require<string>();
            auto& version_b = context.argument(1).require<string>();

            // The regex used to split the version numbers into sub-sections
            static const std::regex version_regex("[-.]|\\d+|[^-.\\d]+");
            // The regex used to determine if all characters in a sub-section are numeric.
            static const std::regex digit_regex("^\\d+$");

            regex_iterator<string::const_iterator> iter_a(version_a.begin(), version_a.end(), version_regex);
            regex_iterator<string::const_iterator> iter_b(version_b.begin(), version_b.end(), version_regex);
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
            return static_cast<int64_t>(version_a.compare(version_b));
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
