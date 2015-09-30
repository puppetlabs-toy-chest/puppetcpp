#include <puppet/runtime/functions/defined.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    struct defined_visitor : boost::static_visitor<bool>
    {
        defined_visitor(call_context& context, size_t index) :
            _context(context),
            _index(index)
        {
        }

        result_type operator()(string const& argument) const
        {
            auto& context = _context.evaluator().evaluation_context();

            // Check for variable lookup
            if (boost::starts_with(argument, "$")) {
                return context.lookup(argument.substr(1)).get();
            }
            // Otherwise, treat as a type / class / defined type name
            auto catalog = context.catalog();
            if (!catalog) {
                return false;
            }
            // Check for "main" class (no definition)
            if (argument == "main" || argument == "Main") {
                return context.find_scope("").get();
            }
            // Check for "settings" class (no definition)
            if (argument == "settings" || argument == "Settings") {
                return context.find_scope("settings").get();
            }
            // Check for built-in type or class
            if (types::resource(argument).is_builtin() || catalog->find_class(types::klass(argument), &context)) {
                return true;
            }
            // Lastly, check for defined type with a lowercase string
            return catalog->find_defined_type(boost::to_lower_copy(argument), &context);
        }

        result_type operator()(values::type const& argument) const
        {
            auto& context = _context.evaluator().evaluation_context();
            auto catalog = context.catalog();
            if (!catalog) {
                return false;
            }

            if (auto resource = boost::get<types::resource>(&argument)) {
                // Ensure the type isn't simply an unqualified Resource type
                if (resource->type_name().empty()) {
                    throw _context.evaluator().create_exception(_context.position(_index), (boost::format("expected a qualified %1%.") % types::resource::name()).str());
                }
                // If no title, check for built-in or defined type
                if (resource->title().empty()) {
                    return resource->is_builtin() || catalog->find_defined_type(boost::to_lower_copy(resource->title()), &context);
                }
                // Find the resource
                return catalog->find_resource(*resource);
            }
            if (auto klass = boost::get<types::klass>(&argument)) {
                // Ensure the type isn't simply an unqualified Class type
                if (klass->title().empty()) {
                    throw _context.evaluator().create_exception(_context.position(_index), (boost::format("expected a qualified %1%.") % types::klass::name()).str());
                }
                return catalog->find_resource(types::resource("class", klass->title()));
            }
            if (auto type = boost::get<types::type>(&argument)) {
                if (!type->type()) {
                    throw _context.evaluator().create_exception(_context.position(_index), (boost::format("expected a qualified %1%.") % types::type::name()).str());
                }
                // For resource types, simply recurse on the nested type
                if (boost::get<types::resource>(&*type->type())) {
                    return operator()(*type->type());
                }
                // For class types, only check if the class is defined
                if (auto klass = boost::get<types::klass>(&*type->type())) {
                    // Ensure the type isn't simply an unqualified Class type
                    if (klass->title().empty()) {
                        throw _context.evaluator().create_exception(_context.position(_index), (boost::format("expected a qualified %1%.") % types::klass::name()).str());
                    }
                    return catalog->find_class(*klass, &context);
                }
                throw _context.evaluator().create_exception(
                    _context.position(_index),
                    (
                        boost::format("expected %1% or %2% for %3% parameter but found %4%.") %
                        types::resource::name() %
                        types::klass::name() %
                        types::type::name() %
                        get_type(*type->type())
                    ).str());
            }

            // Treat as undef to raise an unexpected type error
            return operator()(undef());
        }

        template <typename T>
        result_type operator()(T const&) const
        {
            throw _context.evaluator().create_exception(
                _context.position(_index),
                (
                    boost::format("expected %1%, qualified %2%, qualified %3%, or qualified %4% for argument but found %5%.") %
                    types::string::name() %
                    types::resource::name() %
                    types::klass::name() %
                    types::type::name() %
                    get_type(_context.arguments()[_index])
                ).str());
        }

     private:
        call_context& _context;
        size_t _index;
    };

    value defined::operator()(call_context& context) const
    {
        // Ensure there is at least one argument
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw context.evaluator().create_exception(context.position(), (boost::format("expected at least 1 argument to '%1%' function.") % context.name()).str());
        }

        // Return true if any argument is defined
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (boost::apply_visitor(defined_visitor(context, i), dereference(arguments[i]))) {
                return true;
            }
        }
        return false;
    }

}}}  // namespace puppet::runtime::functions
