#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/unicode/string.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value split_characters(string const& str)
    {
        values::array result;
        unicode::string string{ str };
        for (auto& grapheme : string) {
            result.emplace_back(std::string{ grapheme.begin(), grapheme.end() });
        }
        return result;
    }

    descriptor split::create_descriptor()
    {
        functions::descriptor descriptor{ "split" };

        descriptor.add("Callable[String, String]", [](call_context& context) -> values::value {
            auto& first = context.argument(0).require<string>();
            auto& second = context.argument(1).require<string>();
            if (second.empty()) {
                return split_characters(first);
            }

            values::array result;
            unicode::string string{ first };
            auto end = string.split_end();
            for (auto it = string.split_begin(second); it != end; ++it) {
                if (!*it) {
                    continue;
                }
                result.emplace_back(std::string{ it->begin(), it->end() });
            }
            return result;
        });
        descriptor.add("Callable[String, Regexp]", [](call_context& context) -> values::value {
            auto& first = context.argument(0).require<string>();
            auto& second = context.argument(1).require<values::regex>();

            if (second.pattern().empty()) {
                return split_characters(first);
            }

            values::array result;
            utility::regex_split_iterator end;
            for (utility::regex_split_iterator it{ second, first }; it != end; ++it) {
                result.emplace_back(string{ it->begin(), it->end() });
            }
            return result;
        });
        descriptor.add("Callable[String, Type[Regexp]]", [](call_context& context) -> values::value {
            auto& first = context.argument(0).require<string>();
            auto& second = boost::get<types::regexp>(context.argument(1).require<values::type>());

            if (second.pattern().empty()) {
                return split_characters(first);
            }

            utility::regex regex{ second.pattern() };

            values::array result;
            utility::regex_split_iterator end;
            for (utility::regex_split_iterator it{ regex, first }; it != end; ++it) {
                result.emplace_back(string{ it->begin(), it->end() });
            }
            return result;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
