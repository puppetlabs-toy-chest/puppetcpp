#include <puppet/runtime/functions/realize.hpp>
#include <puppet/runtime/collectors/list_collector.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct realize_visitor : boost::static_visitor<void>
    {
        realize_visitor(call_context& context, size_t index, list<pair<types::resource, position>>& list) :
            _context(context),
            _index(index),
            _list(list)
        {
        }

        result_type operator()(string const& argument)
        {
            auto resource = types::resource::parse(argument);
            if (!resource) {
                throw _context.evaluator().create_exception(
                    _context.position(_index),
                    (boost::format("expected a qualified resource string but found \"%1%\".") % argument).str());
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
                boost::apply_visitor(*this, dereference(element));
            }
        }
        result_type operator()(types::resource const& argument)
        {
            realize_resource(argument);
        }

        template <typename T>
        result_type operator()(T const& argument)
        {
            throw _context.evaluator().create_exception(_context.position(_index),
                (boost::format("expected %1%, %2%, or qualified %2% for argument but found %3%.") %
                   types::string::name() %
                   types::array::name() %
                   types::resource::name() %
                   get_type(argument)).str());
        }

     private:
        void realize_resource(types::resource resource)
        {
            auto& position = _context.position(_index);
            if (resource.is_class()) {
                throw _context.evaluator().create_exception(position, "classes cannot be realized.");
            }
            if (!resource.fully_qualified()) {
                throw _context.evaluator().create_exception(position, "expected a fully-qualified resource to realize.");
            }
            _list.emplace(_list.end(), make_pair(rvalue_cast(resource), position));
        }

        call_context& _context;
        size_t _index;
        list<pair<types::resource, position>>& _list;
    };

    value realize::operator()(call_context& context) const
    {
        auto& evaluator = context.evaluator();
        auto& arguments = context.arguments();
        auto catalog = evaluator.evaluation_context().catalog();
        if (arguments.empty()) {
            throw evaluator.create_exception(context.position(), (boost::format("expected at least one argument to '%1%' function.") % context.name()).str());
        }
        if (!catalog) {
            throw evaluator.create_exception(context.position(), (boost::format("cannot call '%1%' function: catalog functions are not supported.") % context.name()).str());
        }

        // Populate a list of resource types from the arguments
        list<pair<types::resource, position>> list;
        for (size_t i = 0; i < arguments.size(); ++i) {
            realize_visitor visitor{context, i, list};
            boost::apply_visitor(visitor, dereference(arguments[i]));
        }

        values::array result;
        if (!list.empty()) {
            // Populate the resulting array
            for (auto const& element : list) {
                result.emplace_back(element.first);
            }

            // Add a list collector
            catalog->add_collector(std::make_shared<collectors::list_collector>(evaluator.compilation_context(), rvalue_cast(list)));
        }
        return result;
    }

}}}  // namespace puppet::runtime::functions
