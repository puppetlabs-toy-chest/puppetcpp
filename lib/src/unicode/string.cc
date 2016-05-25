#include <puppet/unicode/string.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <unicode/utf8.h>
#include <unicode/ucol.h>
#include <unicode/ucasemap.h>
#include <unicode/utypes.h>

using namespace std;

namespace puppet { namespace unicode {

    struct posix_locale_helper
    {
        posix_locale_helper()
        {
            // If the default locale is POSIX (a.k.a. the "C" locale), switch to "en_US"
            // The POSIX locale will use a collator that is always case-sensitive (i.e. identical strength)
            // By using the en_US locale, it will fallback to the ICU root locale and use a secondary collation strength
            if (strcmp(uloc_getDefault(), "en_US_POSIX") == 0) {
                UErrorCode status = U_ZERO_ERROR;
                uloc_setDefault("en_US", &status);
                if (U_FAILURE(status)) {
                    throw unicode_exception(
                        (boost::format("failed to override POSIX ICU locale: %1%.") %
                         u_errorName(status)
                        ).str()
                    );
                }
            }
        }
    };

    static posix_locale_helper helper;

    struct collator
    {
        explicit collator(bool ignore_case)
        {
            UErrorCode status = U_ZERO_ERROR;
            _collator = ucol_open(nullptr, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to open ICU collator: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }

            if (ignore_case) {
                // Set the collation strength to secondary (ignore case differences)
                status = U_ZERO_ERROR;
                ucol_setAttribute(_collator, UCOL_STRENGTH, UCOL_SECONDARY, &status);
                if (U_FAILURE(status)) {
                    throw unicode_exception(
                        (boost::format("failed to set ICU collator normalization: %1%.") %
                         u_errorName(status)
                        ).str()
                    );
                }
            }

            // Turn on normalization because we don't support per-string locale
            status = U_ZERO_ERROR;
            ucol_setAttribute(_collator, UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to set ICU collator normalization: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }
        }

        ~collator()
        {
            ucol_close(_collator);
        }

        int compare(char const* left, int64_t left_size, char const* right, int64_t right_size) const
        {
            UErrorCode status = U_ZERO_ERROR;
            auto result = ucol_strcollUTF8(_collator, left, left_size, right, right_size, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to collate strings with ICU: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }
            if (result == UCOL_EQUAL) {
                return 0;
            }
            return result == UCOL_LESS ? -1 : 1;
        }

        size_t hash(char const* data, int32_t length) const
        {
            // State used for iterating the string
            uint32_t state[] = { 0, 0 };
            UCharIterator iterator;
            uiter_setUTF8(&iterator, data, length);

            // Buffer used to temporarily hold the sort key bytes
            uint8_t buffer[100];

            size_t seed = 0;
            while (true) {
                // Get the next part of the sort key bytes
                UErrorCode status = U_ZERO_ERROR;
                auto count = ucol_nextSortKeyPart(_collator, &iterator, state, buffer, sizeof(buffer), &status);
                if (U_FAILURE(status)) {
                    throw unicode_exception(
                        (boost::format("failed to get ICU sort key bytes: %1%.") %
                         u_errorName(status)
                        ).str()
                    );
                }
                // Hash the bytes
                if (count > 0) {
                    boost::hash_range(seed, buffer, buffer + count);
                }
                // Break out if the entire buffer wasn't filled (indicates we're done)
                if (count < static_cast<int>(sizeof(buffer))) {
                    break;
                }
            }
            return seed;
        }

        static collator const& insensitive()
        {
            static collator instance{ true };
            return instance;
        }

        static collator const& sensitive()
        {
            static collator instance{ false };
            return instance;
        }

     private:
         UCollator* _collator = nullptr;
    };

    struct case_map
    {
        case_map()
        {
            UErrorCode status = U_ZERO_ERROR;
            _map = ucasemap_open(nullptr, 0, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to open ICU case map: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }
        }

        ~case_map()
        {
            ucasemap_close(_map);
        }

        std::string to_upper(char const* string, int32_t length) const
        {
            return change_case(string, length, true);
        }

