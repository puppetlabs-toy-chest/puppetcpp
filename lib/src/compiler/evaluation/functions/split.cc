#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct split_visitor : boost::static_visitor<values::value>
    {
        explicit split_visitor(function_call_context& context) :
            _context(context)
        {
        }

        result_type operator()(string const& first, string const& second) const
        {
            if (second.empty()) {
                return split_empty(first);
            }
            values::array result;
            boost::split_iterator<string::const_iterator> end;
            for (auto it = boost::make_split_iterator(first, boost::first_finder(second, boost::is_equal())); it != end; ++it) {
                if (!*it) {
                    continue;
                }
                result.emplace_back(boost::copy_range<string>(*it));
            }
            return result;
        }

        result_type operator()(string const& first, values::regex const& second) const
        {
            if (second.pattern().empty()) {
                return split_empty(first);
            }
            values::array result;
            for (sregex_token_iterator begin{ first.begin(), first.end(), second.value(), -1}, end; begin != end; ++begin) {
                result.emplace_back(string(*begin));
            }
            return result;
        }

        result_type operator()(string const& first, values::type const& second) const
        {
            auto regexp = boost::get<types::regexp>(&second);
            if (!regexp) {
                throw evaluation_exception((boost::format("expected %1% or %2% for second argument but found %3%.") % types::string::name() % types::regexp::name() % values::value(second).get_type()).str(), _context.argument_context(1));
            }
            if (regexp->pattern().empty()) {
                return split_empty(first);
            }
            values::array result;
            std::regex pattern(regexp->pattern());
            for (sregex_token_iterator begin{ first.begin(), first.end(), pattern, -1}, end; begin != end; ++begin) {
                result.emplace_back(string(*begin));
            }
            return result;
        }

        template <
            typename Second,
            typename = typename enable_if<!is_same<Second, string>::value>::type,
            typename = typename enable_if<!is_same<Second, values::regex>::value>::type,
            typename = typename enable_if<!is_same<Second, values::type>::value>::type
        >
        result_type operator()(string const&, Second const& second) const
        {
            throw evaluation_exception((boost::format("expected %1% or %2% for second argument but found %3%.") % types::string::name() % types::regexp::name() % values::value(second).get_type()).str(), _context.argument_context(1));
        }

        template <typename First, typename Second>
        result_type operator()(First const& first, Second const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for first argument but found %2%.") % types::string::name() % values::value(first).get_type()).str(), _context.argument_context(0));
        }

     private:
        static result_type split_empty(string const& str)
        {
            values::array result;
            values::enumerate_string(str, [&](string codepoint) {
                result.emplace_back(rvalue_cast(codepoint));
                return true;
            });
            return result;
        }

        function_call_context& _context;
    };

    values::value split::operator()(function_call_context& context) const
    {
        auto& arguments = context.arguments();
        auto count = arguments.size();
        if (count != 2) {
            throw evaluation_exception((boost::format("expected 2 arguments to '%1%' function but %2% were given.") % context.name() % count).str(), count > 2 ? context.argument_context(2) : context.name());
        }
        return boost::apply_visitor(split_visitor(context), arguments[0], arguments[1]);
    }

}}}}  // namespace puppet::compiler::evaluation::functions
