#include <puppet/compiler/evaluation/operators/match.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct match_visitor : boost::static_visitor<value>
    {
        explicit match_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(string const& left, string const& right) const
        {
            try {
                smatch matches;
                bool result = right.empty() || regex_search(left, matches, std::regex(right));
                _context.context().set(matches);
                return result;
            } catch (regex_error const& ex) {
                throw evaluation_exception((boost::format("invalid regular expression: %1%") % ex.what()).str(), _context.right_context());
            }
        }

        result_type operator()(string const& left, values::regex const& right) const
        {
            smatch matches;
            bool result = right.pattern().empty() || regex_search(left, matches, right.value());
            _context.context().set(matches);
            return result;
        }

        template <typename Left>
        result_type operator()(Left const& left, type const& right) const
        {
            return right.is_instance(left);
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% or %2% for match but found %3%.") % types::string::name() % types::regexp::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Left,
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for match but found %2%.") % types::string::name() % value(left).get_type()).str(), _context.left_context());
        }

    private:
        binary_operator_context const& _context;
    };

    value match::operator()(binary_operator_context const& context) const
    {
        return boost::apply_visitor(match_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
