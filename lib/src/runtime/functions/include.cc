#include <puppet/runtime/functions/include.hpp>
#include <puppet/runtime/definition_scanner.hpp>

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
            if (!argument.is_class()) {
                throw _context.evaluator().create_exception(_context.position(_index), (boost::format("expected Class %1% for argument but found %2%.") % types::resource::name() % argument).str());
            }
            declare_class(types::klass(argument.title()));
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw _context.evaluator().create_exception(_context.position(_index),
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
            types::resource type("class", klass.title());

            auto& evaluator = _context.evaluator();
            if (!type.fully_qualified()) {
                throw evaluator.create_exception(_context.position(_index), "cannot include a class with an unspecified title.");
            }

            // Check to see if the class already exists in the catalog; if so, do nothing
            auto& context = evaluator.evaluation_context();
            auto catalog = context.catalog();
            if (catalog->find_resource(type)) {
                return;
            }

            // Declare the class
            catalog->declare_class(context, type, evaluator.compilation_context(), _context.position(_index));
        }

        call_context& _context;
        size_t _index;
    };

    value include::operator()(call_context& context) const
    {
        auto& evaluator = context.evaluator();
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluator.create_exception(context.position(), (boost::format("expected at least one argument to '%1%' function.") % context.name()).str());
        }
        if (!evaluator.evaluation_context().catalog()) {
            throw evaluator.create_exception(context.position(), (boost::format("cannot call '%1%' function: catalog functions are not supported.") % context.name()).str());
        }
        for (size_t i = 0; i < arguments.size(); ++i) {
            boost::apply_visitor(include_visitor(context, i), arguments[i]);
        }
        return value();
    }

}}}  // namespace puppet::runtime::functions
