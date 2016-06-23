#include <puppet/compiler/evaluation/postfix_evaluator.hpp>
#include <puppet/compiler/evaluation/access_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation {

    struct postfix_visitor : boost::static_visitor<>
    {
        postfix_visitor(evaluation::context& context, postfix_expression const& expression) :
            _evaluator(context)
        {
            _value = _evaluator.evaluate(expression.operand);
            _value_context = expression.operand.context();
            _splat = expression.is_splat();

            for (auto const& operation : expression.operations) {
                boost::apply_visitor(*this, operation);
                _splat = false;
            }
        }

        void operator()(selector_expression const& expression)
        {
            // Selector expressions create a new match scope
            match_scope scope{ _evaluator.context() };

            auto& cases = expression.cases;
            boost::optional<size_t> default_index;
            for (size_t i = 0; i < cases.size(); ++i) {
                auto& selector_case = cases[i];

                // Evaluate the option
                value selector = _evaluator.evaluate(selector_case.first);
                if (selector.is_default()) {
                    // Remember where the default case is and keep going
                    default_index = i;
                    continue;
                }

                if (_evaluator.is_match(_value, _value_context, selector, selector_case.second.context())) {
                    _value = _evaluator.evaluate(selector_case.second);
                    _value_context.end = selector_case.second.context().end;
                    return;
                }

                // If splat, treat each element as an option
                if (selector_case.first.is_splat()) {
                    auto unfolded = selector.to_array();
                    for (auto& element : unfolded) {
                        if (_evaluator.is_match(_value, _value_context, element, selector_case.second.context())) {
                            _value = _evaluator.evaluate(selector_case.second);
                            _value_context.end = selector_case.second.context().end;
                            return;
                        }
                    }
                }
            }

            // Handle no matching case
            if (!default_index) {
                throw evaluation_exception(
                    (boost::format("no matching selector case for value '%1%'.") %
                     _value
                    ).str(),
                    _value_context,
                    _evaluator.context().backtrace()
                );
            }

            // Evaluate the default case
            auto const& default_case = cases[*default_index];
            _value = _evaluator.evaluate(default_case.second);
            _value_context.end = default_case.second.context().end;
        }

        void operator()(access_expression const& expression)
        {
            access_evaluator evaluator(_evaluator.context());
            _value = evaluator.evaluate(_value, expression);
            _value_context.end = expression.end;
        }

        void operator()(method_call_expression const& expression)
        {
            // Find the function before executing the call to ensure it is imported
            _evaluator.context().find_function(expression.method.value);

            functions::call_context context{ _evaluator.context(), expression, _value, _value_context, _splat };
            _value = _evaluator.context().dispatcher().dispatch(context);
            _value_context.end = expression.context().end;
        }

        value& result()
        {
            return _value;
        }

     private:
        evaluator _evaluator;
        value _value;
        ast::context _value_context;
        bool _splat;
    };

    postfix_evaluator::postfix_evaluator(evaluation::context& context) :
        _context(context)
    {
    }

    value postfix_evaluator::evaluate(postfix_expression const& expression)
    {
        postfix_visitor visitor{ _context, expression };
        return rvalue_cast(visitor.result());
    }

}}}  // namespace puppet::compiler::evaluation
