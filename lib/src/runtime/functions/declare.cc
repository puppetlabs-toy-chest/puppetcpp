#include <puppet/runtime/functions/declare.hpp>
#include <puppet/runtime/definition_scanner.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct declare_visitor : boost::static_visitor<void>
    {
        declare_visitor(call_context& context, size_t index, boost::optional<runtime::relationship> const& relationship) :
            _context(context),
            _index(index),
            _relationship(relationship)
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
                boost::apply_visitor(*this, dereference(element));
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
                throw evaluator.create_exception(_context.position(_index), (boost::format("cannot %1% a class with an unspecified title.") % _context.name()).str());
            }

            auto& context = evaluator.evaluation_context();
            auto catalog = context.catalog();

            // Declare the class
            auto& resource = catalog->declare_class(context, type, evaluator.compilation_context(), _context.position(_index));

            // Add a relationship to the current resource
            if (_relationship) {
                auto* current = context.current_scope()->resource();
                if (!current) {
                    throw evaluator.create_exception(_context.position(_index), "the current scope has no associated resource.");
                }
                catalog->add_relationship(*_relationship, *current, resource);
            }
        }

        call_context& _context;
        size_t _index;
        boost::optional<runtime::relationship> const& _relationship;
    };

    declare::declare(boost::optional<runtime::relationship> relationship) :
        _relationship(rvalue_cast(relationship))
    {
    }

    value declare::operator()(call_context& context) const
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
            boost::apply_visitor(declare_visitor(context, i, _relationship), arguments[i]);
        }
        return value();
    }

}}}  // namespace puppet::runtime::functions
