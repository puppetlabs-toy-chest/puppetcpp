#include <puppet/runtime/functions/include.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct include_visitor : boost::static_visitor<void>
    {
        include_visitor(call_context& context, size_t index) :
            _context(context),
            _index(index)
        {
        }

        result_type operator()(string const& argument) const
        {
            declare_class(types::klass(argument));
        }

        result_type operator()(values::type const& argument) const
        {
            boost::apply_visitor(*this, argument);
        }

        result_type operator()(values::array const& argument) const
        {
            for (auto const& element : argument) {
                boost::apply_visitor(*this, element);
            }
        }

        result_type operator()(types::klass const& argument) const
        {
            declare_class(argument);
        }

        result_type operator()(types::resource const& argument) const
        {
            if (argument.type_name() != "Class") {
                throw evaluation_exception(_context.position(_index), (boost::format("expected Class %1% for argument but found %2%.") % types::resource::name() % argument).str());
            }
            declare_class(types::klass(argument.title()));
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw evaluation_exception(_context.position(_index),
                (boost::format("expected %1%, %2%, %3%, or Class %4% for argument but found %5%.") %
                   types::string::name() %
                   types::array::name() %
                   types::klass::name() %
                   types::resource::name() %
                   get_type(argument)).str());
        }

     private:
        void declare_class(types::klass const& klass) const
        {
            auto& evaluator = _context.evaluator();
            if (klass.title().empty()) {
                throw evaluation_exception(_context.position(_index), "cannot include a Class with an unspecified title.");
            }

            auto catalog = evaluator.catalog();
            if (!catalog->is_class_defined(klass)) {
                throw evaluation_exception(_context.position(_index), (boost::format("cannot include class '%1%' because the class has not been defined.") % klass.title()).str());
            }

            if (!catalog->declare_class(evaluator.context(), klass, evaluator.path(), _context.position(_index))) {
                throw evaluation_exception(_context.position(_index), (boost::format("failed to include class '%1%'.") % klass.title()).str());
            }
        }

        call_context& _context;
        size_t _index;
    };

    value include::operator()(call_context& context) const
    {
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception(context.position(), (boost::format("expected at least one argument to '%1%' function.") % context.name()).str());
        }
        if (!context.evaluator().catalog()) {
            throw evaluation_exception(context.position(), (boost::format("cannot call '%1%' function: catalog functions are not supported.") % context.name()).str());
        }
        for (size_t i = 0; i < arguments.size(); ++i) {
            boost::apply_visitor(include_visitor(context, i), arguments[i]);
        }
        return value();
    }

}}}  // namespace puppet::runtime::functions
