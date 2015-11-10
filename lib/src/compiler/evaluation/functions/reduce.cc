#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct reduce_visitor : boost::static_visitor<values::value>
    {
        reduce_visitor(function_call_context& context, boost::optional<values::value> memo) :
            _context(context),
            _memo(rvalue_cast(memo))
        {
        }

        result_type operator()(string const& argument)
        {
            values::array arguments;
            arguments.reserve(2);

            // Enumerate the string as Unicode code points
            values::enumerate_string(argument, [&](string codepoint) {
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
            return values::undef();
        }

        result_type operator()(int64_t argument)
        {
            if (argument <= 0) {
                return values::array();
            }
            return enumerate(types::integer(0, argument));
        }

        result_type operator()(values::array const& argument)
        {
            values::array arguments;
            arguments.reserve(2);
            for (size_t i = 0; i < argument.size(); ++i) {
                // If no memo, set it now
                if (!_memo) {
                    _memo = argument[i];
                    continue;
                }

                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(argument[i]);
                _memo = _context.yield(arguments);
            }
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return values::undef();
        }

        result_type operator()(values::hash const& argument)
        {
            values::array arguments;
            arguments.reserve(2);
            for (auto& kvp : argument) {
                values::array pair(2);
                pair[0] = kvp.key();
                pair[1] = kvp.value();

                if (!_memo) {
                    _memo = rvalue_cast(pair);
                    continue;
                }

                arguments.clear();
                arguments.emplace_back(rvalue_cast(*_memo));
                arguments.emplace_back(rvalue_cast(pair));
                _memo = _context.yield(arguments);
            }
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return values::undef();
        }

        result_type operator()(values::type const& argument)
        {
            return boost::apply_visitor(*this, argument);
        }

        result_type operator()(types::integer const& argument)
        {
            if (!argument.enumerable()) {
                throw evaluation_exception((boost::format("%1% is not enumerable.") % argument).str(), _context.argument_context(0));
            }
            return enumerate(argument);
        }

        template <typename T>
        result_type operator()(T const& argument)
        {
            throw evaluation_exception((boost::format("expected enumerable type for first argument but found %1%.") % values::value(argument).get_type()).str(), _context.argument_context(0));
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
                _memo = _context.yield(arguments);
                return true;
            });
            if (_memo) {
                return rvalue_cast(*_memo);
            }
            return values::undef();
        }

        function_call_context& _context;
        boost::optional<values::value> _memo;
    };

    values::value reduce::operator()(function_call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        auto count = arguments.size();
        if (arguments.size() == 0 || arguments.size() > 2) {
            throw evaluation_exception((boost::format("expected 1 or 2 arguments to '%1%' function but %2% were given.") % context.name() % count).str(), count > 2 ? context.argument_context(2) : context.call_site());
        }

        // Check the lambda
        if (!context.lambda()) {
            throw evaluation_exception((boost::format("expected a lambda to '%1%' function but one was not given.") % context.name()).str(), context.call_site());
        }
        count = context.lambda()->parameters.size();
        if (count != 2) {
            throw evaluation_exception((boost::format("expected 2 lambda parameters but %1% were given.") % count).str(), context.lambda()->context);
        }

        // Use the provided memo if there is one
        boost::optional<values::value> memo;
        if (arguments.size() == 2) {
            memo.emplace(rvalue_cast(arguments[1]));
        }

        reduce_visitor visitor{context, rvalue_cast(memo)};
        return boost::apply_visitor(visitor, arguments[0]);
    }

}}}}  // namespace puppet::compiler::evaluation::functions
