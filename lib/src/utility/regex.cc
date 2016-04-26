#include <puppet/utility/regex.hpp>

using namespace std;

namespace puppet { namespace utility {

    regex::regions::regions()
    {
        onig_region_init(&_data);
    }

    regex::regions::~regions()
    {
        onig_region_free(&_data, 0);
    }

    regex::regions::regions(regions const& other)
    {
        onig_region_init(&_data);
        // Sigh, C developers can't be bothered to be const-correct
        onig_region_copy(&_data, const_cast<OnigRegion*>(&other._data));
    }

    regex::regions::regions(regions&& other)
    {
        _data = other._data;
        onig_region_init(&other._data);
    }

    regex::regions& regex::regions::operator=(regions const& other)
    {
        onig_region_init(&_data);
        // Sigh, C developers can't be bothered to be const-correct
        onig_region_copy(&_data, const_cast<OnigRegion*>(&other._data));
        return *this;
    }

    regex::regions& regex::regions::operator=(regions&& other)
    {
        _data = other._data;
        onig_region_init(&other._data);
        return *this;
    }

    size_t regex::regions::count() const
    {
        return static_cast<size_t>(_data.num_regs);
    }

    bool regex::regions::empty(size_t index) const
    {
        if (index >= static_cast<size_t>(_data.num_regs)) {
            return true;
        }
        return _data.beg[index] == _data.end[index];
    }

    size_t regex::regions::begin(size_t index) const
    {
        if (index >= static_cast<size_t>(_data.num_regs)) {
            return numeric_limits<size_t>::max();
        }
        return _data.beg[index];
    }

    size_t regex::regions::end(size_t index) const
    {
        if (index >= static_cast<size_t>(_data.num_regs)) {
            return numeric_limits<size_t>::max();
        }
        return _data.end[index];
    }

    string regex::regions::substring(string const& str, size_t index) const
    {
        auto begin = this->begin(index);
        auto end = this->end(index);
        if (begin >= str.size() || end > str.size()) {
            return {};
        }
        return {
            str.begin() + begin,
            str.begin() + end
        };
    }

    vector<string> regex::regions::substrings(string const& str) const
    {
        vector<string> strings;
        for (int i = 0; i < _data.num_regs; ++i) {
            strings.emplace_back(substring(str, i));
        }
        return strings;
    }

    regex_exception::regex_exception(string const& message, int code) :
        runtime_error(message),
        _code(code)
    {
    }

    int regex_exception::code() const
    {
        return _code;
    }

    regex::regex(string const& expression) :
        _wrapper(make_shared<wrapper>())
    {
        OnigErrorInfo error_info;
        int result = onig_new_without_alloc(
            &_wrapper->get(),
            reinterpret_cast<OnigUChar const*>(expression.data()),
            reinterpret_cast<OnigUChar const*>(expression.data() + expression.size()),
            ONIG_OPTION_DEFAULT,
            ONIG_ENCODING_UTF8,
            ONIG_SYNTAX_RUBY,
            &error_info
        );
        if (result != ONIG_NORMAL) {
            OnigUChar message[ONIG_MAX_ERROR_MESSAGE_LEN] = {};
            onig_error_code_to_str(message, result, &error_info);
            throw regex_exception(reinterpret_cast<char const*>(message), result);
        }
    }

    bool regex::match(string const& str, regex::regions* regions) const
    {
        auto start = str.data();
        auto end = start + str.size();

        // Casting away const on _regex; onig_match should internally be thread safe despite not being const-correct
        auto result = onig_match(
            const_cast<regex_t*>(&_wrapper->get()),
            reinterpret_cast<OnigUChar const*>(start),
            reinterpret_cast<OnigUChar const*>(end),
            reinterpret_cast<OnigUChar const*>(start),
            regions ? &regions->_data : nullptr,
            ONIG_OPTION_NONE
        );
        // Check for no match or a match that did not span the entire length of the string
        if (result == ONIG_MISMATCH || result != (end - start)) {
            // Ensure no regions are returned when there is no match
            if (regions) {
                regions->_data.num_regs = 0;
            }
            return false;
        }
        if (result < 0) {
            OnigUChar message[ONIG_MAX_ERROR_MESSAGE_LEN] = {};
            onig_error_code_to_str(message, result);
            throw regex_exception(reinterpret_cast<char const*>(message), result);
        }
        return true;
    }

