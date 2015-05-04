#include <puppet/runtime/operators/match.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct match_visitor : boost::static_visitor<value>
    {
        match_visitor(token_position const& left_position, token_position const& right_position, context& ctx) :
            _left_position(left_position),
            _right_position(right_position),
            _context(ctx)
        {
        }

        result_type operator()(string const& left, string const& right)
        {
            smatch matches;
            bool result = right.empty() || regex_search(left, matches, std::regex(right));
            _context.current().set(matches);
            return result;
        }

        result_type operator()(string const& left, values::regex const& right)
        {
            smatch matches;
            bool result = right.pattern().empty() || regex_search(left, matches, right.value());
            _context.current().set(matches);
            return result;
        }

        template <typename Left>
        result_type operator()(Left const& left, type const& right)
        {
            return is_instance(left, right);
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(string const&, Right const& right)
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% or %2% for match but found %3%.") % types::string::name() % types::regexp::name() % get_type(right)).str());
        }

        template <
            typename Left,
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(Left const& left, Right const&)
        {
            throw evaluation_exception(_left_position, (boost::format("expected %1% for match but found %2%.") % types::string::name() % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
        context& _context;
    };

    value match::operator()(binary_context& context) const
    {
        match_visitor visitor(context.left_position(), context.right_position(), context.evaluation_context());
        return boost::apply_visitor(visitor, dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
