#include <puppet/compiler/evaluation/functions/realize.hpp>
#include <puppet/compiler/evaluation/collectors/list_collector.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct realize_visitor : boost::static_visitor<void>
    {
        realize_visitor(collectors::list_collector::list_type& list, ast::context const& context) :
            _list(list),
            _context(context)
        {
        }

        result_type operator()(string const& argument)
        {
            auto resource = types::resource::parse(argument);
            if (!resource) {
                throw evaluation_exception((boost::format("expected a qualified resource string but found \"%1%\".") % argument).str(), _context);
            }

            realize_resource(rvalue_cast(*resource));
        }

        result_type operator()(values::type const& argument)
        {
            boost::apply_visitor(*this, argument);
        }

        result_type operator()(values::array const& argument)
        {
            for (auto const& element : argument) {
                boost::apply_visitor(*this, element);
            }
        }
        result_type operator()(types::resource const& argument)
        {
            realize_resource(argument);
        }

        template <typename T>
        result_type operator()(T const& argument)
        {
            throw evaluation_exception(
                (boost::format("expected %1%, %2%, or qualified %2% for argument but found %3%.") %
                 types::string::name() %
                 types::array::name() %
                 types::resource::name() %
                 values::value(argument).get_type()
                ).str());
        }

     private:
        void realize_resource(types::resource resource)
        {
            if (resource.is_class()) {
                throw evaluation_exception("classes cannot be realized.", _context);
            }
            if (!resource.fully_qualified()) {
                throw evaluation_exception("expected a fully-qualified resource to realize.", _context);
            }

            _list.emplace(_list.end(), make_pair(rvalue_cast(resource), _context));
        }

        collectors::list_collector::list_type& _list;
        ast::context const& _context;
    };

    values::value realize::operator()(function_call_context& context) const
    {
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception((boost::format("expected at least one argument to '%1%' function.") % context.name()).str(), context.name());
        }

        // Populate a list of resource types from the arguments
        collectors::list_collector::list_type list;
        for (size_t i = 0; i < arguments.size(); ++i) {
            realize_visitor visitor{ list, context.argument_context(i) };
            boost::apply_visitor(visitor, arguments[i]);
        }

        values::array result;
        if (!list.empty()) {
            // Populate the resulting array
            for (auto const& element : list) {
                result.emplace_back(element.first);
            }

            // Add a list collector
            context.context().add(std::make_shared<collectors::list_collector>(rvalue_cast(list)));
        }
        return result;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
