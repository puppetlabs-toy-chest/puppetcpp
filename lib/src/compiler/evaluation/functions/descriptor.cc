#include <puppet/compiler/evaluation/functions/descriptor.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor::descriptor(string name) :
        _name(rvalue_cast(name))
    {
    }

    string const& descriptor::name() const
    {
        return _name;
    }

    bool descriptor::dispatchable() const
    {
        return !_dispatch_descriptors.empty();
    }

    void descriptor::add(string const& signature, callback_type callback)
    {
        auto callable = values::type::parse_as<types::callable>(signature);
        if (!callable) {
            throw runtime_error((boost::format("function '%1%' cannot add an overload with invalid signature '%2%'.") % _name % signature).str());
        }

        dispatch_descriptor descriptor;
        descriptor.signature = rvalue_cast(*callable);
        descriptor.callback = rvalue_cast(callback);
        _dispatch_descriptors.emplace_back(rvalue_cast(descriptor));
    }

    values::value descriptor::dispatch(call_context& context) const
    {
        // Search for a dispatch descriptor with a matching signature
        // TODO: in the future, this should dispatch to the most specific overload rather than the first dispatchable overload
        for (auto& descriptor : _dispatch_descriptors) {
            if (descriptor.signature.can_dispatch(context)) {
                return descriptor.callback(context);
            }
        }

        // Find the reason the call could not be dispatched
        auto invocable = check_argument_count(context);
        check_block_parameters(context, invocable);
        check_parameter_types(context, invocable);

        // Generic error in case the above fails
        throw evaluation_exception((boost::format("function '%1%' cannot be dispatched.") % _name).str(), context.name());
    }

    vector<descriptor::dispatch_descriptor const*> descriptor::check_argument_count(call_context const& context) const
    {
        // The call could not be dispatched, determine the reason
        auto argument_count = static_cast<int64_t>(context.arguments().size());
        bool block_passed = static_cast<bool>(context.block());
        int64_t min_arguments = -1;
        int64_t max_arguments = -1;

        // Get the argument counts, block parameter counts, and the set of descriptors that could be invoked
        vector<dispatch_descriptor const*> invocable;
        for (auto& descriptor : _dispatch_descriptors) {

            // Find the minimum number of arguments required for any overload
            if (min_arguments < 0 || min_arguments > descriptor.signature.min()) {
                min_arguments = descriptor.signature.min();
            }
            // Find the maximum number of arguments required for any overload
            if (max_arguments < 0 || max_arguments < descriptor.signature.max()) {
                max_arguments = descriptor.signature.max();
            }

            // Ignore overloads that aren't a match for number of arguments given
            if (argument_count < descriptor.signature.min() || argument_count > descriptor.signature.max()) {
                continue;
            }

            // Ignore overloads with a block mismatch
            types::callable const* block;
            bool required = false;
            tie(block, required) = descriptor.signature.block();
            if ((!block && block_passed) || (block && required && !block_passed)) {
                continue;
            }

            invocable.emplace_back(&descriptor);
        }
        // Check for argument count mismatch
        if (argument_count != min_arguments && min_arguments == max_arguments) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects %2% %3%.") %
                 _name %
                 min_arguments %
                 (min_arguments == 1 ? "argument" : "arguments")
                ).str(),
                (argument_count == 0 || argument_count < min_arguments) ? context.name() : context.argument_context(argument_count - 1)
            );
        }
        if (argument_count < min_arguments) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects at least %2% %3%.") %
                 _name %
                 min_arguments %
                 (min_arguments == 1 ? "argument" : "arguments")
                ).str(),
                context.name()
            );
        }
        if (argument_count > max_arguments) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects at most %2% %3%.") %
                 _name %
                 max_arguments %
                 (max_arguments == 1 ? "argument" : "arguments")
                ).str(),
                argument_count == 0 ? context.name() : context.argument_context(argument_count - 1)
            );
        }
        return invocable;
    }

    void descriptor::check_block_parameters(call_context const& context, vector<dispatch_descriptor const*> const& invocable) const
    {
        auto& block = context.block();

        // If the invocable set is empty, then there was a block mismatch
        if (invocable.empty()) {
            if (block) {
                throw evaluation_exception((boost::format("function '%1%' does not accept a block.") % _name).str(), *block);
            }
            throw evaluation_exception((boost::format("function '%1%' requires a block to be passed.") % _name).str(), context.name());
        }

        // If there's no block, nothing to validate
        if (!block) {
            return;
        }

        // Find block parameter count mismatch
        int64_t block_parameter_count = static_cast<int64_t>(block->parameters.size());
        int64_t min_block_parameters = -1;
        int64_t max_block_parameters = -1;
        for (auto descriptor : invocable) {
            // Ignore overloads with a block mismatch
            types::callable const* block_signature;
            bool required = false;
            tie(block_signature, required) = descriptor->signature.block();
            if (!block_signature) {
                continue;
            }

            // Find the minimum number of block parameters for any invocable overload
            if (min_block_parameters < 0 || min_block_parameters > block_signature->min()) {
                min_block_parameters = block_signature->min();
            }
            // Find the minimum number of block parameters for any invocable overload
            if (max_block_parameters < 0 || max_block_parameters < block_signature->max()) {
                max_block_parameters = block_signature->max();
            }
        }

        // Check for parameter count mismatch
        if (block_parameter_count != min_block_parameters && min_block_parameters == max_block_parameters) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects %2% block %3%.") %
                 _name %
                 min_block_parameters %
                 (min_block_parameters == 1 ? "parameter" : "parameters")
                ).str(),
                (block_parameter_count == 0 || block_parameter_count < min_block_parameters) ?
                static_cast<ast::context>(*block) :
                block->parameters[block_parameter_count - 1].context()
            );
        }
        if (block_parameter_count < min_block_parameters) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects at least %2% block %3%.") %
                 _name %
                 min_block_parameters %
                 (min_block_parameters == 1 ? "parameter" : "parameters")
                ).str(),
                *block
            );
        }
        if (block_parameter_count > max_block_parameters) {
            throw evaluation_exception(
                (boost::format("function '%1%' expects at least %2% block %3%.") %
                 _name %
                 max_block_parameters %
                 (max_block_parameters == 1 ? "parameter" : "parameters")
                ).str(),
                block_parameter_count == 0 ?
                static_cast<ast::context>(*block) :
                block->parameters[block_parameter_count - 1].context()
            );
        }
    }

    void descriptor::check_parameter_types(call_context const& context, vector<dispatch_descriptor const*> const& invocable) const
    {
        struct indirect_hasher
        {
            size_t operator()(values::type const* type) const
            {
                return hash_value(*type);
            }
        };

        struct indirect_comparer
        {
            bool operator()(values::type const* right, values::type const* left) const
            {
                return *right == *left;
            }
        };

        // Determine the first (lowest index) argument with a type mismatch
        int64_t min_argument_mismatch = -1;
        for (auto descriptor : invocable) {
            auto index = descriptor->signature.find_mismatch(context.arguments());
            if (min_argument_mismatch < 0 || min_argument_mismatch > index) {
                min_argument_mismatch = index;
            }
        }

        // Number of arguments and block parameters is correct; the problem lies with one of the argument's type
        // This determines the set of possible types for the first
        vector<values::type const*> types;
        unordered_set<values::type const*, indirect_hasher, indirect_comparer> type_set;

        for (auto& descriptor : invocable) {
            auto type = descriptor->signature.parameter_type(min_argument_mismatch);
            if (!type || !type_set.emplace(type).second) {
                continue;
            }
            types.emplace_back(type);
        }
        if (types.empty()) {
            return;
        }
        ostringstream type_message;
        auto count = types.size();
        for (size_t i = 0; i < count; ++i) {
            if (i > 0) {
                if (count > 2) {
                    type_message << ",";
                }
                type_message << " ";
            }
            if (count != 0 && i == (count - 1)) {
                type_message << "or ";
            }
            type_message << *types[i];
        }
        throw evaluation_exception(
            (boost::format("function '%1%' expects %2% but was given %3%.") %
             _name %
             type_message.str() %
             context.argument(min_argument_mismatch).get_type()
            ).str(),
            context.argument_context(min_argument_mismatch));
    }

}}}}  // namespace puppet::compiler::evaluation::functions
