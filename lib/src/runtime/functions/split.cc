#include <puppet/runtime/functions/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct split_visitor : boost::static_visitor<value>
    {
        explicit split_visitor(call_context const& context) :
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

        result_type operator()(string const& first, type const& second) const
        {
            throw evaluation_exception(_context.position(2), "Type argument not yet implemented.");
        }

        template <
            typename Second,
            typename = typename enable_if<!is_same<Second, string>::value>::type,
            typename = typename enable_if<!is_same<Second, values::regex>::value>::type,
            typename = typename enable_if<!is_same<Second, type>::value>::type
        >
        result_type operator()(string const&, Second const& second) const
        {
            throw evaluation_exception(_context.position(1), (boost::format("expected String, Regex, or Type[Regexp] for second argument but found %1%.") % get_type_name(second)).str());
        }

        template <typename First, typename Second>
        result_type operator()(First const& first, Second const&) const
        {
            throw evaluation_exception(_context.position(0), (boost::format("expected String for first argument but found %1%.") % get_type_name(first)).str());
        }

     private:
        static result_type split_empty(string const& str)
        {
            values::array result;
            result.reserve(str.size());
            for (auto c : str) {
                result.emplace_back(string(1, c));
            }
            return result;
        }

        call_context const& _context;
    };

    value split::operator()(call_context& context) const
    {
        auto& arguments = context.arguments();
        if (arguments.size() != 2) {
            throw evaluation_exception(arguments.size() > 2 ? context.position(2) : context.position(), (boost::format("expected 2 arguments to split function but %1% were given.") % arguments.size()).str());
        }
        return boost::apply_visitor(split_visitor(context), arguments[0], arguments[1]);
    }

}}}  // namespace puppet::runtime::functions
