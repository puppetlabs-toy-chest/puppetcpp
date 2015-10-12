#include <puppet/runtime/functions/reduce.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct reduce_visitor : boost::static_visitor<value>
    {
        explicit reduce_visitor(call_context& context, boost::optional<values::value> memo) :
            _context(context),
            _memo(rvalue_cast(memo))
        {
        }

        result_type operator()(string const& argument)
        {
            values::array arguments;
            arguments.reserve(2);

            // Enumerate the string as Unicode code points
            enumerate_string(argument, [&](string codepoint) {
                // If no memo, set it now
                if (!_memo) {
                    _memo.emplace(rvalue_cast(codepoint));
                    return true;
                }

                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(rvalue_cast(codepoint));
                _memo.emplace(_context.yield(arguments));
                return true;
            });
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return undef();
        }

        result_type operator()(int64_t argument)
        {
            if (argument <= 0) {
                return values::array();
            }
            return enumerate(types::integer(0, argument));
        }

        result_type operator()(values::array& argument)
        {
            values::array arguments;
            arguments.reserve(2);
            for (size_t i = 0; i < argument.size(); ++i) {
                // If no memo, set it now
                if (!_memo) {
                    _memo.emplace(argument[i]);
                    continue;
                }

                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(rvalue_cast(argument[i]));
                _memo.emplace(_context.yield(arguments));
            }
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return undef();
        }

        result_type operator()(values::hash& argument)
        {
            values::array arguments;
            arguments.reserve(2);
            for (auto& kvp : argument) {
                values::array pair(2);
                pair[0] = kvp.first;
                pair[1] = rvalue_cast(kvp.second);

                if (!_memo) {
                    _memo.emplace(rvalue_cast(pair));
                    continue;
                }

                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(rvalue_cast(pair));
                _memo.emplace(_context.yield(arguments));
            }
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return undef();
        }

        result_type operator()(type const& argument)
        {
            return boost::apply_visitor(*this, argument);
        }

        result_type operator()(types::integer const& argument)
        {
            if (!argument.enumerable()) {
                throw _context.evaluator().create_exception(_context.position(0), (boost::format("%1% is not enumerable.") % argument).str());
            }
            return enumerate(argument);
        }

        template <typename T>
        result_type operator()(T const& argument)
        {
            throw _context.evaluator().create_exception(_context.position(0), (boost::format("expected enumerable type for first argument but found %1%.") % get_type(argument)).str());
        }

     private:
        result_type enumerate(types::integer const& range)
        {
            values::array arguments;
            arguments.reserve(2);

            range.each([&](int64_t index, int64_t value) {
                if (!_memo) {
                    _memo = value;
                    return true;
                }
                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(value);
                _memo.emplace(_context.yield(arguments));
                return true;
            });
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return undef();
        }

        call_context& _context;
        boost::optional<values::value> _memo;
    };

    value reduce::operator()(call_context& context) const
    {
        auto& evaluator = context.evaluator();

        // Check the argument count
        auto& arguments = context.arguments();
        if (arguments.size() == 0 || arguments.size() > 2) {
            throw evaluator.create_exception(arguments.size() > 2 ? context.position(2) : context.position(), (boost::format("expected 1 or 2 arguments to '%1%' function but %2% were given.") % context.name() % arguments.size()).str());
        }

        // Check the lambda
        if (!context.lambda_given()) {
            throw evaluator.create_exception(context.position(), (boost::format("expected a lambda to '%1%' function but one was not given.") % context.name()).str());
        }
        auto count = context.lambda_parameter_count();
        if (count != 2) {
            throw evaluator.create_exception(context.lambda_position(), (boost::format("expected 2 lambda parameters but %1% were given.") % count).str());
        }

        // Use the provided memo if there is one
        boost::optional<value> memo;
        if (arguments.size() == 2) {
            memo.emplace(rvalue_cast(arguments[1]));
        }

        auto argument = mutate(arguments[0]);
        reduce_visitor visitor{context, rvalue_cast(memo)};
        return boost::apply_visitor(visitor, argument);
    }

}}}  // namespace puppet::runtime::functions