        std::string to_lower(char const* string, int32_t length) const
        {
            return change_case(string, length, false);
        }

        std::string capitalize(char const* string, int32_t length) const
        {
            if (length == 0) {
                return {};
            }

            string_iterator end;
            string_iterator it{ string, static_cast<size_t>(length) };
            if (it == end) {
                return {};
            }
            auto first_size = it->size();
            auto result = to_upper(it->begin(), it->size());
            ++it;
            if (it == end) {
                return result;
            }
            result += to_lower(it->begin(), length - first_size);
            return result;
        }

        static case_map const& instance()
        {
            static case_map instance{};
            return instance;
        }

     private:
        std::string change_case(char const* string, int32_t length, bool upper) const
        {
            if (length == 0) {
                return {};
            }

            int32_t (*func)(UCaseMap const*, char*, int32_t, char const*, int32_t, UErrorCode*) =
                upper ? ucasemap_utf8ToUpper : ucasemap_utf8ToLower;

            std::string buffer;
            buffer.resize(length);

            while (true) {
                UErrorCode status = U_ZERO_ERROR;
                int32_t new_length = func(_map, &buffer[0], buffer.size(), string, length, &status);
                if (U_FAILURE(status)) {
                    if (status == U_BUFFER_OVERFLOW_ERROR) {
                        buffer.resize(new_length);
                        continue;
                    }
                    throw unicode_exception(
                        (boost::format("failed to %1% string with ICU: %2%.") %
                         (upper ? "upcase" : "downcase") %
                         u_errorName(status)
                        ).str()
                    );
                }
                buffer.resize(new_length);
                break;
            }
            return buffer;
        }

        UCaseMap* _map = nullptr;
    };

    string_iterator::string_iterator(std::string const& string, bool iterate_units, bool reversed) :
        string_iterator(string.data(), string.size(), iterate_units, reversed)
    {
    }

