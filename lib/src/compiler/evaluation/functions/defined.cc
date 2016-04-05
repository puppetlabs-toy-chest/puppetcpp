#include <puppet/compiler/evaluation/functions/defined.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static bool is_defined(evaluation::context& context, std::string const& argument, ast::context const& argument_context)
    {
        // Check for variable lookup
        if (boost::starts_with(argument, "$")) {
            ast::variable variable;
            variable.begin = argument_context.begin;
            variable.end = argument_context.end;
            variable.tree = argument_context.tree;
            variable.name = argument.substr(1);
            return context.lookup(variable, false).get();
        }

        // Check for built-in type
        if (types::resource::is_builtin(types::resource{argument}.type_name())) {
            return true;
        }

        // Check for "main" class (no definition)
        if (boost::iequals(argument, "main")) {
            return context.find_scope("").get();
        }
        // Check for "settings" class (no definition)
        if (boost::iequals(argument, "settings")) {
            return context.find_scope("settings").get();
        }

        // Check type name
        return context.is_defined(argument);
    }

    static bool is_defined(evaluation::context& context, values::type const& argument, ast::context const& argument_context);

    static bool is_defined(evaluation::context& context, types::resource const& argument, ast::context const& argument_context)
    {
        // Ensure the type isn't simply an unqualified Resource type
        if (argument.type_name().empty()) {
            throw evaluation_exception((boost::format("expected a qualified %1%.") % types::resource::name()).str(), argument_context, context.backtrace());
        }

        // If no title, check for built-in or defined type
        if (argument.title().empty()) {
            if (types::resource::is_builtin(argument.type_name())) {
                return true;
            }
            return context.is_defined(argument.title(), false, true);
        }

        // Find the resource in the catalog
        return context.catalog().find(argument);
    }

    static bool is_defined(evaluation::context& context, types::klass const& argument, ast::context const& argument_context)
    {
        // Ensure the type isn't simply an unqualified Class type
        if (argument.title().empty()) {
            throw evaluation_exception((boost::format("expected a qualified %1%.") % types::klass::name()).str(), argument_context, context.backtrace());
        }

        // Check that the class is defined
        return context.is_defined(argument.title(), true, false);
    }

    static bool is_defined(evaluation::context& context, types::type const& argument, ast::context const& argument_context)
    {
        if (!argument.parameter()) {
            throw evaluation_exception((boost::format("expected a parameterized %1%.") % types::type::name()).str(), argument_context, context.backtrace());
        }
        // For resource types, simply recurse on the nested type
        if (boost::get<types::resource>(argument.parameter().get())) {
            return is_defined(context, *argument.parameter(), argument_context);
        }
        // For class types, only check if the class is defined
        if (auto klass = boost::get<types::klass>(argument.parameter().get())) {
            // Ensure the type isn't simply an unqualified Class type
            if (klass->title().empty()) {
                throw evaluation_exception((boost::format("expected a qualified %1%.") % types::klass::name()).str(), argument_context, context.backtrace());
            }
            return context.is_defined(klass->title(), true, false);
        }
        throw evaluation_exception(
            (boost::format("expected %1% or %2% for type parameter but found %3%.") %
             types::resource::name() %
             types::klass::name() %
             values::value(*argument.parameter()).get_type()
            ).str(),
            argument_context,
            context.backtrace()
        );
    }

    static bool is_defined(evaluation::context& context, values::type const& argument, ast::context const& argument_context)
    {
        if (auto resource = boost::get<types::resource>(&argument)) {
            return is_defined(context, *resource, argument_context);
        }
        if (auto klass = boost::get<types::klass>(&argument)) {
            return is_defined(context, *klass, argument_context);
        }
        if (auto type = boost::get<types::type>(&argument)) {
            return is_defined(context, *type, argument_context);
        }
        throw evaluation_exception(
            (boost::format("expected %1%, %2%, or %3% but found %4%.") %
             types::resource::name() %
             types::klass::name() %
             types::type::name() %
             values::value(argument).get_type()
            ).str(),
            argument_context,
            context.backtrace()
        );
    }

    static bool is_defined(evaluation::context& context, values::value const& argument, ast::context const& argument_context)
    {
        if (auto string = argument.as<std::string>()) {
            return is_defined(context, *string, argument_context);
        }
        if (auto type = argument.as<values::type>()) {
            return is_defined(context, *type, argument_context);
        }

        throw evaluation_exception(
            (boost::format("expected %1% or %2% but found %3%.") %
             types::string::name() %
             types::type::name() %
             argument.get_type()
            ).str(),
            argument_context,
            context.backtrace()
        );
    }

    descriptor defined::create_descriptor()
    {
        functions::descriptor descriptor{ "defined" };

        descriptor.add("Callable[Variant[String, Type, Resource, Class], 1]", [](call_context& context) {
            // Return true if any argument is defined; otherwise false
            auto count = context.arguments().size();
            for (size_t i = 0; i < count; ++i) {
                if (is_defined(context.context(), context.argument(i), context.argument_context(i))) {
                    return true;
                }
            }
            return false;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
