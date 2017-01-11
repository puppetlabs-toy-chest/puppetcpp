#include <puppet/experimental/api.h>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/functions/alert.hpp>
#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/functions/crit.hpp>
#include <puppet/compiler/evaluation/functions/debug.hpp>
#include <puppet/compiler/evaluation/functions/each.hpp>
#include <puppet/compiler/evaluation/functions/emerg.hpp>
#include <puppet/compiler/evaluation/functions/epp.hpp>
#include <puppet/compiler/evaluation/functions/err.hpp>
#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/evaluation/functions/file.hpp>
#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/info.hpp>
#include <puppet/compiler/evaluation/functions/inline_epp.hpp>
#include <puppet/compiler/evaluation/functions/map.hpp>
#include <puppet/compiler/evaluation/functions/new.hpp>
#include <puppet/compiler/evaluation/functions/notice.hpp>
#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/reverse_each.hpp>
#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/evaluation/functions/step.hpp>
#include <puppet/compiler/evaluation/functions/type.hpp>
#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/warning.hpp>
#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet;
using namespace puppet::logging;
using namespace puppet::compiler;
using namespace puppet::compiler::evaluation;
using namespace puppet::runtime;

struct callback_logger : logger
{
    explicit callback_logger(void (*callback)(puppet_log_entry const*)) :
        _callback(callback)
    {
    }

 protected:
    void log_message(logging::level level, size_t line, size_t column, size_t length, string const& text, string const& path, string const& message) override
    {
        puppet_log_entry entry{
            static_cast<puppet_log_level>(level),
            static_cast<uint64_t>(line),
            static_cast<uint64_t>(column),
            static_cast<uint64_t>(length),
            {
                static_cast<uint64_t>(text.size()),
                text.data()
            },
            {
                static_cast<uint64_t>(path.size()),
                path.data()
            },
            {
                static_cast<uint64_t>(message.size()),
                message.data()
            }
        };
        _callback(&entry);
    }

    void log_backtrace(vector<evaluation::stack_frame> const& backtrace) override
    {
        // Unused
    }

 private:
    void (*_callback)(puppet_log_entry const*);
};

struct puppet_compiler_session
{
    puppet_compiler_session(char const* name, char const* directory, puppet_log_level level, void (*callback)(puppet_log_entry const*)) :
        _logger(callback)
    {
        _logger.level(static_cast<logging::level>(level));

        auto environment = make_shared<compiler::environment>(name, directory, settings{});
        auto& dispatcher = environment->dispatcher();

        // Add the supported built-in functions and operators
        // Resource-related functions are not included
        dispatcher.add(functions::alert::create_descriptor());
        dispatcher.add(functions::assert_type::create_descriptor());
        dispatcher.add(functions::crit::create_descriptor());
        dispatcher.add(functions::debug::create_descriptor());
        dispatcher.add(functions::each::create_descriptor());
        dispatcher.add(functions::emerg::create_descriptor());
        dispatcher.add(functions::epp::create_descriptor());
        dispatcher.add(functions::err::create_descriptor());
        dispatcher.add(functions::fail::create_descriptor());
        dispatcher.add(functions::file::create_descriptor());
        dispatcher.add(functions::filter::create_descriptor());
        dispatcher.add(functions::info::create_descriptor());
        dispatcher.add(functions::inline_epp::create_descriptor());
        dispatcher.add(functions::new_::create_descriptor());
        dispatcher.add(functions::map::create_descriptor());
        dispatcher.add(functions::notice::create_descriptor());
        dispatcher.add(functions::reduce::create_descriptor());
        dispatcher.add(functions::reverse_each::create_descriptor());
        dispatcher.add(functions::split::create_descriptor());
        dispatcher.add(functions::step::create_descriptor());
        dispatcher.add(functions::type::create_descriptor());
        dispatcher.add(functions::versioncmp::create_descriptor());
        dispatcher.add(functions::warning::create_descriptor());
        dispatcher.add(functions::with::create_descriptor());
        dispatcher.add_builtin_operators();

        _node = make_unique<compiler::node>(_logger, name, rvalue_cast(environment));
    }