    bool regex::search(string const& str, regex::regions* regions, size_t offset) const
    {
        auto start = str.data();
        auto end = start + str.size();

        // Casting away const on _regex; onig_search should internally be thread safe despite not being const-correct
        auto result = onig_search(
            const_cast<regex_t*>(&_wrapper->get()),
            reinterpret_cast<OnigUChar const*>(start),
            reinterpret_cast<OnigUChar const*>(end),
            reinterpret_cast<OnigUChar const*>(start + offset),
            reinterpret_cast<OnigUChar const*>(end),
            regions ? &regions->_data : nullptr,
            ONIG_OPTION_NONE
        );
        if (result == ONIG_MISMATCH) {
            // Ensure no regions are returned when there is no match
            if (regions) {
                regions->_data.num_regs = 0;
            }
            return false;
        }
        if (result < 0) {
            OnigUChar message[ONIG_MAX_ERROR_MESSAGE_LEN] = {};
            onig_error_code_to_str(message, result);
            throw regex_exception(reinterpret_cast<char const*>(message), result);
        }
        return true;
    }

    regex::wrapper::wrapper()
    {
        memset(&_regex, 0, sizeof(_regex));
    }

    regex::wrapper::~wrapper()
    {
        onig_free_body(&_regex);
    }

    regex_t const& regex::wrapper::get() const
    {
        return _regex;
    }

    regex_t& regex::wrapper::get()
    {
        return _regex;
    }

    regex_iterator::regex_iterator()
    {
        move_to_end();
    }

    regex_iterator::regex_iterator(utility::regex const& regex, string const& str) :
        _regex(&regex),
        _string(&str),
        _offset(0)
    {
        increment();
    }

    void regex_iterator::increment()
    {
        if (!_regex || !_string || _offset >= _string->size()) {
            move_to_end();
            return;
        }

        if (!_regex->search(*_string, &_regions, _offset)) {
            move_to_end();
            return;
        }

        _value = boost::make_iterator_range(
            _string->begin() + _regions.begin(0),
            _string->begin() + _regions.end(0)
        );

        _offset = _regions.end(0);
    }

    bool regex_iterator::equal(regex_iterator const& other) const
    {
        return _regex == other._regex && _string == other._string && _offset == other._offset;
    }

    regex_iterator::reference regex_iterator::dereference() const
    {
        return _value;
    }

    void regex_iterator::move_to_end()
    {
        _regex = nullptr;
        _string = nullptr;
        _offset = 0;
    }

    regex_split_iterator::regex_split_iterator()
    {
        move_to_end();
    }

    regex_split_iterator::regex_split_iterator(utility::regex const& regex, string const& str) :
        _regex(&regex),
        _string(&str),
        _offset(0),
        _region(numeric_limits<size_t>::max())
    {
        increment();
    }

    void regex_split_iterator::increment()
    {
        if (!_regex || !_string || _offset >= _string->size()) {
            move_to_end();
            return;
        }

        // Check to see if there are remaining capture groups to return
        if (_region < _regions.count()) {
            _value = boost::make_iterator_range(
                _string->begin() + _regions.begin(_region),
                _string->begin() + _regions.end(_region)
            );
            ++_region;
            return;
        }

        // If we're done with regions, move to the end of the last match
        if (_regions.count() > 0) {
            _offset = _regions.end(0);

            if (_offset >= _string->size()) {
                move_to_end();
                return;
            }
        }

        _region = numeric_limits<size_t>::max();

        if (!_regex->search(*_string, &_regions, _offset)) {
            _value = boost::make_iterator_range(_string->begin() + _offset, _string->end());
            _offset = _string->size();
            return;
        }

        // The current value is everything up to the current match
        _value = boost::make_iterator_range(
            _string->begin() + _offset,
            _string->begin() + _regions.begin(0)
        );

        // Region with index 0 is the match itself, which we don't return, so start at 1
        _region = 1;
    }

    bool regex_split_iterator::equal(regex_split_iterator const& other) const
    {
        return _regex == other._regex && _string == other._string && _offset == other._offset && _region == other._region;
    }

    regex_split_iterator::reference regex_split_iterator::dereference() const
    {
        return _value;
    }

    void regex_split_iterator::move_to_end()
    {
        _regex = nullptr;
        _string = nullptr;
        _offset = 0;
        _region = numeric_limits<size_t>::max();
    }

}}  // puppet::utility
