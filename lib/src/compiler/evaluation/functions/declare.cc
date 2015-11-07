#include <puppet/compiler/evaluation/functions/declare.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct declare_visitor : boost::static_visitor<void>
    {
        declare_visitor(function_call_context& context, ast::context const& argument_context, boost::optional<compiler::relationship> const& relationship) :
            _context(context),
            _argument_context(argument_context),
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
                throw evaluation_exception((boost::format("expected %1% for argument but found %2%.") % types::resource("class") % values::value(argument).get_type()).str(), _argument_context);
            }
            declare_class(types::klass(argument.title()));
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw evaluation_exception(
                (boost::format("expected %1%, %2%, %3%, or %4% for argument but found %5%.") %
                 types::string::name() %
                 types::array::name() %
                 types::klass::name() %
                 types::resource("class") %
                 values::value(argument).get_type()
                ).str(),
                _argument_context);
        }

     private:
        void declare_class(types::klass const& klass) const
        {
            types::resource type("class", klass.title());

            if (!type.fully_qualified()) {
                throw evaluation_exception((boost::format("cannot %1% a class with an unspecified title.") % _context.name()).str(), _argument_context);
            }

            auto& context = _context.context();

            // Declare the class
            auto resource = context.declare_class(klass.title(), _argument_context);

            // Add a relationship to the current resource
            if (_relationship) {
                auto* current = context.current_scope()->resource();
                if (!current) {
                    throw evaluation_exception("the current scope has no associated resource.", _argument_context);
                }
                context.catalog().relate(*_relationship, *current, *resource);
            }
        }

        function_call_context& _context;
        ast::context const& _argument_context;
        boost::optional<compiler::relationship> const& _relationship;
    };

    declare::declare(boost::optional<compiler::relationship> relationship) :
        _relationship(relationship)
    {
    }

    values::value declare::operator()(function_call_context& context) const
    {
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception((boost::format("expected at least one argument to '%1%' function.") % context.name()).str(), context.call_site());
        }
        for (size_t i = 0; i < arguments.size(); ++i) {
            boost::apply_visitor(declare_visitor(context, context.argument_context(i), _relationship), arguments[i]);
        }
        return values::undef();
    }

}}}}  // namespace puppet::compiler::evaluation::functions
