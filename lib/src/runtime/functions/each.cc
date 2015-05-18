#include <puppet/runtime/functions/each.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct each_visitor : boost::static_visitor<void>
    {
        explicit each_visitor(call_context& context) :
            _context(context)
        {
        }

        result_type operator()(string const& argument) const
        {
            values::array arguments;
            arguments.reserve(2);
            for (size_t i = 0; i < argument.size(); ++i) {
                arguments.clear();
                if (_context.yielder().parameter_count() == 1) {
                    arguments.push_back(string(1, argument[i]));
                } else {
                    arguments.push_back(static_cast<int64_t>(i));
                    arguments.push_back(string(1, argument[i]));
                }
                _context.yielder().yield(arguments);
            }
        }

        result_type operator()(int64_t argument) const
        {
            if (argument > 0) {
                enumerate(types::integer(0, argument));
            }
        }

        result_type operator()(values::array const& argument) const
        {
            values::array arguments;
            arguments.reserve(2);
            for (size_t i = 0; i < argument.size(); ++i) {
                arguments.clear();
                if (_context.yielder().parameter_count() == 1) {
                    arguments.push_back(argument[i]);
                } else {
                    arguments.push_back(static_cast<int64_t>(i));
                    arguments.push_back(argument[i]);
                }
                _context.yielder().yield(arguments);
            }
        }

        result_type operator()(values::hash const& argument) const
        {
            values::array arguments;
            arguments.reserve(2);
            for (auto const& kvp : argument) {
                arguments.clear();
                if (_context.yielder().parameter_count() == 1) {
                    arguments.emplace_back(values::array { kvp.first, kvp.second });
                } else {
                    arguments.push_back(kvp.first);
                    arguments.push_back(kvp.second);
                }
                _context.yielder().yield(arguments);
            }
        }

        result_type operator()(type const& argument) const
        {
            return boost::apply_visitor(*this, argument);
        }

        result_type operator()(types::integer const& argument) const
        {
            if (!argument.enumerable()) {
                throw evaluation_exception(_context.position(0), (boost::format("%1% is not enumerable.") % argument).str());
            }
            enumerate(argument);
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw evaluation_exception(_context.position(0), (boost::format("expected enumerable type for first argument but found %1%.") % get_type(argument)).str());
        }

     private:
        void enumerate(types::integer const& range) const
        {
            values::array arguments;
            arguments.reserve(2);
            range.each([&](int64_t index, int64_t value) {
                arguments.clear();
                if (_context.yielder().parameter_count() == 1) {
                    arguments.push_back(value);
                } else {
                    arguments.push_back(index);
                    arguments.push_back(value);
                }
                _context.yielder().yield(arguments);
                return true;
            });
        }

        call_context& _context;
    };

    value each::operator()(call_context& context) const
    {
        auto& yielder = context.yielder();

        // Check the argument count
        auto& arguments = context.arguments();
        if (arguments.size() != 1) {
            throw evaluation_exception(arguments.size() > 1 ? context.position(1) : context.position(), (boost::format("expected 1 argument to 'each' function but %1% were given.") % arguments.size()).str());
        }

        // Check the lambda
        if (!yielder.lambda_given()) {
            throw evaluation_exception(context.position(), "expected a lambda to 'each' function but one was not given.");
        }
        auto count = yielder.parameter_count();
        if (count == 0 || count > 2) {
            throw evaluation_exception(yielder.position(), (boost::format("expected 1 or 2 lambda parameters but %1% were given.") % count).str());
        }

        // Visit the argument and return it
        boost::apply_visitor(each_visitor(context), dereference(arguments[0]));
        return std::move(arguments[0]);
    }

}}}  // namespace puppet::runtime::functions
