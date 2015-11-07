#include <puppet/compiler/evaluation/operators/in.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct in_visitor : boost::static_visitor<bool>
    {
        explicit in_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(string const& left, string const& right) const
        {
            return boost::algorithm::icontains(right, left);
        }

        result_type operator()(values::regex const& left, string const& right) const
        {
            smatch matches;
            bool result = left.pattern().empty() || regex_search(right, matches, left.value());
            _context.context().set(matches);
            return result;
        }

        result_type operator()(type const& left, values::array const& right) const
        {
            for (auto const& element : right) {
                if (left.is_instance(element)) {
                    return true;
                }
            }
            return false;
        }

        result_type operator()(values::regex const& left, values::array const& right) const
        {
            for (auto const& element : right) {
                auto ptr = element->as<std::string>();
                if (ptr && operator()(left, *ptr)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left>
        result_type operator()(Left const& left, values::array const& right) const
        {
            for (auto const& element : right) {
                if (left == element) {
                    return true;
                }
            }
            return false;
        }

        result_type operator()(type const& left, values::hash const& right) const
        {
            for (auto const& element : right) {
                if (left.is_instance(element.first)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left>
        result_type operator()(Left const& left, values::hash const& right) const
        {
            for (auto const& element : right) {
                if (left == element.first) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left, typename Right>
        result_type operator()(Left const&, Right const& right) const
        {
            return false;
        }

     private:
        binary_operator_context const& _context;
    };

    value in::operator()(binary_operator_context const& context) const
    {
        return boost::apply_visitor(in_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
