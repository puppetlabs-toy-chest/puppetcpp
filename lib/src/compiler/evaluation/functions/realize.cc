#include <puppet/compiler/evaluation/functions/realize.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/collectors/list_collector.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static void add_resource(evaluation::context& context, collectors::list_collector::list_type& list, types::resource resource, ast::context const& argument_context)
    {
        if (resource.is_class()) {
            throw evaluation_exception("classes cannot be realized.", argument_context, context.backtrace());
        }
        if (!resource.fully_qualified()) {
            throw evaluation_exception("expected a fully-qualified resource to realize.", argument_context, context.backtrace());
        }

        list.emplace(list.end(), make_pair(rvalue_cast(resource), argument_context));
    }

    static void add_resource(evaluation::context& context, collectors::list_collector::list_type& list, values::value const& argument, ast::context const& argument_context)
    {
        if (auto string = argument.as<std::string>()) {
            // Parse the string a a type
            auto resource = types::resource::parse(*string);
            if (!resource) {
                throw evaluation_exception(
                    (boost::format("expected a qualified resource string but found \"%1%\".") %
                     argument
                    ).str(),
                    argument_context,
                    context.backtrace()
                );
            }

            add_resource(context, list, rvalue_cast(*resource), argument_context);
            return;
        }
        if (auto array = argument.as<values::array>()) {
            // Recurse on array elements
            for (auto const& element : *array) {
                add_resource(context, list, element, argument_context);
            }
            return;
        }
        if (auto type = argument.as<values::type>()) {
            if (auto resource = boost::get<types::resource>(type)) {
                add_resource(context, list, *resource, argument_context);
                return;
            }
        }

        throw evaluation_exception(
            (boost::format("expected %1%, %2%, or qualified %3% for argument but found %4%.") %
             types::string::name() %
             types::array::name() %
             types::resource::name() %
             argument.infer_type()
            ).str(),
            argument_context,
            context.backtrace()
        );
    }

    descriptor realize::create_descriptor()
    {
        functions::descriptor descriptor{ "realize" };

        descriptor.add("Callable[Variant[String, Array[Any], Resource], 1]", [](call_context& context) {
            auto& arguments = context.arguments();
            auto& evaluation_context = context.context();

            // Populate a list of resource types from the arguments
            collectors::list_collector::list_type list;
            for (size_t i = 0; i < arguments.size(); ++i) {
                add_resource(evaluation_context, list, context.argument(i), context.argument_context(i));
            }

            values::array result;
            if (!list.empty()) {
                // Populate the resulting array
                for (auto const& element : list) {
                    result.emplace_back(element.first);
                }

                // Add a list collector
                evaluation_context.add(std::make_shared<collectors::list_collector>(rvalue_cast(list)));
            }
            return result;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
