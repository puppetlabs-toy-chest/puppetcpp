#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/unicode/string.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace boost::spirit;

namespace puppet { namespace compiler { namespace lexer {

    char const* const EPP_STATE = "EPP";
    char const* const SQ_ESCAPES = R"(\')";
    char const* const DQ_ESCAPES = "\\\"'nrtsu$\n";

    lexer_istreambuf_iterator lex_begin(ifstream& file)
    {
        return lexer_istreambuf_iterator(typename lexer_istreambuf_iterator::base_type(make_default_multi_pass(istreambuf_iterator<char>(file))));
    }

    lexer_istreambuf_iterator lex_end(ifstream& file)
    {
        return lexer_istreambuf_iterator(typename lexer_istreambuf_iterator::base_type(make_default_multi_pass(istreambuf_iterator<char>())));
    }

    lexer_string_iterator lex_begin(string const& str)
    {
        return lexer_string_iterator(typename lexer_string_iterator::base_type(str.begin()));
    }

    lexer_string_iterator lex_end(string const& str)
    {
        return lexer_string_iterator(typename lexer_string_iterator::base_type(str.end()));
    }

    lexer_string_iterator lex_begin(boost::iterator_range<lexer_string_iterator> const& range)
    {
        return range.begin();
    }

    lexer_string_iterator lex_end(boost::iterator_range<lexer_string_iterator> const& range)
    {
        return range.end();
    }

    struct scoped_file_position
    {
        scoped_file_position(ifstream& fs) :
            _fs(fs),
            _position(fs.tellg())
        {
        }

        ~scoped_file_position()
        {
            _fs.seekg(_position);
        }

     private:
        ifstream& _fs;
        size_t _position;
    };

    line_info get_line_info(ifstream& input, size_t position, size_t length, size_t tab_width)
    {
        const size_t READ_SIZE = 4096;
        char buf[READ_SIZE];

        scoped_file_position guard(input);

        // Read backwards in chunks looking for the closest newline before the given position
        size_t start;
        for (start = (position > (READ_SIZE + 1) ? position - READ_SIZE - 1 : 0); input; start -= (start < READ_SIZE ? start : READ_SIZE)) {
            if (!input.seekg(start)) {
                return line_info{};
            }

            // Read data into the buffer
            if (!input.read(buf, position < READ_SIZE ? position : READ_SIZE)) {
                return line_info{};
            }

            // Find the last newline in the buffer
            auto it = find(reverse_iterator<char*>(buf + input.gcount()), reverse_iterator<char*>(buf), '\n');
            if (it != reverse_iterator<char*>(buf)) {
                start += distance(buf, it.base());
                break;
            }

            if (start == 0) {
                break;
            }
        }

        // Find the end of the current line
        size_t end = position;
        input.seekg(end);
        auto eof =  istreambuf_iterator<char>();
        for (auto it = istreambuf_iterator<char>(input.rdbuf()); it != eof; ++it) {
            if (*it == '\n') {
                break;
            }
            ++end;
        }

        // Read the line's text
        size_t size = end - start;
        input.seekg(start);
        vector<char> buffer(size);
        if (!input.read(buffer.data(), buffer.size())) {
            return line_info{};
        }

        line_info info;

        // Use a unicode string to count graphemes
        info.text.assign(buffer.data(), buffer.size());
        unicode::string unicode_text{ info.text };

        // The column is 1-based, so start at 1
        info.column = 1;

        auto highlight_start = info.text.data() + position - start;
        auto highlight_end = highlight_start + length;
        for (auto& grapheme : unicode_text) {
            // If the grapheme comes before the highlight, increment the column count
            if (grapheme.begin() < highlight_start) {
                info.column += grapheme.size() == 1 && *grapheme.begin() == '\t' ? tab_width : 1;
                continue;
            }
            // If the grapheme is part of the highlight, increment the length
            if (grapheme.begin() >= highlight_start && grapheme.end() <= highlight_end) {
                ++info.length;
                continue;
            }
            break;
        }
        return info;
    }

    line_info get_line_info(std::string const& input, size_t position, size_t length, size_t tab_width)
    {
        // Truncate to the end if needed
        if (position > input.size()) {
            position = input.size() - 1;
        }

        // Find the starting newline by walking backwards from the given position
        auto start = input.rfind('\n', position == 0 ? 0 : position - 1);
        if (start == string::npos) {
            start = 0;
        } else {
            ++start;
        }

        // Find the ending newline by walking forward from the start
        auto end = input.find('\n', start);

        line_info info;

        // Use a unicode string to count graphemes
        info.text = input.substr(start, end == string::npos ? end : end - start);
        unicode::string unicode_text{ info.text };

        // The column is 1-based, so start at 1
        info.column = 1;

        auto highlight_start = info.text.data() + position - start;
        auto highlight_end = highlight_start + length;
        for (auto& grapheme : unicode_text) {
            // If the grapheme comes before the highlight, increment the column count
            if (grapheme.begin() < highlight_start) {
                info.column += grapheme.size() == 1 && *grapheme.begin() == '\t' ? tab_width : 1;
                continue;
            }
            // If the grapheme is part of the highlight, increment the length
            if (grapheme.begin() >= highlight_start && grapheme.end() <= highlight_end) {
                ++info.length;
                continue;
            }
            break;
        }
        return info;
    }

    position get_last_position(ifstream& input)
    {
        // We need to read the entire file looking for new lines
        scoped_file_position guard(input);

        input.seekg(0);

        std::size_t offset = 0, line = 1;
        std::size_t current_offset = 0, current_line = 1;
        for (std::istreambuf_iterator<char> it(input), end; it != end; ++it) {
            if (*it == '\n') {
                ++current_line;
            }
            ++current_offset;

            // If not whitespace, point the last position to that location
            if (!isspace(*it)) {
                offset = current_offset;
                line = current_line;
            }
        }
        return position(offset, line);
    }

    position get_last_position(string const& input)
    {
        std::size_t offset = 0, line = 1;
        std::size_t current_offset = 0, current_line = 1;
        for (auto it = input.begin(); it != input.end(); ++it) {
            if (*it == '\n') {
                ++current_line;
            }
            ++current_offset;

            // If not whitespace, point the last position to that location
            if (!isspace(*it)) {
                offset = current_offset;
                line = current_line;
            }
        }
        return position(offset, line);
    }

    position get_last_position(boost::iterator_range<lexer_string_iterator> const& range)
    {
        // Get the last position in the range (end is non-inclusive)
        auto last = range.begin();
        for (auto it = range.begin(); it != range.end(); ++it) {
            last = it;
        }
        return last.position();
    }

}}}  // namespace puppet::compiler::lexer