    string_iterator::string_iterator(char const* data, size_t length, bool iterate_units, bool reversed) :
        _data(data),
        _length(length),
        _iterate_units(iterate_units),
        _reversed(reversed)
    {
        // Initially set the value to an empty range at the start
        char const* start = _data + (_reversed ? _length : 0);
        _value = boost::make_iterator_range(start, start);

        // If iterating over Unicode graphemes, initialize the ICU data structures
        if (_length > 0 && !_iterate_units) {
            // Open the UTF8 text
        	UErrorCode status = U_ZERO_ERROR;
            utext_openUTF8(&_text, _data, _length, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to open ICU utext: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }

            // Create an empty iterator
            status = U_ZERO_ERROR;
            _iterator = ubrk_open(UBRK_CHARACTER, nullptr, nullptr, 0, &status);
            if (U_FAILURE(status)) {
                utext_close(&_text);
                throw unicode_exception(
                    (boost::format("failed to create ICU break iterator: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }

            // Set the iterator's text
            status = U_ZERO_ERROR;
            ubrk_setUText(_iterator, &_text, &status);
            if (U_FAILURE(status)) {
                ubrk_close(_iterator);
                utext_close(&_text);
                throw unicode_exception(
                    (boost::format("failed to set ICU break iterator text: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }

            // If reverse, move to the end
            if (_reversed) {
                ubrk_last(_iterator);
            }
        }

        // Increment the iterator to the starting value
        increment();
    }

    string_iterator::string_iterator(string_iterator const& other)
    {
        *this = other;
    }

    string_iterator::string_iterator(string_iterator&& other) noexcept
    {
        *this = rvalue_cast(other);
    }

    string_iterator& string_iterator::operator=(string_iterator const& other)
    {
        close();

        _data = other._data;
        _length = other._length;
        _value = other._value;
        _iterate_units = other._iterate_units;
        _reversed = other._reversed;

        // Clone the ICU data
        if (_data && _length > 0 && !_iterate_units) {
            UErrorCode status = U_ZERO_ERROR;
            utext_clone(&_text, &other._text, false, true, &status);
            if (U_FAILURE(status)) {
                throw unicode_exception(
                    (boost::format("failed to clone ICU utext: %1%.") %
                     u_errorName(status)
                    ).str()
                );
            }
            if (other._iterator) {
                status = U_ZERO_ERROR;
                _iterator = ubrk_safeClone(other._iterator, nullptr, 0, &status);
                if (U_FAILURE(status)) {
                    utext_close(&_text);
                    throw unicode_exception(
                        (boost::format("failed to clone ICU utext: %1%.") %
                         u_errorName(status)
                        ).str()
                    );
                }
            }
        }
        return *this;
    }

    string_iterator& string_iterator::operator=(string_iterator&& other) noexcept
    {
        _data = other._data;
        _length = other._length;
        _text = other._text;
        _iterator = other._iterator;
        _value = other._value;
        _iterate_units = other._iterate_units;
        _reversed = other._reversed;

        other.close(false /* don't release ICU objects */);
        return *this;
    }

    string_iterator::~string_iterator()
    {
        close();
    }

    void string_iterator::increment()
    {
        if (!_data) {
            return;
        }

        // Check if the iterator is at the end
        if (_length == 0 ||
            (_reversed && (_value.begin() <= _data)) ||
            (!_reversed && (_value.end() >= _data + _length))) {
            close();
            return;
        }

        if (_iterate_units) {
            if (_reversed) {
                _value = boost::make_iterator_range(_value.begin() - 1, _value.begin());
            } else {
                _value = boost::make_iterator_range(_value.end(), _value.end() + 1);
            }
        } else {
            auto pos = _reversed ? ubrk_previous(_iterator) : ubrk_next(_iterator);
            if (pos == UBRK_DONE) {
                pos = _reversed ? 0 : _length;
            }
            if (_reversed) {
                _value = boost::make_iterator_range(_data + pos, _value.begin());
            } else {
                _value = boost::make_iterator_range(_value.end(), _data + pos);
            }
        }
    }

    bool string_iterator::equal(string_iterator const& other) const
    {
        return _value == other._value;
    }

    string_iterator::reference string_iterator::dereference() const
    {
        return _value;
    }

    void string_iterator::close(bool release)
    {
        if (release) {
            if (_iterator) {
                ubrk_close(_iterator);
            }

            // This is a no-op if the text was never opened
            utext_close(&_text);
        }

        _data = nullptr;
        _length = 0;
        _text = UTEXT_INITIALIZER;
        _iterator = nullptr;
        _value = { nullptr, nullptr };
        _iterate_units = false;
        _reversed = false;
    }

    split_iterator::split_iterator(unicode::string const& string, char const* delimiter, size_t delimiter_length, bool ignore_case) :
        _string(&string),
        _delimiter(delimiter),
        _delimiter_length(delimiter_length),
        _start(string.data()),
        _ignore_case(ignore_case)
    {
        increment();
    }

    void split_iterator::increment()
    {
        if (!_string || !_delimiter || _delimiter_length == 0 || _start >= _string->eos()) {
            close();
            return;
        }

        auto range = _string->find(_start, _delimiter, _delimiter_length, _ignore_case);
        _value = { _start, range.begin() };
        _start = range.end();
    }

    bool split_iterator::equal(split_iterator const& other) const
    {
        return _string == other._string && _delimiter == other._delimiter &&
               _delimiter_length == other._delimiter_length && _start == other._start;
    }

    split_iterator::reference split_iterator::dereference() const
    {
        return _value;
    }

    void split_iterator::close()
    {
        _string = nullptr;
        _delimiter = nullptr;
        _delimiter_length = 0;
        _start = nullptr;
        _value = { nullptr, nullptr };
    }

    size_t const string::npos = std::numeric_limits<size_t>::max();

    string::string(std::string const& s) :
        _data(s.data()),
        _graphemes(0),
        _units(s.size())
    {
        count_graphemes();
    }

    string::string(char const* s) :
        _data(s),
        _graphemes(0),
        _units(npos)
    {
        count_graphemes();
    }

    string::const_iterator string::begin() const
    {
        return string_iterator{ _data, _units, invariant() };
    }

    string::const_iterator string::end() const
    {
        return string_iterator{};
    }

    string::const_iterator string::cbegin() const
    {
        return begin();
    }

    string::const_iterator string::cend() const
    {
        return end();
    }

    string::const_reverse_iterator string::rbegin() const
    {
        return string_iterator{ _data, _units, invariant(), true };
    }

    string::const_reverse_iterator string::rend() const
    {
        return end();
    }

    string::const_reverse_iterator string::crbegin() const
    {
        return rbegin();
    }

    string::const_reverse_iterator string::crend() const
    {
        return rend();
    }

    size_t string::graphemes() const noexcept
    {
        return _graphemes;
    }

    size_t string::units() const noexcept
    {
        return _units;
    }

    bool string::empty() const noexcept
    {
        return _units == 0;
    }

    bool string::invariant() const noexcept
    {
        return _units == _graphemes;
    }

    char const* string::data() const noexcept
    {
        return _data;
    }

    std::string string::substr(size_t start, size_t length) const
    {
        if (invariant()) {
            auto begin = _data + start;
            auto end = _data + start + length;
            auto eos = this->eos();
            if (begin < _data || begin > eos) {
                begin = eos;
            }
            if (end < begin || end > eos) {
                end = eos;
            }
            return std::string{ begin, end };
        }

        string_iterator it{ _data, _units };
        string_iterator end;
        size_t i = 0;
        for (i = 0; i < start && it != end; ++i, ++it);
        if (it == end) {
            return std::string{};
        }

        char const* begin = it->begin();
        for (i = 0; i < length && it != end; ++i, ++it);
        if (it == end) {
            return std::string{ begin, static_cast<size_t>(eos() - begin) };
        }

        return std::string{ begin, it->begin() };
    }

    int string::compare(string const& other, bool ignore_case) const
    {
        return (ignore_case ? collator::insensitive() : collator::sensitive()).compare(data(), units(), other.data(), other.units());
    }

    int string::compare(std::string const& other, bool ignore_case) const
    {
        return (ignore_case ? collator::insensitive() : collator::sensitive()).compare(data(), units(), other.data(), other.size());
    }

    int string::compare(char const* other, bool ignore_case) const
    {
        return (ignore_case ? collator::insensitive() : collator::sensitive()).compare(data(), units(), other, -1);
    }

    bool string::starts_with(string const& other) const
    {
        return starts_with(other.data(), other.units());
    }

    bool string::starts_with(std::string const& other) const
    {
        return starts_with(other.data(), other.size());
    }

    bool string::starts_with(char const* other) const
    {
        if (!other) {
            return false;
        }
        return starts_with(other, strlen(other));
    }

    std::string string::lowercase() const
    {
        if (invariant()) {
            std::string s{ _data, _units };
            for (auto& c : s) {
                c = std::tolower(c);
            }
            return s;
        }
        return case_map::instance().to_lower(_data, _units);
    }

    std::string string::uppercase() const
    {
        if (invariant()) {
            std::string s{ _data, _units };
            for (auto& c : s) {
                c = std::toupper(c);
            }
            return s;
        }
        return case_map::instance().to_upper(_data, _units);
    }

    std::string string::capitalize() const
    {
        if (invariant()) {
            std::string s{ _data, _units };
            for (size_t i = 0; i < s.size(); ++i) {
                auto& c = s[i];
                c = i == 0 ? std::toupper(c) : std::tolower(c);
            }
            return s;
        }
        return case_map::instance().capitalize(_data, _units);
    }

    std::string string::capitalize_segments() const
    {
        if (_units == 0) {
            return std::string{};
        }

        std::string result;
        auto range = boost::make_iterator_range(_data, eos());
        boost::split_iterator<char const*> end;
        for (auto it = boost::make_split_iterator(range, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            // Check for a leading delimiter
            if (it->begin() == _data && it->end() == _data) {
                result += "::";
                continue;
            }
            if (*it) {
                if (invariant()) {
                    bool first = true;
                    for (auto c : *it) {
                        result += first ? std::toupper(c) : std::tolower(c);
                        first = false;
                    }
                } else {
                    result += case_map::instance().capitalize(&*it->begin(), it->size());
                }
            }
            // Append a delimiter if not at the end of the string
            if (it->end() != eos()) {
                result += "::";
            }
        }
        return result;
    }

    std::string string::trim_left() const
    {
        return trim(true, false);
    }

    std::string string::trim_right() const
    {
        return trim(false, true);
    }

    std::string string::trim() const
    {
        return trim(true, true);
    }

    bool string::contains_any(string const& graphemes) const
    {
        return contains_any(graphemes.data(), graphemes.units());
    }

    bool string::contains_any(std::string const& graphemes) const
    {
        return contains_any(graphemes.data(), graphemes.size());
    }

    bool string::contains_any(char const* graphemes) const
    {
        if (!graphemes) {
            return false;
        }
        return contains_any(graphemes, strlen(graphemes));
    }

    string::value_type string::find(string const& substring, bool ignore_case) const
    {
        return find(_data, substring.data(), substring.units(), ignore_case);
    }

    string::value_type string::find(std::string const& substring, bool ignore_case) const
    {
        return find(_data, substring.data(), substring.size(), ignore_case);
    }

    string::value_type string::find(char const* substring, bool ignore_case) const
    {
        if (!substring) {
            return boost::make_iterator_range(eos(), eos());
        }
        return find(_data, substring, strlen(substring), ignore_case);
    }

    split_iterator string::split_begin(string const& delimiter, bool ignore_case)
    {
        return split_iterator{ *this, delimiter.data(), delimiter.units(), ignore_case };
    }

    split_iterator string::split_begin(std::string const& delimiter, bool ignore_case)
    {
        return split_iterator{ *this, delimiter.data(), delimiter.size(), ignore_case };
    }

    split_iterator string::split_begin(char const* delimiter, bool ignore_case)
    {
        return split_iterator{ *this, delimiter, strlen(delimiter), ignore_case };
    }

    split_iterator string::split_end() const
    {
        return split_iterator{};
    }

    char const* string::eos() const noexcept
    {
        return _data + _units;
    }

    bool string::starts_with(char const* data, size_t length) const
    {
        if (!data) {
            return false;
        }
        if (length == 0) {
            // All strings start with the empty string
            return true;
        }
        // For invariant strings, do a memcmp because this operation is case-sensitive
        // If the other string is not invariant, then this string cannot possibly start with the other one
        if (invariant()) {
            return _units >= length && memcmp(_data, data, length) == 0;
        }

        // Otherwise, use string iterators to compare each grapheme
        auto other = string_iterator{ data, length };
        auto end = this->end();
        for (auto it = begin(); it != end && other != end; ++it, ++other) {
            if (collator::sensitive().compare(it->begin(), it->size(), other->begin(), other->size()) != 0) {
                return false;
            }
        }
        return other == end;
    }

    std::string string::trim(bool left, bool right) const
    {
        if (!left && !right) {
            return std::string{ _data, _units };
        }

        auto size = _units;
        size_t start = 0;
        bool finished_left = !left;
        bool finished_right = !right;

        while (size > 0 && start < _units && (!finished_left || !finished_right)) {
            // The string has been checked for UTF-8 validity, so do an unsafe increment and decrement
            if (left && !finished_left) {
                UChar32 code_point = 0;
                auto next = start;
                U8_NEXT_UNSAFE(_data, next, code_point);
                if (u_isUWhiteSpace(code_point)) {
                    start = next;
                } else {
                    finished_left = true;
                }
            }
            if (right && !finished_right) {
                UChar32 code_point = 0;
                auto prev = size;
                U8_PREV_UNSAFE(_data, prev, code_point);
                if (u_isUWhiteSpace(code_point)) {
                    size = prev;
                } else {
                    finished_right = true;
                }
            }
        }
        if (start >= _units) {
            return std::string{};
        }
        if (start + size > _units) {
            return std::string{ _data + start, _units - start };
        }
        return std::string{ _data + start, size - start };
    }

    bool string::contains_any(char const* data, size_t length) const
    {
        if (length == 0) {
            return false;
        }
        if (invariant()) {
            for (size_t i = 0; i < _units; ++i) {
                auto c = _data[i];
                for (size_t j = 0; j < length; ++j) {
                    if (c == data[j]) {
                        return true;
                    }
                }
            }
            return false;
        }

        auto end = string_iterator{};
        for (auto& grapheme : *this) {
            for (auto it = string_iterator{ data, length }; it != end; ++it) {
                if (collator::sensitive().compare(grapheme.begin(), grapheme.size(), it->begin(), it->size()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    string::value_type string::find(char const* start, char const* data, size_t length, bool ignore_case) const
    {
        auto eos = this->eos();

        if (length == 0) {
            return boost::make_iterator_range(eos, eos);
        }

        size_t remaining = _units - static_cast<size_t>(start - _data);

        if (invariant()) {
            for (size_t i = 0; i < remaining; ++i) {
                bool found = true;
                // Search for the substring
                for (size_t j = i, k = 0; j < remaining && k < length; ++j, ++k) {
                    char ours = start[j];
                    char theirs = data[k];
                    if (ignore_case) {
                        ours = std::tolower(ours);
                        theirs = std::tolower(theirs);
                    }
                    if (ours != theirs) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return boost::make_iterator_range(start + i, start + i + length);
                }
            }
            return boost::make_iterator_range(eos, eos);
        }

        // Otherwise, use string iterators to compare each grapheme
        auto end = this->end();
        for (auto it = string_iterator{ start, remaining }; it != end; ++it) {
            bool found = true;
            auto range_start = it->begin();
            auto other = string_iterator{ data, length };
            for (; it != end && other != end; ++other) {
                if ((ignore_case ? collator::insensitive() : collator::sensitive()).compare(it->begin(), it->size(), other->begin(), other->size()) != 0) {
                    found = false;
                    break;
                }
                ++it;
            }
            if (found) {
                if (it == end && other != end) {
                    return boost::make_iterator_range(eos, eos);
                }
                return boost::make_iterator_range(range_start, it == end ? eos : it->begin());
            }
        }
        return boost::make_iterator_range(eos, eos);
    }

    void string::count_graphemes()
    {
        // Iterate the string as code points to determine if the string contains only invariant code units
        // This also verifies that the string only contains valid UTF-8 data
        bool is_invariant = true;
        int32_t i = 0;
        int32_t length = _units == npos ? -1 : _units;
        while (length == -1 || i < length) {
            UChar32 code_point = 0;
            U8_NEXT(_data, i, length, code_point);
            if (code_point < 0) {
                throw unicode_exception("the string contains an invalid UTF-8 sequence.");
            }
            if (code_point > 0x7f) {
                is_invariant = false;
            }
            // Check for null terminated strings
            if (length == -1 && code_point == 0) {
                _units = i - 1;
                break;
            }
        }

        if (is_invariant) {
            _graphemes = _units;
        } else {
            string_iterator end;
            _graphemes = 0;
            for (string_iterator it{ _data, _units }; it != end; ++it, ++_graphemes);
        }
    }

    bool operator==(string const& left, string const& right)
    {
        // If either string is invariant, just use memcmp
        if (left.invariant() || right.invariant()) {
            return left.units() == right.units() && memcmp(left.data(), right.data(), left.units()) == 0;
        }
        return left.compare(right) == 0;
    }

    bool operator==(string const& left, std::string const& right)
    {
        // If the left is invariant, use memcmp
        if (left.invariant()) {
            return left.units() == right.size() && memcmp(left.data(), right.data(), left.units()) == 0;
        }
        return left.compare(right) == 0;
    }

    bool operator==(string const& left, char const* right)
    {
        // If the left is invariant, use strcmp
        if (left.invariant()) {
            return right && strcmp(left.data(), right) == 0;
        }
        return left.compare(right) == 0;
    }

    bool operator==(std::string const& left, string const& right)
    {
        // If the right is invariant, use memcmp
        if (right.invariant()) {
            return left.size() == right.units() && memcmp(left.data(), right.data(), right.units()) == 0;
        }
        return right.compare(left) == 0;
    }

    bool operator==(char const* left, string const& right)
    {
        // If the right is invariant, use strcmp
        if (right.invariant()) {
            return left && strcmp(left, right.data()) == 0;
        }
        return right.compare(left) == 0;
    }

    bool operator!=(string const& left, string const& right)
    {
        // If either string is invariant, just use memcmp
        if (left.invariant() || right.invariant()) {
            return left.units() != right.units() || memcmp(left.data(), right.data(), left.units()) != 0;
        }
        return left.compare(right) != 0;
    }

    bool operator!=(string const& left, std::string const& right)
    {
        // If the right is invariant, use memcmp
        if (left.invariant()) {
            return left.units() != right.size() || memcmp(left.data(), right.data(), left.units()) != 0;
        }
        return left.compare(right) != 0;
    }

    bool operator!=(string const& left, char const* right)
    {
        // If the left is invariant, use strcmp
        if (left.invariant()) {
            return !right || strcmp(left.data(), right) != 0;
        }
        return left.compare(right) != 0;
    }

    bool operator!=(std::string const& left, string const& right)
    {
        // If the right is invariant, use memcmp
        if (right.invariant()) {
            return left.size() != right.units() || memcmp(left.data(), right.data(), right.units()) != 0;
        }
        return right.compare(left) != 0;
    }

    bool operator!=(char const* left, string const& right)
    {
        // If the right is invariant, use strcmp
        if (right.invariant()) {
            return !left || strcmp(left, right.data()) != 0;
        }
        return right.compare(left) != 0;
    }

    bool operator<(string const& left, string const& right)
    {
        return left.compare(right) < 0;
    }

    bool operator<(string const& left, std::string const& right)
    {
        return left.compare(right) < 0;
    }

    bool operator<(string const& left, char const* right)
    {
        return left.compare(right) < 0;
    }

    bool operator<(std::string const& left, string const& right)
    {
        return right.compare(left) > 0;
    }

    bool operator<(char const* left, string const& right)
    {
        return right.compare(left) > 0;
    }

    bool operator<=(string const& left, string const& right)
    {
        return left.compare(right) <= 0;
    }

    bool operator<=(string const& left, std::string const& right)
    {
        return left.compare(right) <= 0;
    }

    bool operator<=(string const& left, char const* right)
    {
        return left.compare(right) <= 0;
    }

    bool operator<=(std::string const& left, string const& right)
    {
        return right.compare(left) >= 0;
    }

    bool operator<=(char const* left, string const& right)
    {
        return right.compare(left) >= 0;
    }

    bool operator>(string const& left, string const& right)
    {
        return left.compare(right) > 0;
    }

    bool operator>(string const& left, std::string const& right)
    {
        return left.compare(right) > 0;
    }

    bool operator>(string const& left, char const* right)
    {
        return left.compare(right) > 0;
    }

    bool operator>(std::string const& left, string const& right)
    {
        return right.compare(left) < 0;
    }

    bool operator>(char const* left, string const& right)
    {
        return right.compare(left) < 0;
    }

    bool operator>=(string const& left, string const& right)
    {
        return left.compare(right) >= 0;
    }

    bool operator>=(string const& left, std::string const& right)
    {
        return left.compare(right) >= 0;
    }

    bool operator>=(string const& left, char const* right)
    {
        return left.compare(right) >= 0;
    }

    bool operator>=(std::string const& left, string const& right)
    {
        return right.compare(left) <= 0;
    }

    bool operator>=(char const* left, string const& right)
    {
        return right.compare(left) <= 0;
    }

    ostream& operator<<(ostream& os, unicode::string const& string)
    {
        os << string.data();
        return os;
    }

     size_t hash_value(unicode::string const& string)
     {
         // If it's an invariant string, don't bother with the collator
         if (string.invariant()) {
             return boost::hash_range(string.data(), string.data() + string.units());
         }
         return collator::sensitive().hash(string.data(), string.units());
     }

}}  // namespace puppet::unicode
