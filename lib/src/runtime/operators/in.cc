#include <puppet/runtime/operators/in.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct in_visitor : boost::static_visitor<bool>
    {
        explicit in_visitor(context& ctx) :
            _context(ctx)
        {
        }

        result_type operator()(string const& left, string const& right)
        {
            return boost::algorithm::icontains(right, left);
        }

        result_type operator()(values::regex const& left, string const& right)
        {
            smatch matches;
            bool result = left.pattern().empty() || regex_search(right, matches, left.value());
            _context.current().set(matches);
            return result;
        }

        result_type operator()(type const& left, values::array const& right)
        {
            for (auto const& element : right) {
                if (is_instance(element, left)) {
                    return true;
                }
            }
            return false;
        }

        result_type operator()(values::regex const& left, values::array const& right)
        {
            for (auto const& element : right) {
                auto ptr = boost::get<string>(&element);
                if (ptr && operator()(left, *ptr)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left>
        result_type operator()(Left const& left, values::array const& right)
        {
            for (auto const& element : right) {
                if (equals(left, element)) {
                    return true;
                }
            }
            return false;
        }

        result_type operator()(type const& left, values::hash const& right)
        {
            for (auto const& element : right) {
                if (is_instance(element.first, left)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left>
        result_type operator()(Left const& left, values::hash const& right)
        {
            for (auto const& element : right) {
                if (equals(left, element.first)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left, typename Right>
        result_type operator()(Left const&, Right const& right)
        {
            return false;
        }

    private:
        context& _context;
    };

    value in::operator()(binary_context& context) const
    {
        in_visitor visitor(context.evaluation_context());
        return boost::apply_visitor(visitor, dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
