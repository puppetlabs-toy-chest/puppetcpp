#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct filter_visitor : boost::static_visitor<values::value>
    {
        explicit filter_visitor(function_call_context& context) :
            _context(context),
            _lambda_parameter_count(_context.lambda()->parameters.size())
        {
        }

        result_type operator()(string const& argument) const
        {
            values::array result;

            values::array arguments;
            arguments.reserve(2);

            // Enumerate the string as Unicode codepoints
            int64_t i = 0;
            values::enumerate_string(argument, [&](string codepoint) {
                arguments.clear();
                if (_lambda_parameter_count == 1) {
                    arguments.push_back(codepoint);
                } else {
                    arguments.push_back(i++);
                    arguments.push_back(codepoint);
                }
                if (_context.yield(arguments).is_true()) {
                    result.emplace_back(rvalue_cast(codepoint));
                }
                return true;
            });
            return result;
        }

        result_type operator()(int64_t argument) const
        {
            if (argument <= 0) {
                return values::array();
            }
            return enumerate(types::integer(0, argument));
        }

        result_type operator()(values::array const& argument) const
        {
            values::array result;

            values::array arguments;
            arguments.reserve(2);
            for (size_t i = 0; i < argument.size(); ++i) {
                arguments.clear();
                if (_lambda_parameter_count == 1) {
                    arguments.push_back(argument[i]);
                } else {
                    arguments.push_back(static_cast<int64_t>(i));
                    arguments.push_back(argument[i]);
                }
                if (_context.yield(arguments).is_true()) {
                    result.emplace_back(argument[i]);
                }
            }
            return result;
        }

        result_type operator()(values::hash const& argument) const
        {
            values::array arguments;
            arguments.reserve(2);

            values::hash result;
            for (auto const& kvp : argument) {
                arguments.clear();
                if (_lambda_parameter_count == 1) {
                    values::array pair(2);
                    pair[0] = kvp.first;
                    pair[1] = kvp.second;
                    arguments.emplace_back(rvalue_cast(pair));
                } else {
                    arguments.push_back(kvp.first);
                    arguments.push_back(kvp.second);
                }
                if (_context.yield(arguments).is_true()) {
                    result[kvp.first] = kvp.second;
                }
            }
            return result;
        }

        result_type operator()(values::type const& argument) const
        {
            return boost::apply_visitor(*this, argument);
        }

        result_type operator()(types::integer const& argument) const
        {
            if (!argument.enumerable()) {
                throw evaluation_exception((boost::format("%1% is not enumerable.") % argument).str(), _context.argument_context(0));
            }
            return enumerate(argument);
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw evaluation_exception((boost::format("expected enumerable type for first argument but found %1%.") % values::value(argument).get_type()).str(), _context.argument_context(0));
        }

     private:
        result_type enumerate(types::integer const& range) const
        {
            values::array result;

            values::array arguments;
            arguments.reserve(2);

            range.each([&](int64_t index, int64_t value) {
                arguments.clear();
                if (_lambda_parameter_count == 1) {
                    arguments.push_back(value);
                } else {
                    arguments.push_back(index);
                    arguments.push_back(value);
                }
                if (_context.yield(arguments).is_true()) {
                    result.emplace_back(value);
                }
                return true;
            });
            return result;
        }

        function_call_context& _context;
        size_t _lambda_parameter_count;
    };

    values::value filter::operator()(function_call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        if (arguments.size() != 1) {
            throw evaluation_exception(
                (boost::format("expected 1 argument to '%1%' function but %2% were given.") %
                    context.name() %
                    arguments.size()
                ).str(),
                arguments.size() > 1 ? context.argument_context(1) : context.call_site()
            );
        }

        // Check the lambda
        if (!context.lambda()) {
            throw evaluation_exception((boost::format("expected a lambda to '%1%' function but one was not given.") % context.name()).str(), context.call_site());
        }
        auto count = context.lambda()->parameters.size();
        if (count == 0 || count > 2) {
            throw evaluation_exception((boost::format("expected 1 or 2 lambda parameters but %1% were given.") % count).str(), context.lambda()->context);
        }
        return boost::apply_visitor(filter_visitor(context), arguments[0]);
    }

}}}}  // namespace puppet::compiler::evaluation::functions
