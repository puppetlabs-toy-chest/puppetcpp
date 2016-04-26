#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <boost/algorithm/string.hpp>

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
            static const utility::regex version_regex(R"([-.]|\d+|[^-.\d])");
            // The regex used to determine if all characters in a sub-section are numeric.
            static const utility::regex digit_regex(R"(^\d+$)");

            utility::regex_iterator iter_a{ version_regex, version_a };
            utility::regex_iterator iter_b{ version_regex, version_b };
            utility::regex_iterator end;
            for (; iter_a != end && iter_b != end; ++iter_a, ++iter_b) {
                string a = string{ iter_a->begin(), iter_a->end() };
                string b = string{ iter_b->begin(), iter_b->end() };
                if (a == b) {
                    continue;
                }
                if (a == "-") {
                    return static_cast<int64_t>(-1);
                }
                if (b == "-") {
                    return static_cast<int64_t>(1);
                }
                if (a == ".") {
                    return static_cast<int64_t>(-1);
                }
                if (b == ".") {
                    return static_cast<int64_t>(1);
                }
                if (digit_regex.match(a) && digit_regex.match(b) && a[0] != '0' && b[0] != '0') {
                    auto result = stoi(a) - stoi(b);
                    if (result > 0) {
                        return static_cast<int64_t>(1);
                    }
                    if (result < 0) {
                        return static_cast<int64_t>(-1);
                    }
                    return static_cast<int64_t>(0);
                }
                return static_cast<int64_t>(boost::ilexicographical_compare(a, b) ? -1 : 1);
            }
            auto result = version_a.compare(version_b);
            if (result > 0) {
                return static_cast<int64_t>(1);
            }
            if (result < 0) {
                return static_cast<int64_t>(-1);
            }
            return static_cast<int64_t>(0);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
