#include <puppet/compiler/evaluation/functions/defined.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    struct defined_visitor : boost::static_visitor<bool>
    {
        defined_visitor(function_call_context& context, ast::context const& argument_context) :
            _context(context),
            _argument_context(argument_context)
        {
        }

        result_type operator()(string const& argument) const
        {
            auto& context = _context.context();

            // Check for variable lookup
            if (boost::starts_with(argument, "$")) {
                ast::variable variable;
                variable.begin = _argument_context.begin;
                variable.end = _argument_context.end;
                variable.tree = _argument_context.tree;
                variable.name = argument.substr(1);
                return context.lookup(variable, false).get();
            }

            // Check for built-in type
            if (types::resource(argument).is_builtin()) {
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
            return context.is_defined(argument);
        }

        result_type operator()(values::type const& argument) const
        {
            auto& context = _context.context();
            auto& catalog = context.catalog();

            if (auto resource = boost::get<types::resource>(&argument)) {
                // Ensure the type isn't simply an unqualified Resource type
                if (resource->type_name().empty()) {
                    throw evaluation_exception((boost::format("expected a qualified %1%.") % types::resource::name()).str(), _argument_context);
                }
                // If no title, check for built-in or defined type
                if (resource->title().empty()) {
                    if (resource->is_builtin()) {
                        return true;
                    }
                    return context.is_defined(resource->title(), false, true);
                }
                // Find the resource in the catalog
                return catalog.find(*resource);
            }
            if (auto klass = boost::get<types::klass>(&argument)) {
                // Ensure the type isn't simply an unqualified Class type
                if (klass->title().empty()) {
                    throw evaluation_exception((boost::format("expected a qualified %1%.") % types::klass::name()).str(), _argument_context);
                }
                return context.is_defined(klass->title(), true, false);
            }
            if (auto type = boost::get<types::type>(&argument)) {
                if (!type->parameter()) {
                    throw evaluation_exception((boost::format("expected a qualified %1%.") % types::type::name()).str(), _argument_context);
                }
                // For resource types, simply recurse on the nested type
                if (boost::get<types::resource>(&*type->parameter())) {
                    return operator()(*type->parameter());
                }
                // For class types, only check if the class is defined
                if (auto klass = boost::get<types::klass>(&*type->parameter())) {
                    // Ensure the type isn't simply an unqualified Class type
                    if (klass->title().empty()) {
                        throw evaluation_exception((boost::format("expected a qualified %1%.") % types::klass::name()).str(), _argument_context);
                    }
                    return context.is_defined(klass->title(), true, false);
                }
                throw evaluation_exception(
                    (
                     boost::format("expected %1% or %2% for %3% parameter but found %4%.") %
                     types::resource::name() %
                     types::klass::name() %
                     types::type::name() %
                     values::value(*type->parameter()).get_type()
                    ).str(),
                    _argument_context);
            }

            // Treat as undef to raise an unexpected type error
            return operator()(values::undef());
        }

        template <typename T>
        result_type operator()(T const& argument) const
        {
            throw evaluation_exception(
                (
                 boost::format("expected %1%, qualified %2%, qualified %3%, or qualified %4% for argument but found %5%.") %
                 types::string::name() %
                 types::resource::name() %
                 types::klass::name() %
                 types::type::name() %
                 values::value(argument).get_type()
                ).str(),
                _argument_context);
        }

     private:
        function_call_context& _context;
        ast::context const& _argument_context;
    };

    values::value defined::operator()(function_call_context& context) const
    {
        // Ensure there is at least one argument
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception((boost::format("expected at least one argument to '%1%' function.") % context.name()).str(), context.name());
        }

        // Return true if any argument is defined
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (boost::apply_visitor(defined_visitor(context, context.argument_context(i)), arguments[i])) {
                return true;
            }
        }
        return false;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