    compiler::node& node() const
    {
        return *_node;
    }

 private:
    callback_logger _logger;
    unique_ptr<compiler::node> _node;
};

struct puppet_exception
{
    explicit puppet_exception(compilation_exception ex) :
        exception(rvalue_cast(ex))
    {
        auto& backtrace = exception.backtrace();
        if (!backtrace.empty()) {
            for (auto& frame : backtrace) {
                auto& context = frame.current();
                names.push_back(frame.name());
                auto& name = names.back();
                frames.push_back({
                    {
                        static_cast<uint64_t>(name.size()),
                        name.data()
                    },
                    {
                        context.tree ? static_cast<uint64_t>(context.tree->path().size()) : 0,
                        context.tree ? context.tree->path().data() : nullptr
                    },
                    {
                        static_cast<uint64_t>(context.begin.line()),
                        static_cast<uint64_t>(context.begin.offset()),
                    },
                    {
                        static_cast<uint64_t>(context.end.line()),
                        static_cast<uint64_t>(context.end.offset()),
                    }
                });
            }
        }
    }

    compilation_exception exception;
    vector<puppet_stack_frame> frames;
    deque<string> names;
};

extern "C"
{

puppet_compiler_session* puppet_create_session(char const* name, char const* directory, puppet_log_level level, void (*callback)(puppet_log_entry const*))
{
    try {
        return new puppet_compiler_session{ name, directory, level, callback };
    } catch (compilation_exception const&) {
        return nullptr;
    }
}

int puppet_define_function(puppet_compiler_session* session, char const* name, puppet_function_dispatch const* dispatches, uint64_t count)
{
    if (!session || !name) {
        return 0;
    }

    functions::descriptor descriptor{ name };

    try {
        for (uint64_t i = 0; i < count; ++i) {
            auto& dispatch = dispatches[i];

            if (!dispatch.specification || !dispatch.callback) {
                return 0;
            }

            // Copy the data into the closure
            auto data = dispatch.data;
            auto callback = dispatch.callback;
            descriptor.add(dispatch.specification, [=](functions::call_context& context) -> values::value {
                vector<puppet_value const*> arguments;
                arguments.reserve(context.arguments().size());
                for (auto& arg : context.arguments()) {
                    arguments.push_back(reinterpret_cast<puppet_value const*>(arg.get_ptr()));
                }
                auto result = callback(
                    reinterpret_cast<puppet_call_context*>(&context),
                    data,
                    arguments.data(),
                    static_cast<uint64_t>(arguments.size())
                );
                if (result.exception) {
                    // Delete any value and ignore it
                    delete reinterpret_cast<values::value*>(result.value);

                    // Throw the exception
                    throw result.exception->exception;
                }
                if (result.value) {
                    // Return the value
                    auto value = reinterpret_cast<values::value*>(result.value);
                    values::value ret{ rvalue_cast(*value) };
                    delete value;
                    return ret;
                }
                return values::undef{};
            });
        }

        session->node().environment().dispatcher().add(rvalue_cast(descriptor));
    } catch (runtime_error const&) {
        return 0;
    }
    return 1;
}

int puppet_block_passed(puppet_call_context* context)
{
    return context && reinterpret_cast<functions::call_context*>(context)->block() ? 1 : 0;
}

int puppet_get_caller_data(puppet_call_context const* context, puppet_caller_data* data)
{
    if (!context || !data) {
        return 0;
    }

    auto& eval_context = reinterpret_cast<functions::call_context const*>(context)->context();
    auto caller_context = eval_context.nearest_context();
    if (!caller_context || !caller_context->tree) {
        return 0;
    }

    auto& path = caller_context->tree->path();
    data->path.size = path.size();
    data->path.bytes = path.data();
    data->line = caller_context->begin.line();
    return 1;
}

puppet_evaluation_result puppet_yield(puppet_call_context* context, puppet_value** arguments, uint64_t count)
{
    puppet_evaluation_result result{ nullptr, nullptr };

    auto cxt = reinterpret_cast<functions::call_context*>(context);
    if (!cxt) {
        // Delete any passed in values
        for (uint64_t i = 0; i < count; ++i) {
            delete reinterpret_cast<values::value*>(arguments[i]);
        }
        result.exception = puppet_create_exception("missing call context.");
        return result;
    }

    // Move the values into a Puppet array
    values::array args;
    args.reserve(static_cast<size_t>(count));
    for (uint64_t i = 0; i < count; ++i) {
        auto ptr = reinterpret_cast<values::value*>(arguments[i]);
        args.emplace_back(rvalue_cast(*ptr));
        delete ptr;
    }

    // Yield to the block
    try {
        result.value = reinterpret_cast<puppet_value*>(new values::value(cxt->yield(args)));
    } catch (evaluation_exception const& ex) {
        result.exception = new puppet_exception(compilation_exception(ex));
    } catch (compilation_exception const& ex) {
        result.exception = new puppet_exception(ex);
    } catch (exception const& ex) {
        result.exception = new puppet_exception{ compilation_exception{ (boost::format("unhandled exception: %1%") % ex.what()).str() } };
    }
    return result;
}

void puppet_free_session(puppet_compiler_session* session)
{
    delete session;
}

puppet_evaluation_result puppet_evaluate_file(puppet_compiler_session* session, char const* path)
{
    puppet_evaluation_result result{ nullptr, nullptr };

    if (session) {
        auto& logger = session->node().logger();

        try {
            // Attempt to parse the file
            auto tree = parser::parse_file(logger, path);

            // Validate the AST and disallow catalog statements
            tree->validate(false, false);

            // Evaluate the AST and return the value
            evaluation::context context{ session->node() };
            evaluation::evaluator evaluator{ context };

            {
                evaluation::scoped_stack_frame frame{ context, evaluation::stack_frame{ "<main>", context.top_scope(), false }};
                result.value = reinterpret_cast<puppet_value*>(new values::value{ evaluator.evaluate(*tree) });
            }
        } catch (parse_exception const& ex) {
            result.exception = new puppet_exception{ compilation_exception{ ex, path } };
        } catch (evaluation_exception const& ex) {
            result.exception = new puppet_exception{ compilation_exception{ ex } };
        } catch (compilation_exception const& ex) {
            result.exception = new puppet_exception{ ex };
        } catch (exception const& ex) {
            result.exception = new puppet_exception{ compilation_exception{ (boost::format("unhandled exception: %1%") % ex.what()).str() } };
        }
    }
    return result;
}

puppet_exception* puppet_create_exception(char const* message)
{
    return new puppet_exception{ compilation_exception(message) };
}

puppet_exception* puppet_create_exception_with_context(char const* message, puppet_call_context const* context)
{
    if (!context) {
        return puppet_create_exception(message);
    }

    auto backtrace = reinterpret_cast<functions::call_context const*>(context)->context().backtrace();
    return new puppet_exception{ compilation_exception{ evaluation_exception{ message, rvalue_cast(backtrace) } } };
}

int puppet_get_exception_data(puppet_exception const* exception, puppet_exception_data* data)
{
    if (!exception || !data) {
        return 0;
    }

    auto& inner = exception->exception;
    data->message = inner.what();
    data->line = inner.line();
    data->column = inner.column();
    data->span = inner.length();
    data->text.size = static_cast<uint64_t>(inner.text().size());
    data->text.bytes = inner.text().data();
    data->path.size = static_cast<uint64_t>(inner.path().size());
    data->path.bytes = inner.path().data();
    data->frame_count = static_cast<uint32_t>(exception->frames.size());
    data->frames = exception->frames.data();
    return 1;
}

void puppet_free_exception(puppet_exception* exception)
{
    delete exception;
}

puppet_value* puppet_create_value()
{
    return reinterpret_cast<puppet_value*>(new values::value{});
}

puppet_value* puppet_clone_value(puppet_value const* value)
{
    return reinterpret_cast<puppet_value*>(new values::value{ *reinterpret_cast<values::value const*>(value) });
}

void puppet_free_value(puppet_value* value)
{
    delete reinterpret_cast<values::value*>(value);
}

int puppet_get_value_kind(puppet_value const* value, puppet_value_kind* kind)
{
    struct visitor : boost::static_visitor<puppet_value_kind>
    {
        result_type operator()(values::undef const&) const
        {
            return PUPPET_VALUE_UNDEF;
        }

        result_type operator()(values::defaulted const&) const
        {
            return PUPPET_VALUE_DEFAULT;
        }

        result_type operator()(int64_t) const
        {
            return PUPPET_VALUE_INTEGER;
        }

        result_type operator()(double) const
        {
            return PUPPET_VALUE_FLOAT;
        }

        result_type operator()(bool) const
        {
            return PUPPET_VALUE_BOOLEAN;
        }

        result_type operator()(string const&) const
        {
            return PUPPET_VALUE_STRING;
        }

        result_type operator()(values::regex const&) const
        {
            return PUPPET_VALUE_REGEXP;
        }

        result_type operator()(values::type const& type) const
        {
            return PUPPET_VALUE_TYPE;
        }

        result_type operator()(values::variable const& variable) const
        {
            return boost::apply_visitor(*this, variable.value());
        }

        result_type operator()(values::array const& array) const
        {
            return PUPPET_VALUE_ARRAY;
        }

        result_type operator()(values::hash const& hash) const
        {
            return PUPPET_VALUE_HASH;
        }

        result_type operator()(values::iterator const& iterator) const
        {
            return PUPPET_VALUE_ITERATOR;
        }
    };

    if (!value || !kind) {
        return 0;
    }

    *kind = boost::apply_visitor(visitor{}, *reinterpret_cast<values::value const*>(value));
    return 1;
}

int puppet_is_immutable(puppet_value const* value)
{
    auto ptr = reinterpret_cast<values::value const*>(value);
    return (!ptr ||
           static_cast<bool>(boost::get<values::variable>(ptr)) ||
           static_cast<bool>(boost::get<values::iterator>(ptr)))
           ? 1 : 0;
}

int puppet_set_undef(puppet_value* value)
{
    if (puppet_is_immutable(value)) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = values::undef{};
    return 1;
}

int puppet_set_default(puppet_value* value)
{
    if (puppet_is_immutable(value)) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = values::defaulted{};
    return 1;
}

int puppet_get_integer(puppet_value const* value, int64_t* data)
{
    if (!value || !data) {
        return 0;
    }
    if (auto integer = reinterpret_cast<values::value const*>(value)->as<int64_t>()) {
        *data = *integer;
        return 1;
    }
    return 0;
}

int puppet_set_integer(puppet_value* value, int64_t data)
{
    if (puppet_is_immutable(value)) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = data;
    return 1;
}

int puppet_get_float(puppet_value const* value, double* data)
{
    if (!value || !data) {
        return 0;
    }
    if (auto f = reinterpret_cast<values::value const*>(value)->as<double>()) {
        *data = *f;
        return 1;
    }
    return 0;
}

int puppet_set_float(puppet_value* value, double data)
{
    if (puppet_is_immutable(value)) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = data;
    return 1;
}

int puppet_get_boolean(puppet_value const* value, uint8_t* data)
{
    if (!value || !data) {
        return 0;
    }
    if (auto f = reinterpret_cast<values::value const*>(value)->as<bool>()) {
        *data = f ? 1 : 0;
        return 1;
    }
    return 0;
}

int puppet_set_boolean(puppet_value* value, uint8_t data)
{
    if (puppet_is_immutable(value)) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = data != 0;
    return 1;
}

int puppet_get_string(puppet_value const* value, puppet_utf8_string* data)
{
    if (value && data) {
        if (auto string = reinterpret_cast<values::value const*>(value)->as<std::string>()) {
            data->size = static_cast<uint64_t>(string->size());
            data->bytes = string->c_str();
            return 1;
        }
    }
    return 0;
}

int puppet_set_string(puppet_value* value, puppet_utf8_string const* data)
{
    if (puppet_is_immutable(value) || !data) {
        return 0;
    }
    *reinterpret_cast<values::value*>(value) = std::string{ data->bytes, data->size };
    return 1;
}

int puppet_get_regexp(puppet_value const* value, puppet_utf8_string* data)
{
    if (value && data) {
        if (auto regex = reinterpret_cast<values::value const*>(value)->as<values::regex>()) {
            data->size = static_cast<uint64_t>(regex->pattern().size());
            data->bytes = regex->pattern().c_str();
            return 1;
        }
    }
    return 0;
}

int puppet_set_regexp(puppet_value* value, puppet_utf8_string const* data)
{
    try {
        if (puppet_is_immutable(value) || !data) {
            return 0;
        }
        *reinterpret_cast<values::value*>(value) = values::regex{ std::string{ data->bytes, data->size } };
        return 1;
    } catch (utility::regex_exception const&) {
        return 0;
    }
}

int puppet_set_type(puppet_value* value, char const* specification)
{
    if (puppet_is_immutable(value) || !specification) {
        return 0;
    }

    auto result = values::type::parse(specification);
    if (!result) {
        return 0;
    }

    *reinterpret_cast<values::value*>(value) = rvalue_cast(*result);
    return 1;
}

puppet_value* puppet_create_array(uint64_t capacity)
{
    values::array array;
    array.reserve(capacity);
    return reinterpret_cast<puppet_value*>(new values::value{ rvalue_cast(array) });
}

int puppet_array_size(puppet_value const* value, uint64_t* size)
{
    if (value && size) {
        if (auto array = reinterpret_cast<values::value const*>(value)->as<values::array>()) {
            *size = static_cast<uint64_t>(array->size());
            return 1;
        }
    }
    return 0;
}

int puppet_array_elements(puppet_value const* value, puppet_value const** elements, uint64_t count)
{
    if (!value || !elements) {
        return 0;
    }
    auto array = reinterpret_cast<values::value const*>(value)->as<values::array>();
    if (!array || count > static_cast<uint64_t>(array->size())) {
        return 0;
    }
    for (uint64_t i = 0; i < count; ++i) {
        elements[i] = reinterpret_cast<puppet_value const*>((*array)[static_cast<size_t>(i)].get_ptr());
    }
    return 1;
}

int puppet_array_get(puppet_value const* value, uint64_t index, puppet_value const** element)
{
    if (!value || !element) {
        return 0;
    }
    auto array = reinterpret_cast<values::value const*>(value)->as<values::array>();
    if (!array || index >= static_cast<uint64_t>(array->size())) {
        return 0;
    }
    *element = reinterpret_cast<puppet_value const*>((*array)[static_cast<size_t>(index)].get_ptr());
    return 1;
}

int puppet_array_set(puppet_value* value, uint64_t index, puppet_value* element)
{
    if (puppet_is_immutable(value) || !element) {
        return 0;
    }

    // Note: using boost::get here for a mutable reference to the array; this relies on the above immutable check
    auto array = boost::get<values::array>(reinterpret_cast<values::value*>(value));
    if (!array || index >= static_cast<uint64_t>(array->size())) {
        return 0;
    }

    // Move the value into the array and delete the original
    auto ptr = reinterpret_cast<values::value*>(element);
    (*array)[index] = rvalue_cast(*ptr);
    delete ptr;
    return 1;
}

int puppet_array_push(puppet_value* value, puppet_value* element)
{
    if (puppet_is_immutable(value) || !element) {
        return 0;
    }
    // Note: using boost::get here for a mutable reference to the array; this relies on the above immutable check
    auto array = boost::get<values::array>(reinterpret_cast<values::value*>(value));
    if (!array) {
        return 0;
    }
    auto ptr = reinterpret_cast<values::value*>(element);
    array->push_back(rvalue_cast(*ptr));
    delete ptr;
    return 1;
}

puppet_value* puppet_create_hash()
{
    return reinterpret_cast<puppet_value*>(new values::value{ values::hash{} });
}

int puppet_hash_size(puppet_value const* value, uint64_t* size)
{
    if (value && size) {
        if (auto hash = reinterpret_cast<values::value const*>(value)->as<values::hash>()) {
            *size = static_cast<uint64_t>(hash->size());
            return 1;
        }
    }
    return 0;
}

int puppet_hash_elements(puppet_value const* value, puppet_hash_element* elements, uint64_t count)
{
    if (!value || !elements) {
        return 0;
    }
    auto hash = reinterpret_cast<values::value const*>(value)->as<values::hash>();
    if (!hash || count > static_cast<uint64_t>(hash->size())) {
        return 0;
    }
    uint64_t i = 0;
    for (auto it = hash->begin(); it != hash->end() && i < count; ++it, ++i) {
        auto& element = elements[i];
        element.key = reinterpret_cast<puppet_value const*>(&it->key());
        element.value = reinterpret_cast<puppet_value const*>(&it->value());
    }
    return 1;
}

int puppet_hash_get(puppet_value const* hash, puppet_value const* key, puppet_value const** value)
{
    if (!hash || !key || !value) {
        return 0;
    }

    auto hash_ptr = reinterpret_cast<values::value const*>(hash)->as<values::hash>();
    if (!hash_ptr) {
        return 0;
    }
    *value = reinterpret_cast<puppet_value const*>(hash_ptr->get(*reinterpret_cast<values::value const*>(key)));
    return 1;
}

int puppet_hash_set(puppet_value* hash, puppet_value* key, puppet_value* value)
{
    if (puppet_is_immutable(hash) || !key || !value) {
        return 0;
    }

    // Note: using boost::get here for a mutable reference to the array; this relies on the above immutable check
    auto hash_ptr = boost::get<values::hash>(reinterpret_cast<values::value*>(value));
    if (!hash_ptr) {
        return 0;
    }

    auto key_ptr = reinterpret_cast<values::value*>(key);
    auto value_ptr = reinterpret_cast<values::value*>(value);

    hash_ptr->set(rvalue_cast(*key_ptr), rvalue_cast(*value_ptr));

    delete key_ptr;
    delete value_ptr;
    return 1;
}

int puppet_iterate(puppet_value const* value, void const* data, int (*callback)(void const*, puppet_value const*, puppet_value const*))
{
    if (!value || !callback) {
        return 0;
    }

    try {
        // Iterate the value
        boost::apply_visitor(
            values::iteration_visitor{
                [&](auto const* key, auto const& value) {
                    return callback(data, reinterpret_cast<puppet_value const*>(key), reinterpret_cast<puppet_value const*>(&value)) != 0;
                }
            },
            *reinterpret_cast<values::value const*>(value)
        );
    } catch (values::type_not_iterable_exception const&) {
        return 0;
    }
    return 1;
}

puppet_value* puppet_value_to_string(puppet_value const* value)
{
    if (!value) {
        return nullptr;
    }

    ostringstream stream;
    stream << *reinterpret_cast<values::value const*>(value);
    return reinterpret_cast<puppet_value*>(new values::value{ stream.str() });
}

}  // extern "C"
