#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/validation/type_validator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::compiler;

namespace puppet { namespace runtime { namespace values {

    type_variant& type::get()
    {
        return _value;
    }

    type_variant const& type::get() const
    {
        return _value;
    }

    bool type::is_alias() const
    {
        return boost::get<types::alias>(&_value);
    }

    type const& type::dereference() const
    {
        if (auto alias = boost::get<types::alias>(&_value)) {
            return alias->resolved_type().dereference();
        }
        return *this;
    }

    struct generalize_visitor : boost::static_visitor<type>
    {
        template <typename T>
        result_type operator()(T const& type) const
        {
            return type.generalize();
        }
    };

    type type::generalize() const
    {
        return boost::apply_visitor(generalize_visitor{}, _value);
    }

    struct is_instance_visitor : boost::static_visitor<bool>
    {
        is_instance_visitor(values::value const& value, types::recursion_guard& guard) :
            _value(value),
            _guard(guard)
        {
        }

        template <typename T>
        bool operator()(T const& type) const
        {
            return type.is_instance(_value, _guard);
        }

     private:
        values::value const& _value;
        types::recursion_guard& _guard;
    };

    bool type::is_instance(values::value const& value, types::recursion_guard& guard) const
    {
        return boost::apply_visitor(is_instance_visitor{ value, guard }, _value);
    }

    struct is_assignable_visitor : boost::static_visitor<bool>
    {
        is_assignable_visitor(values::type const& type, types::recursion_guard& guard) :
            _type(type),
            _guard(guard)
        {
        }

        template <typename T>
        bool operator()(T const& type) const
        {
            return type.is_assignable(_type, _guard);
        }

     private:
        values::type const& _type;
        types::recursion_guard& _guard;
    };

    bool type::is_assignable(values::type const& type, types::recursion_guard& guard) const
    {
        // Check for Variant[T1, T2...]; all types must be assignable to this type
        if (auto variant = boost::get<types::variant>(&type._value)) {
            for (auto& t : variant->types()) {
                if (!is_assignable(*t, guard)) {
                    return false;
                }
            }
            return true;
        }

        // Check for NotUndef[T] and check T for assignability to this type
        if (auto not_undef = boost::get<types::not_undef>(&type._value)) {
            if (not_undef->type()) {
                return is_assignable(*not_undef->type(), guard);
            }
        }
        return boost::apply_visitor(is_assignable_visitor{ type, guard }, _value);
    }

    bool type::is_real(types::recursion_guard& guard) const
    {
        if (auto alias = boost::get<types::alias>(&_value)) {
            auto result = guard.add(*alias);
            if (result.recursed()) {
                return result.value();
            }
            result.value(alias->resolved_type().is_real(guard));
            return result.value();
        }
        if (auto variant = boost::get<types::variant>(&_value)) {
            // For variants, consider it real if all the types are real
            // Skip over any unreal member that is referencing this variant
            bool has_real_type = false;
            for (auto& type : variant->types()) {
                if (!type->is_real(guard)) {
                    if (type->references(*this, guard)) {
                        continue;
                    }
                    return false;
                } else {
                    has_real_type = true;
                }
            }
            return variant->types().empty() || has_real_type;
        }
        // All other types are real
        return true;
    }

