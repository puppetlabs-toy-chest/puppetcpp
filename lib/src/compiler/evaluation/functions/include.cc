#include <puppet/compiler/evaluation/functions/include.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static void declare_class(evaluation::context& context, types::klass const& klass, ast::context const& declaration_context, boost::optional<compiler::relationship> const& relationship)
    {
        types::resource type("class", klass.title());

        if (!type.fully_qualified()) {
            throw evaluation_exception("cannot declare a class with an unspecified title.", declaration_context, context.backtrace());
        }

        // Declare the class
        auto resource = context.declare_class(klass.title(), declaration_context);

        // Add a relationship to the current resource
        if (relationship) {
            auto* current = context.current_scope()->resource();
            if (!current) {
                throw evaluation_exception("the current scope has no associated resource to form a relationship with.", declaration_context, context.backtrace());
            }
            context.catalog().relate(*relationship, *current, *resource);
        }
    }

    static void declare_class(evaluation::context& context, values::value name, ast::context const& declaration_context, boost::optional<compiler::relationship> const& relationship)
    {
        if (name.as<std::string>()) {
            declare_class(context, types::klass{ name.move_as<std::string>() }, declaration_context, relationship);
            return;
        }
        if (name.as<values::array>()) {
            // Recurse on each element of the array
            auto array = name.move_as<values::array>();
            for (auto& element : array) {
                declare_class(context, rvalue_cast(element.get()), declaration_context, relationship);
            }
            return;
        }
        if (name.as<values::type>()) {
            auto type = name.move_as<values::type>();
            if (auto klass = boost::get<types::klass>(&type)) {
                declare_class(context, *klass, declaration_context, relationship);
                return;
            }
            if (auto resource = boost::get<types::resource>(&type)) {
                if (!resource->is_class()) {
                    throw evaluation_exception("resource type must be class.", declaration_context, context.backtrace());
                }
                declare_class(context, types::klass{ resource->title() }, declaration_context, relationship);
                return;
            }
        }
        throw evaluation_exception((boost::format("cannot declare class with argument type %1%.") % name.get_type()).str(), declaration_context, context.backtrace());
    }

    void declare_class(call_context& context, boost::optional<compiler::relationship> relationship = boost::none)
    {
        auto count = context.arguments().size();
        for (size_t i = 0; i < count; ++i) {
            declare_class(context.context(), rvalue_cast(context.argument(i)), context.argument_context(i), relationship);
        }
    }

    descriptor include::create_descriptor()
    {
        functions::descriptor descriptor{ "include" };

        descriptor.add("Callable[Variant[String, Array[Any], Class, Resource], 1]", [](call_context& context) {
            declare_class(context);
            return values::undef{};
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
