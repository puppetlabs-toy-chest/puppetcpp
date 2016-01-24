#include <puppet/compiler/evaluation/dispatcher.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/functions/alert.hpp>
#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/functions/contain.hpp>
#include <puppet/compiler/evaluation/functions/crit.hpp>
#include <puppet/compiler/evaluation/functions/debug.hpp>
#include <puppet/compiler/evaluation/functions/defined.hpp>
#include <puppet/compiler/evaluation/functions/each.hpp>
#include <puppet/compiler/evaluation/functions/emerg.hpp>
#include <puppet/compiler/evaluation/functions/err.hpp>
#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/include.hpp>
#include <puppet/compiler/evaluation/functions/info.hpp>
#include <puppet/compiler/evaluation/functions/inline_epp.hpp>
#include <puppet/compiler/evaluation/functions/map.hpp>
#include <puppet/compiler/evaluation/functions/notice.hpp>
#include <puppet/compiler/evaluation/functions/realize.hpp>
#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/require.hpp>
#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/evaluation/functions/tag.hpp>
#include <puppet/compiler/evaluation/functions/tagged.hpp>
#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/warning.hpp>
#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/evaluation/operators/binary/assignment.hpp>
#include <puppet/compiler/evaluation/operators/binary/divide.hpp>
#include <puppet/compiler/evaluation/operators/binary/equals.hpp>
#include <puppet/compiler/evaluation/operators/binary/greater.hpp>
#include <puppet/compiler/evaluation/operators/binary/greater_equal.hpp>
#include <puppet/compiler/evaluation/operators/binary/in.hpp>
#include <puppet/compiler/evaluation/operators/binary/left_shift.hpp>
#include <puppet/compiler/evaluation/operators/binary/less.hpp>
#include <puppet/compiler/evaluation/operators/binary/less_equal.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::compiler::evaluation::operators;

namespace puppet { namespace compiler { namespace evaluation {

    void dispatcher::add_builtins()
    {
        // Add the built-in functions
        add(functions::alert::create_descriptor());
        add(functions::assert_type::create_descriptor());
        add(functions::contain::create_descriptor());
        add(functions::crit::create_descriptor());
        add(functions::debug::create_descriptor());
        add(functions::defined::create_descriptor());
        add(functions::each::create_descriptor());
        add(functions::emerg::create_descriptor());
        add(functions::err::create_descriptor());
        add(functions::fail::create_descriptor());
        add(functions::filter::create_descriptor());
        add(functions::include::create_descriptor());
        add(functions::info::create_descriptor());
        add(functions::inline_epp::create_descriptor());
        add(functions::map::create_descriptor());
        add(functions::notice::create_descriptor());
        add(functions::realize::create_descriptor());
        add(functions::reduce::create_descriptor());
        add(functions::require::create_descriptor());
        add(functions::split::create_descriptor());
        add(functions::tag::create_descriptor());
        add(functions::tagged::create_descriptor());
        add(functions::versioncmp::create_descriptor());
        add(functions::warning::create_descriptor());
        add(functions::with::create_descriptor());

        // Add the built-in binary operators
        add(binary::assignment::create_descriptor());
        add(binary::divide::create_descriptor());
        add(binary::equals::create_descriptor());
        add(binary::greater::create_descriptor());
        add(binary::greater_equal::create_descriptor());
        add(binary::in::create_descriptor());
        add(binary::left_shift::create_descriptor());
        add(binary::less::create_descriptor());
        add(binary::less_equal::create_descriptor());
    }

    void dispatcher::add(functions::descriptor descriptor)
    {
        if (descriptor.name().empty()) {
            throw runtime_error("cannot add a function with an empty name to the dispatcher.");
        }
        if (!descriptor.dispatchable()) {
            throw runtime_error("cannot add a function that is not dispatchable to the dispatcher.");
        }
        string name = descriptor.name();
        if (!_functions.emplace(rvalue_cast(name), rvalue_cast(descriptor)).second) {
            throw runtime_error((boost::format("function '%1%' already exists in the dispatcher.") % name).str());
        }
    }

    void dispatcher::add(binary::descriptor descriptor)
    {
        if (!descriptor.dispatchable()) {
            throw runtime_error("cannot add a binary operator that is not dispatchable to the dispatcher.");
        }
        if (find(descriptor.oper())) {
            throw runtime_error((boost::format("operator '%1%' already exists in the dispatcher.") % descriptor.oper()).str());
        }
        _binary_operators.emplace_back(rvalue_cast(descriptor));
    }

    void dispatcher::add(unary::descriptor descriptor)
    {
        if (!descriptor.dispatchable()) {
            throw runtime_error("cannot add a unary operator that is not dispatchable to the dispatcher.");
        }
        if (find(descriptor.oper())) {
            throw runtime_error((boost::format("operator '%1%' already exists in the dispatcher.") % descriptor.oper()).str());
        }
        _unary_operators.emplace_back(rvalue_cast(descriptor));
    }

    functions::descriptor* dispatcher::find(string const& name)
    {
        return const_cast<functions::descriptor*>(static_cast<dispatcher const*>(this)->find(name));
    }

    functions::descriptor const* dispatcher::find(string const& name) const
    {
        auto it = _functions.find(name);
        if (it == _functions.end()) {
            return nullptr;
        }
        return &it->second;
    }

    binary::descriptor* dispatcher::find(ast::binary_operator oper)
    {
        return const_cast<binary::descriptor*>(static_cast<dispatcher const*>(this)->find(oper));
    }

    binary::descriptor const* dispatcher::find(ast::binary_operator oper) const
    {
        auto it = std::find_if(_binary_operators.begin(), _binary_operators.end(), [=](auto const& descriptor) { return descriptor.oper() == oper; });
        if (it == _binary_operators.end()) {
            return nullptr;
        }
        return &*it;
    }

    unary::descriptor* dispatcher::find(ast::unary_operator oper)
    {
        return const_cast<unary::descriptor*>(static_cast<dispatcher const*>(this)->find(oper));
    }

    unary::descriptor const* dispatcher::find(ast::unary_operator oper) const
    {
        auto it = std::find_if(_unary_operators.begin(), _unary_operators.end(), [=](auto const& descriptor) { return descriptor.oper() == oper; });
        if (it == _unary_operators.end()) {
            return nullptr;
        }
        return &*it;
    }

    values::value dispatcher::dispatch(functions::call_context& context) const
    {
        // Find the requested function
        auto descriptor = find(context.name().value);
        if (!descriptor) {
            throw evaluation_exception((boost::format("unknown function '%1%'.") % context.name()).str(), context.name());
        }
        return descriptor->dispatch(context);
    }

    values::value dispatcher::dispatch(binary::call_context& context) const
    {
        // Find the requested function
        auto descriptor = find(context.oper());
        if (!descriptor) {
            throw evaluation_exception((boost::format("unknown binary operator '%1%'.") % context.oper()).str(), context.operator_context());
        }
        return descriptor->dispatch(context);
    }

    values::value dispatcher::dispatch(unary::call_context& context) const
    {
        // Find the requested function
        auto descriptor = find(context.oper());
        if (!descriptor) {
            throw evaluation_exception((boost::format("unknown unary operator '%1%'.") % context.oper()).str(), context.operator_context());
        }
        return descriptor->dispatch(context);
    }

    void dispatcher::fallback(fallback_type fallback)
    {
        _fallback = rvalue_cast(fallback);
    }

}}}  // namespace puppet::compiler::evaluation