    bool type::references(values::type const& other, types::recursion_guard& guard) const
    {
        // Check if the given type is this type
        if (this == &other) {
            return true;
        }
        if (auto alias = boost::get<types::alias>(&_value)) {
            auto result = guard.add(*alias, &other);
            if (result.recursed()) {
                return result.value();
            }
            result.value(alias->resolved_type().references(other, guard));
            return result.value();
        }
        if (auto variant = boost::get<types::variant>(&_value)) {
            // For variants, check the types
            for (auto& type : variant->types()) {
                if (type->references(other, guard)) {
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    struct write_visitor : boost::static_visitor<void>
    {
        write_visitor(ostream& os, bool expand) :
            _os(os),
            _expand(expand)
        {
        }

        template <typename T>
        void operator()(T const& type) const
        {
            type.write(_os, _expand);
        }

     private:
        ostream& _os;
        bool _expand;
    };

    void type::write(ostream& stream, bool expand) const
    {
        boost::apply_visitor(write_visitor{ stream, expand }, _value);
    }

    type const* type::find(string const& name)
    {
        static const unordered_map<string, type> puppet_types = {
            { types::any::name(),           types::any() },
            { types::array::name(),         types::array() },
            { types::boolean::name(),       types::boolean() },
            { types::callable::name(),      types::callable() },
            { types::catalog_entry::name(), types::catalog_entry() },
            { types::collection::name(),    types::collection() },
            { types::data::name(),          types::data() },
            { types::defaulted::name(),     types::defaulted() },
            { types::enumeration::name(),   types::enumeration() },
            { types::floating::name(),      types::floating() },
            { types::hash::name(),          types::hash() },
            { types::integer::name(),       types::integer() },
            { types::iterable::name(),      types::iterable() },
            { types::iterator::name(),      types::iterator() },
            { types::klass::name(),         types::klass() },
            { types::not_undef::name(),     types::not_undef() },
            { types::numeric::name(),       types::numeric() },
            { types::optional::name(),      types::optional() },
            { types::pattern::name(),       types::pattern() },
            { types::regexp::name(),        types::regexp() },
            { types::resource::name(),      types::resource() },
            { types::runtime::name(),       types::runtime() },
            { types::scalar::name(),        types::scalar() },
            { types::string::name(),        types::string() },
            { types::structure::name(),     types::structure() },
            { types::tuple::name(),         types::tuple() },
            { types::type::name(),          types::type() },
            { types::undef::name(),         types::undef() },
            { types::variant::name(),       types::variant() },
        };

        auto it = puppet_types.find(name);
        return it == puppet_types.end() ? nullptr : &it->second;
    }

    boost::optional<type> type::create(ast::postfix_expression const& expression, compiler::evaluation::context* context)
    {
        validation::type_validator::validate(expression);

        // Use an empty evaluation context if not given one (no node, catalog, registry, or dispatcher access)
        evaluation::context empty{};
        if (!context) {
            context = &empty;
        }

        evaluation::evaluator evaluator{ *context };
        auto result = evaluator.evaluate(expression);
        if (result.as<type>()) {
            return result.move_as<type>();
        }
        return boost::none;
    }

    boost::optional<type> type::parse(string const& expression, compiler::evaluation::context* context)
    {
        // Type specifications are postfix access expressions
        auto postfix = parser::parse_postfix(expression);
        if (!postfix) {
            return boost::none;
        }
        return create(*postfix, context);
    }

    ostream& operator<<(ostream& os, values::type const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(type const& left, type const& right)
    {
        return left.get() == right.get();
    }

    bool operator!=(type const& left, type const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::type const& type)
    {
        // Hash the underlying type
        return hash_value(type.get());
    }

    void type_set::add(values::type const& type)
    {
        if (_set.emplace(&type).second) {
            _types.emplace_back(&type);
        }
    }

    void type_set::clear()
    {
        _types.clear();
        _set.clear();
    }

    bool type_set::empty() const
    {
        return _types.empty();
    }

    size_t type_set::size() const
    {
        return _types.size();
    }

    type const& type_set::operator[](size_t index) const
    {
        return *_types[index];
    }

    ostream& operator<<(ostream& os, type_set const& set)
    {
        auto count = set.size();
        for (size_t i = 0; i < count; ++i) {
            if (i > 0) {
                if (count > 2) {
                    os << ",";
                }
                os << " ";
                if (i == (count - 1)) {
                    os << "or ";
                }
            }
            os << set[i];
        }
        return os;
    }

}}}  // namespace puppet::runtime::values
