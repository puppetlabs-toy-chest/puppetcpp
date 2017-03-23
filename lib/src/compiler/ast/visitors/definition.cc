#include <puppet/compiler/ast/visitors/definition.hpp>
#include <puppet/runtime/types/class.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    definition::definition(callback_type callback) :
        _callback(rvalue_cast(callback))
    {
    }

    void definition::visit(syntax_tree const& tree)
    {
        _classes.clear();

        for (auto const& statement : tree.statements) {
            operator()(statement);
        }
    }

    void definition::operator()(ast::statement const& statement)
    {
        boost::apply_visitor(*this, statement);
    }

    void definition::operator()(class_statement const& statement)
    {
        _callback(qualify(statement.name.value), &statement);

        class_helper helper{ *this, statement };

        // Scan the body of the class for nested classes/defined types
        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void definition::operator()(defined_type_statement const& statement)
    {
        _callback(qualify(statement.name.value), &statement);
    }

    void definition::operator()(node_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(function_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(produces_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(consumes_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(application_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(site_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(type_alias_statement const& statement)
    {
        _callback({}, &statement);
    }

    void definition::operator()(function_call_statement const& statement)
    {

    }

    void definition::operator()(relationship_statement const& statement)
    {
    }

    void definition::operator()(break_statement const& statement)
    {
    }

    std::string definition::qualify(std::string const& name) const
    {
        std::string qualified;
        for (auto statement : _classes) {
            if (!qualified.empty()) {
                qualified += "::";
            }
            qualified += statement->name.value;
        }
        if (!qualified.empty()) {
            qualified += "::";
        }
        qualified += name;
        runtime::types::klass::normalize(qualified);
        return qualified;
    }

    definition::class_helper::class_helper(definition& visitor, ast::class_statement const& statement) :
        _visitor(visitor)
    {
        _visitor._classes.push_back(&statement);
    }

    definition::class_helper::~class_helper()
    {
        _visitor._classes.pop_back();
    }

}}}}  // namespace puppet::compiler::visitors
