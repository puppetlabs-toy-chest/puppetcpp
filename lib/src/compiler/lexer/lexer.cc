#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace boost::spirit;

namespace puppet { namespace compiler { namespace lexer {

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

    tuple<string, size_t> get_text_and_column(ifstream& fs, size_t position, size_t tab_width)
    {
        const size_t READ_SIZE = 4096;
        char buf[READ_SIZE];

        scoped_file_position guard(fs);

        // Read backwards in chunks looking for the closest newline before the given position
        size_t start;
        for (start = (position > (READ_SIZE + 1) ? position - READ_SIZE - 1 : 0); fs; start -= (start < READ_SIZE ? start : READ_SIZE)) {
            if (!fs.seekg(start)) {
                return make_tuple("", 1);
            }

            // Read data into the buffer
            if (!fs.read(buf, position < READ_SIZE ? position : READ_SIZE)) {
                return make_tuple("", 1);
            }

            // Find the last newline in the buffer
            auto it = find(reverse_iterator<char*>(buf + fs.gcount()), reverse_iterator<char*>(buf), '\n');
            if (it != reverse_iterator<char*>(buf)) {
                start += distance(buf, it.base());
                break;
            }

            if (start == 0) {
                break;
            }
        }

        // Calculate the column
        size_t column = (position - start) + 1;

        // Find the end of the current line
        size_t end = position;
        fs.seekg(end);
        auto eof =  istreambuf_iterator<char>();
        for (auto it = istreambuf_iterator<char>(fs.rdbuf()); it != eof; ++it) {
            if (*it == '\n') {
                break;
            }
            ++end;
        }

        // Read the line's text
        size_t size = end - start;
        fs.seekg(start);
        vector<char> buffer(size);
        if (!fs.read(buffer.data(), buffer.size())) {
            return make_tuple("", 1);
        }

        // Convert tabs to spaces
        string text = string(buffer.data(), buffer.size());
        if (tab_width > 1) {
            column += count(text.begin(), text.begin() + column, '\t') * (tab_width - 1);
        }

        return make_tuple(rvalue_cast(text), column);
    }

    tuple<string, size_t> get_text_and_column(string const& input, size_t position, size_t tab_width)
    {
        auto start = input.rfind('\n', position);
        if (start == string::npos) {
            start = 0;
        } else {
            ++start;
        }

        string text = input.substr(start, input.find('\n', start));

        // Convert tabs to spaces
        size_t column = (position - start) + 1;
        if (tab_width > 1) {
            column += count(text.begin(), text.begin() + column, '\t') * (tab_width - 1);
        }

        return make_tuple(rvalue_cast(text), column);
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
