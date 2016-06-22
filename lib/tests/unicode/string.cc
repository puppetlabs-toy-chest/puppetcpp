#include <catch.hpp>
#include <puppet/unicode/string.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet;

SCENARIO("using a string iterator", "[values]")
{
    GIVEN("an empty string") {
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            unicode::string_iterator it{ std::string{} };
            THEN("it should iterate as empty") {
                REQUIRE(it == end);
            }
        }
        WHEN("iterating as code units") {
            unicode::string_iterator it{ std::string{}, true };
            THEN("it should iterate as empty") {
                REQUIRE(it == end);
            }
        }
        WHEN("iterating as graphemes in reverse") {
            unicode::string_iterator it{ std::string{}, false, true };
            THEN("it should iterate as empty") {
                REQUIRE(it == end);
            }
        }
        WHEN("iterating as code units in reverse") {
            unicode::string_iterator it{ std::string{}, true, true };
            THEN("it should iterate as empty") {
                REQUIRE(it == end);
            }
        }
    }
    GIVEN("a string containing only invariant graphemes") {
        string s = "hello world";
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            size_t i = 0;
            THEN("it should iterate over the invariant graphemes") {
                for (unicode::string_iterator it{ s }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the invariant graphemes") {
                for (unicode::string_iterator it{ s, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
        WHEN("iterating as graphemes in reverse") {
            size_t i = 0;
            THEN("it should iterate over the invariant graphemes") {
                for (unicode::string_iterator it{ s, false, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the invariant graphemes") {
                for (unicode::string_iterator it{ s, true, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
    }
    GIVEN("a string with tamil graphemes") {
        std::string s = u8"ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்";
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            std::vector<std::string> graphemes = {
                u8"ஸ்",
                u8"றீ",
                u8"னி",
                u8"வா",
                u8"ஸ",
                u8" ",
                u8"ரா",
                u8"மா",
                u8"னு",
                u8"ஜ",
                u8"ன்",
                u8" ",
                u8"ஐ",
                u8"ய",
                u8"ங்",
                u8"கா",
                u8"ர்"
            };
            size_t i = 0;
            THEN("it should iterate over combined Unicode graphemes (and not code units or points)") {
                for (unicode::string_iterator it{ s }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[i] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
            THEN("it should iterate over combined Unicode graphemes (and not code units or points) in reverse") {
                for (unicode::string_iterator it{ s, false, true }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[graphemes.size() - i - 1] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the code units") {
                for (unicode::string_iterator it{ s, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
            THEN("it should iterate over the code units in reverse") {
                for (unicode::string_iterator it{ s, true, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
    }
    GIVEN("a string with cyrillic graphemes") {
        std::string s = u8"На берегу пустынных волн";
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            std::vector<std::string> graphemes = {
                u8"Н",
                u8"а",
                u8" ",
                u8"б",
                u8"е",
                u8"р",
                u8"е",
                u8"г",
                u8"у",
                u8" ",
                u8"п",
                u8"у",
                u8"с",
                u8"т",
                u8"ы",
                u8"н",
                u8"н",
                u8"ы",
                u8"х",
                u8" ",
                u8"в",
                u8"о",
                u8"л",
                u8"н"
            };
            size_t i = 0;
            THEN("it should iterate over combined Unicode graphemes (and not code units or points)") {
                for (unicode::string_iterator it{ s }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[i] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
            THEN("it should iterate over combined Unicode graphemes (and not code units or points) in reverse") {
                for (unicode::string_iterator it{ s, false, true }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[graphemes.size() - i - 1] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the code units") {
                for (unicode::string_iterator it{ s, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
            THEN("it should iterate over the code units in reverse") {
                for (unicode::string_iterator it{ s, true, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
    }
    GIVEN("a string with hiragana graphemes") {
        std::string s = u8"私はガラスを食べられます。それは私を傷つけません。";
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            std::vector<std::string> graphemes = {
                u8"私",
                u8"は",
                u8"ガ",
                u8"ラ",
                u8"ス",
                u8"を",
                u8"食",
                u8"べ",
                u8"ら",
                u8"れ",
                u8"ま",
                u8"す",
                u8"。",
                u8"そ",
                u8"れ",
                u8"は",
                u8"私",
                u8"を",
                u8"傷",
                u8"つ",
                u8"け",
                u8"ま",
                u8"せ",
                u8"ん",
                u8"。"
            };
            size_t i = 0;
            THEN("it should iterate over combined Unicode graphemes (and not code units or points)") {
                for (unicode::string_iterator it{ s }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[i] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
            THEN("it should iterate over combined Unicode graphemes (and not code units or points) in reverse") {
                for (unicode::string_iterator it{ s, false, true }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[graphemes.size() - i - 1] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the code units") {
                for (unicode::string_iterator it{ s, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
            THEN("it should iterate over the code units in reverse") {
                for (unicode::string_iterator it{ s, true, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
    }
    GIVEN("a string with thai graphemes") {
        std::string s = u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ";
        unicode::string_iterator end;
        WHEN("iterating as graphemes") {
            std::vector<std::string> graphemes = {
                u8"ฉั",
                u8"น",
                u8"กิ",
                u8"น",
                u8"ก",
                u8"ร",
                u8"ะ",
                u8"จ",
                u8"ก",
                u8"ไ",
                u8"ด้",
                u8" ",
                u8"แ",
                u8"ต่",
                u8"มั",
                u8"น",
                u8"ไ",
                u8"ม่",
                u8"ทำ",
                u8"ใ",
                u8"ห้",
                u8"ฉั",
                u8"น",
                u8"เ",
                u8"จ็",
                u8"บ"
            };
            size_t i = 0;
            THEN("it should iterate over combined Unicode graphemes (and not code units or points)") {
                for (unicode::string_iterator it{ s }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[i] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
            THEN("it should iterate over combined Unicode graphemes (and not code units or points) in reverse") {
                for (unicode::string_iterator it{ s, false, true }; it != end; ++it, ++i) {
                    REQUIRE(i < graphemes.size());
                    REQUIRE(graphemes[graphemes.size() - i - 1] == *it);
                }
                REQUIRE(i == graphemes.size());
            }
        }
        WHEN("iterating as code units") {
            size_t i = 0;
            THEN("it should iterate over the code units") {
                for (unicode::string_iterator it{ s, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[i] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
            THEN("it should iterate over the code units in reverse") {
                for (unicode::string_iterator it{ s, true, true }; it != end; ++it, ++i) {
                    REQUIRE(it->size() == 1);
                    REQUIRE(i < s.size());
                    REQUIRE(s[s.size() - i - 1] == *it->begin());
                }
                REQUIRE(i == s.size());
            }
        }
    }
}

SCENARIO("using a string", "[values]")
{
    WHEN("constructing from an invalid UTF-8 string") {
        THEN("it throws an exception for an invalid UTF-8 code unit") {
            REQUIRE_THROWS_WITH(unicode::string{u8"invalid encoding: \xFF"}, "the string contains an invalid UTF-8 sequence.");
        }
        THEN("it throws an exception for an invalid code point") {
            REQUIRE_THROWS_WITH(unicode::string{u8"invalid codepoint: \xF7\x8F\xBF\xBF"}, "the string contains an invalid UTF-8 sequence.");
        }
    }
    WHEN("iterating a string") {
        unicode::string s{ "foobar" };
        size_t i = 0;
        THEN("each grapheme should be iterated") {
            for (auto it = s.begin(); it != s.end(); ++it, ++i) {
                REQUIRE(s.data()[i] == *it->begin());
            }
            REQUIRE(i == s.graphemes());
        }
        THEN("each grapheme should be iterated as constant") {
            for (auto it = s.cbegin(); it != s.cend(); ++it, ++i) {
                REQUIRE(s.data()[i] == *it->begin());
            }
            REQUIRE(i == s.graphemes());
        }
        THEN("each grapheme should be iterated in reverse") {
            for (auto it = s.rbegin(); it != s.rend(); ++it, ++i) {
                REQUIRE(s.data()[s.units() - i - 1] == *it->begin());
            }
            REQUIRE(i == s.graphemes());
        }
        THEN("each grapheme should be iterated in reverse as constant") {
            for (auto it = s.crbegin(); it != s.crend(); ++it, ++i) {
                REQUIRE(s.data()[s.units() - i - 1] == *it->begin());
            }
            REQUIRE(i == s.graphemes());
        }
    }
    WHEN("getting the string's size") {
        THEN("it should have the expected number of graphemes for an invariant string") {
            REQUIRE(unicode::string{ "hello" }.graphemes() == 5);
        }
        THEN("it should have the expected numer of graphemes for a Unicode string") {
            std::string s = u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ";
            REQUIRE(unicode::string{ s }.graphemes() == 26);
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ" }.graphemes() == 26);
        }
    }
    WHEN("getting the string's code units count") {
        THEN("it should have the expected count for an invariant string") {
            REQUIRE(unicode::string{ "hello" }.units() == 5);
        }
        THEN("it should have the expected count for a Unicode string") {
            std::string s = u8"ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்";
            REQUIRE(unicode::string{ s }.units() == 80);
            REQUIRE(unicode::string{ u8"ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்" }.units() == 80);
        }
    }
    WHEN("determining if a string is empty") {
        THEN("an empty string should be empty") {
            REQUIRE(unicode::string{ "" }.empty());
        }
        THEN("it should not be empty for non-empty strings") {
            REQUIRE_FALSE(unicode::string{ "hello" }.empty());
            REQUIRE_FALSE(unicode::string{ u8"ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்" }.empty());
        }
    }
    WHEN("determining if a string contains only invariant data") {
        THEN("an empty string should be considered invariant") {
            REQUIRE(unicode::string{ "" }.invariant());
        }
        THEN("an invariant string should be considered invariant") {
            REQUIRE(unicode::string{ "hello" }.invariant());
        }
        THEN("a Unicode string should not be considered invariant") {
            REQUIRE_FALSE(unicode::string{ u8"ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்" }.invariant());
        }
    }
    WHEN("taking a substring") {
        THEN("it should behave like std::string for invariant strings") {
            REQUIRE(unicode::string{ "foobar" }.substr(0, 3) == "foo");
            REQUIRE(unicode::string{ "foobar" }.substr(1, 4) == "ooba");
            REQUIRE(unicode::string{ "foobar" }.substr(3) == "bar");
        }
        THEN("it should substring using graphemes for a Unicode string") {
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ" }.substr(0, 3) == u8"ฉันกิ");
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ" }.substr(1, 4) == u8"นกินก");
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ" }.substr(12) == u8"แต่มันไม่ทำให้ฉันเจ็บ");
        }
    }
    WHEN("comparing two strings") {
        THEN("it should respect case for equality") {
            REQUIRE(unicode::string{ "foobar"}.compare("FOOBAR") != 0 );
            REQUIRE(unicode::string{ "foobar"} != unicode::string{ "FOOBAR" });
            REQUIRE(unicode::string{ "foobar"} != std::string{ "FOOBAR" });
            REQUIRE(unicode::string{ "foobar"} != "FOOBAR");
            REQUIRE(std::string{ "FOOBAR" } != unicode::string{ "foobar"});
            REQUIRE("FOOBAR" != unicode::string{ "foobar"});
            REQUIRE(unicode::string{ u8"çöğiü"}.compare(u8"ÇÖĞIÜ") != 0 );
            REQUIRE(unicode::string{ u8"çöğiü"} != unicode::string{ u8"ÇÖĞIÜ" });
            REQUIRE(unicode::string{ u8"çöğiü"} != std::string{ u8"ÇÖĞIÜ" });
            REQUIRE(unicode::string{ u8"çöğiü"} != u8"ÇÖĞIÜ");
            REQUIRE(std::string{ u8"ÇÖĞIÜ" } != unicode::string{ u8"çöğiü"});
            REQUIRE(u8"ÇÖĞIÜ" != unicode::string{ u8"çöğiü"});
        }
        THEN("it should ignore case for equality when requested") {
            REQUIRE(unicode::string{ "foobar"}.compare(unicode::string{ "FOOBAR" }, true) == 0);
            REQUIRE(unicode::string{ "foobar"}.compare(std::string{ "FOOBAR" }, true) == 0);
            REQUIRE(unicode::string{ "foobar"}.compare("FOOBAR", true) == 0);
            REQUIRE(unicode::string{ u8"çöğiü"}.compare(unicode::string{ u8"ÇÖĞIÜ" }, true) == 0);
            REQUIRE(unicode::string{ u8"çöğiü"}.compare(std::string{ u8"ÇÖĞIÜ" }, true) == 0);
            REQUIRE(unicode::string{ u8"çöğiü"}.compare(u8"ÇÖĞIÜ", true) == 0);
        }
        THEN("two different strings should not be equal") {
            REQUIRE(unicode::string{ "foo"} != unicode::string{ "bar" });
            REQUIRE(unicode::string{ "foo"} != std::string{ "bar" });
            REQUIRE(unicode::string{ "foo"} != "bar");
            REQUIRE(std::string{ "bar" } != unicode::string{ "foo"});
            REQUIRE("bar" != unicode::string{ "foo"});
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได"} != unicode::string{ u8"ராமானுஜன்" });
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได"} != std::string{ u8"ராமானுஜன்" });
            REQUIRE(unicode::string{ u8"ฉันกินกระจกได"} != u8"ராமானுஜன்");
            REQUIRE(std::string{ u8"ராமானுஜன்" } != unicode::string{ u8"ฉันกินกระจกได"});
            REQUIRE(u8"ராமானுஜன்" != unicode::string{ u8"ฉันกินกระจกได"});
        }
        THEN("it should normalize the strings for comparisons") {
            REQUIRE(unicode::string{ u8"this contains a ñ: \u00f1o" } == u8"this contains a ñ: n\u0303o");
        }
        THEN("it should compare for 'less than' correctly") {
            REQUIRE(unicode::string{ "a" }.compare("z") < 0);
            REQUIRE(unicode::string{ "a" } < unicode::string{ "z" });
            REQUIRE_FALSE(unicode::string{ "a" } < unicode::string{ "a" });
            REQUIRE(unicode::string{ "a" } < std::string{ "z" });
            REQUIRE_FALSE(unicode::string{ "a" } < std::string{ "a" });
            REQUIRE(unicode::string{ "a" } < "z" );
            REQUIRE_FALSE(unicode::string{ "a" } < "a" );
            REQUIRE(std::string{ "a" } < unicode::string{ "z" });
            REQUIRE_FALSE(std::string{ "a" } < unicode::string{ "a" });
            REQUIRE("a" < unicode::string{ "z" });
            REQUIRE_FALSE("a" < unicode::string{ "a" });
        }
        THEN("it should compare for 'less than or equal to' correctly") {
            REQUIRE(unicode::string{ "a" }.compare("z") <= 0);
            REQUIRE(unicode::string{ "a" } <= unicode::string{ "z" });
            REQUIRE(unicode::string{ "a" } <= unicode::string{ "a" });
            REQUIRE(unicode::string{ "a" } <= std::string{ "z" });
            REQUIRE(unicode::string{ "a" } <= std::string{ "a" });
            REQUIRE(unicode::string{ "a" } <= "z" );
            REQUIRE(unicode::string{ "a" } <= "a" );
            REQUIRE(std::string{ "a" } <= unicode::string{ "z" });
            REQUIRE(std::string{ "a" } <= unicode::string{ "a" });
            REQUIRE("a" <= unicode::string{ "z" });
            REQUIRE("a" <= unicode::string{ "a" });
        }
        THEN("it should compare for 'greater than' correctly") {
            REQUIRE(unicode::string{ "z" }.compare("a") > 0);
            REQUIRE(unicode::string{ "z" } > unicode::string{ "a" });
            REQUIRE_FALSE(unicode::string{ "z" } > unicode::string{ "z" });
            REQUIRE(unicode::string{ "z" } > std::string{ "a" });
            REQUIRE_FALSE(unicode::string{ "z" } > std::string{ "z" });
            REQUIRE(unicode::string{ "z" } > "a" );
            REQUIRE_FALSE(unicode::string{ "z" } > "z" );
            REQUIRE(std::string{ "z" } > unicode::string{ "a" });
            REQUIRE_FALSE(std::string{ "z" } > unicode::string{ "z" });
            REQUIRE("z" > unicode::string{ "a" });
            REQUIRE_FALSE("z" > unicode::string{ "z" });
        }
        THEN("it should compare for 'less than or equal to' correctly") {
            REQUIRE(unicode::string{ "z" }.compare("a") >= 0);
            REQUIRE(unicode::string{ "z" } >= unicode::string{ "a" });
            REQUIRE(unicode::string{ "z" } >= unicode::string{ "z" });
            REQUIRE(unicode::string{ "z" } >= std::string{ "a" });
            REQUIRE(unicode::string{ "z" } >= std::string{ "z" });
            REQUIRE(unicode::string{ "z" } >= "a" );
            REQUIRE(unicode::string{ "z" } >= "z" );
            REQUIRE(std::string{ "z" } >= unicode::string{ "a" });
            REQUIRE(std::string{ "z" } >= unicode::string{ "z" });
            REQUIRE("z" >= unicode::string{ "a" });
            REQUIRE("z" >= unicode::string{ "z" });
        }
    }
    WHEN("writing a string") {
        char const* data = u8"foo それは私を傷つけません。bar";
        ostringstream buffer;
        buffer << unicode::string{ data };
        THEN("the entire string should be written") {
            REQUIRE(buffer.str() == data);
        }
    }
    WHEN("hashing a string") {
        THEN("two equal strings should hash to the same value") {
            REQUIRE(hash_value(unicode::string{ "foobar" }) == hash_value(unicode::string{ "foobar" }));
            REQUIRE(hash_value(unicode::string{ u8"ฉันกินกระจกได้" }) == hash_value(unicode::string{ u8"ฉันกินกระจกได้" }));
        }
        THEN("two unequal strings should not hash to the same value") {
            REQUIRE_FALSE(hash_value(unicode::string{ "foo" }) == hash_value(unicode::string{ "bar" }));
            REQUIRE_FALSE(hash_value(unicode::string{ u8"ฉันกินกระจกได้" }) == hash_value(unicode::string{ u8"それは私を傷つ" }));
        }
        THEN("two equal strings with different case should not hash to the same value") {
            REQUIRE(hash_value(unicode::string{ u8"Τάχιστη" }) != hash_value(unicode::string{ u8"ΤΆΧΙΣΤΗ" }));
            REQUIRE(hash_value(unicode::string{ "fOO" }) != hash_value(unicode::string{ "foo" }));
        }
        THEN("it should handle normalization") {
            REQUIRE(hash_value(unicode::string{ u8"this contains a ñ: \u00f1o" }) == hash_value(unicode::string{ u8"this contains a ñ: n\u0303o" }));
        }
    }
    WHEN("checking the start of a string") {
        unicode::string invariant{ "foo bar baz" };
        unicode::string unicode{ u8"それは私を傷つ" };
        THEN("strings should start with an empty string") {
            REQUIRE(invariant.starts_with(unicode::string{ "" }));
            REQUIRE(invariant.starts_with(std::string{}));
            REQUIRE(invariant.starts_with(""));
            REQUIRE(unicode.starts_with(unicode::string{ "" }));
            REQUIRE(unicode.starts_with(std::string{}));
            REQUIRE(unicode.starts_with(""));
        }
        THEN("strings should start with a matching starting substring") {
            REQUIRE(invariant.starts_with(unicode::string{ "foo ba" }));
            REQUIRE(invariant.starts_with(std::string{ "f" }));
            REQUIRE(invariant.starts_with("foo"));
            REQUIRE(unicode.starts_with(unicode::string{ u8"それは私" }));
            REQUIRE(unicode.starts_with(std::string{ u8"そ" }));
            REQUIRE(unicode.starts_with(u8"それは"));
        }
        THEN("strings should start with the entire matching string") {
            REQUIRE(invariant.starts_with(unicode::string{ "foo bar baz" }));
            REQUIRE(invariant.starts_with(std::string{ "foo bar baz" }));
            REQUIRE(invariant.starts_with("foo bar baz"));
            REQUIRE(unicode.starts_with(unicode::string{ u8"それは私を傷つ" }));
            REQUIRE(unicode.starts_with(std::string{ u8"それは私を傷つ" }));
            REQUIRE(unicode.starts_with(u8"それは私を傷つ"));
        }
        THEN("strings should not start with a longer string") {
            REQUIRE_FALSE(invariant.starts_with(unicode::string{ "foo bar baz nope" }));
            REQUIRE_FALSE(invariant.starts_with(std::string{ "foo bar baz nope" }));
            REQUIRE_FALSE(invariant.starts_with("foo bar baz nope"));
            REQUIRE_FALSE(unicode.starts_with(unicode::string{ u8"それは私を傷つ。" }));
            REQUIRE_FALSE(unicode.starts_with(std::string{ u8"それは私を傷つ。" }));
            REQUIRE_FALSE(unicode.starts_with(u8"それは私を傷つ。"));
        }
        THEN("strings should not start with a mismatched string") {
            REQUIRE_FALSE(invariant.starts_with(unicode::string{ "bar" }));
            REQUIRE_FALSE(invariant.starts_with(std::string{ "baz" }));
            REQUIRE_FALSE(invariant.starts_with("nope"));
            REQUIRE_FALSE(unicode.starts_with(unicode::string{ u8"ฉันกิน" }));
            REQUIRE_FALSE(unicode.starts_with(std::string{ u8"ฉันกิ" }));
            REQUIRE_FALSE(unicode.starts_with(u8"ฉัน"));
        }
    }
    WHEN("uppercasing a string") {
        THEN("uppercasing an empty string results in an empty string") {
            unicode::string string{ "" };
            REQUIRE(string.uppercase() == "");
        }
        THEN("uppercasing an invariant string results in an uppercase string") {
            unicode::string string{ "foObArBaz" };
            REQUIRE(string.uppercase() == "FOOBARBAZ");
        }
        THEN("uppercasing a unicode string results in an uppercase string") {
            unicode::string string{ u8"Τάχιστη" };
            REQUIRE(string.uppercase() == u8"ΤΆΧΙΣΤΗ");
        }
        THEN("uppercasing a eszett results in a larger string") {
            unicode::string string{ u8"Maße" };
            REQUIRE(string.uppercase() == u8"MASSE");
        }
    }
    WHEN("lowercasing a string") {
        THEN("lowercasing an empty string results in an empty string") {
            unicode::string string{ "" };
            REQUIRE(string.empty());
            REQUIRE(string.lowercase() == "");
        }
        THEN("lowercasing an invariant string results in an lowercase string") {
            unicode::string string{ "foObArBaz" };
            REQUIRE(string.lowercase() == "foobarbaz");
        }
        THEN("lowercasing a unicode string results in an lowercase string") {
            unicode::string string{ u8"Τάχιστη" };
            REQUIRE(string.lowercase() == u8"τάχιστη");
        }
        THEN("lowercasing a eszett keeps the eszett") {
            unicode::string string{ u8"Maße" };
            REQUIRE(string.lowercase() == u8"maße");
        }
    }
    WHEN("capitalizing a string") {
        THEN("capitalizing an empty string results in an empty string") {
            unicode::string string{ "" };
            REQUIRE(string.empty());
            REQUIRE(string.capitalize() == "");
        }
        THEN("capitalizing an invariant string results in a capitalized string") {
            unicode::string string{ "hEllO WoRld" };
            REQUIRE(string.capitalize() == "Hello world");
        }
        THEN("capitalizing a unicode string results in an capitalized string") {
            unicode::string string{ u8"ΤΆΧΙΣΤΗ" };
            REQUIRE(string.capitalize() == u8"Τάχιστη");
        }
        THEN("capitalizing a eszett keeps the eszett") {
            unicode::string string{ u8"sTrAße" };
            REQUIRE(string.capitalize() == u8"Straße");
        }
    }
    WHEN("capitalizing segments of a string") {
        THEN("capitalizing the segments of an empty string results in an empty string") {
            unicode::string string{ "" };
            REQUIRE(string.empty());
            REQUIRE(string.capitalize_segments() == "");
        }
        THEN("capitalizing the segments of a string without any segments results in just a capitalized string") {
            unicode::string string{ "fOo" };
            REQUIRE(string.capitalize_segments() == "Foo");
            string = unicode::string{ u8"sTrAße" };
            REQUIRE(string.capitalize_segments() == u8"Straße");
        }
        THEN("capitalizing the segments of a string with a leading delimiter keeps the delimiter") {
            unicode::string string{ "::fOo::BaR::bAz" };
            string.capitalize_segments();
            REQUIRE(string.capitalize_segments() == "::Foo::Bar::Baz");
            string = unicode::string{ u8"::sTrAße::ΤΆΧΙΣΤΗ" };
            REQUIRE(string.capitalize_segments() == "::Straße::Τάχιστη");
        }
        THEN("capitalizing the segments of a string with a trailing delimiter keeps the delimiter") {
            unicode::string string{ "fOo::BaR::bAz::" };
            REQUIRE(string.capitalize_segments() == "Foo::Bar::Baz::");
            string = unicode::string{ u8"sTrAße::ΤΆΧΙΣΤΗ::" };
            REQUIRE(string.capitalize_segments() == "Straße::Τάχιστη::");
        }
        THEN("capitalizing the segments of a string results in only the first grapheme of each segment being capitalized") {
            unicode::string string{ "fOo::BaR::bAz" };
            REQUIRE(string.capitalize_segments() == "Foo::Bar::Baz");
            string = unicode::string{ u8"sTrAße::ΤΆΧΙΣΤΗ" };
            REQUIRE(string.capitalize_segments() == "Straße::Τάχιστη");
        }
        THEN("capitalizing the segments of a string containing an odd number delimiter doesn't capitalize that segment") {
            unicode::string string{ "fOo:::baR::bAz" };
            REQUIRE(string.capitalize_segments() == "Foo:::bar::Baz");
            string = unicode::string{ u8"sTrAße:::tΆΧΙΣΤΗ" };
            REQUIRE(string.capitalize_segments() == "Straße:::tάχιστη");
        }
        THEN("capitalizing the segments of a string containing an empty segment ignores the empty segment") {
            unicode::string string{ "fOo::::baR::bAz" };
            REQUIRE(string.capitalize_segments() == "Foo::::Bar::Baz");
            string = unicode::string{ u8"sTrAße::::tΆΧΙΣΤΗ" };
            REQUIRE(string.capitalize_segments() == "Straße::::Tάχιστη");
        }
    }
    WHEN("trimming a string from the left") {
        THEN("trimming left on an empty string results in an empty string") {
            unicode::string s{ "" };
            REQUIRE(s.trim_left().empty());
        }
        THEN("trimming left on a string that does not start in whitespace doesn't change the string") {
            unicode::string s{ u8"foo bar   "};
            REQUIRE(s.trim_left() == u8"foo bar   ");
        }
        THEN("trimming left on a string only containing whitespace results in an empty string") {
            unicode::string s{ u8"  \t\r\n \u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim_left().empty());
        }
        THEN("trimming left on a string stops at the first non-whitespace grapheme") {
            unicode::string s{ u8"  \t\r\n foo\u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim_left() == u8"foo\u00A0 \u2003 \u3000  ");
        }
    }
    WHEN("trimming a string from the right") {
        THEN("trimming right on an empty string results in an empty string") {
            unicode::string s{ "" };
            REQUIRE(s.trim_right().empty());
        }
        THEN("trimming right on a string that does not end in whitespace doesn't change the string") {
            unicode::string s{ u8"   foo bar"};
            REQUIRE(s.trim_right() == u8"   foo bar");
        }
        THEN("trimming right on a string only containing whitespace results in an empty string") {
            unicode::string s{ u8"  \t\r\n \u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim_right().empty());
        }
        THEN("trimming right on a string stops at the first non-whitespace grapheme") {
            unicode::string s{ u8"  \t\r\n foo\u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim_right() == u8"  \t\r\n foo");
        }
    }
    WHEN("trimming from both sides of a string") {
        THEN("trimming on an empty string results in an empty string") {
            unicode::string s{ "" };
            REQUIRE(s.trim().empty());
        }
        THEN("trimming on a string that does not start or end in whitespace doesn't change the string") {
            unicode::string s{ u8"foo bar"};
            REQUIRE(s.trim() == u8"foo bar");
        }
        THEN("trimming on a string only containing whitespace results in an empty string") {
            unicode::string s{ u8"  \t\r\n \u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim().empty());
        }
        THEN("trimming on a string stops at the first non-whitespace grapheme") {
            unicode::string s{ u8"  \t\r\n は私\u00A0 \u2003 \u3000  "};
            REQUIRE(s.trim() == u8"は私");
        }
    }
    WHEN("checking if a string contains graphemes") {
        unicode::string invariant{ "foo bar baz" };
        unicode::string unicode{ u8"それは私を傷つ" };
        unicode::string decomposed{ u8"n\u0303" };
        THEN("a check for containing an empty string always returns false") {
            unicode::string empty{ "" };
            REQUIRE_FALSE(empty.contains_any(unicode::string{ "" }));
            REQUIRE_FALSE(empty.contains_any(unicode::string{ "abc" }));
            REQUIRE_FALSE(empty.contains_any(std::string{ "" }));
            REQUIRE_FALSE(empty.contains_any(std::string{ "abc" }));
            REQUIRE_FALSE(empty.contains_any(""));
            REQUIRE_FALSE(empty.contains_any("abc"));
        }
        THEN("checking a string that does not contain the graphemes returns false") {
            REQUIRE_FALSE(invariant.contains_any(unicode::string{ "stu" }));
            REQUIRE_FALSE(invariant.contains_any(std::string{ "stu" }));
            REQUIRE_FALSE(invariant.contains_any("stu"));
            REQUIRE_FALSE(unicode.contains_any(unicode::string{ u8"χιστ" }));
            REQUIRE_FALSE(unicode.contains_any(std::string{ u8"χιστ" }));
            REQUIRE_FALSE(unicode.contains_any(u8"χιστ"));
        }
        THEN("checking a string that does contain the graphemes returns false") {
            REQUIRE(invariant.contains_any(unicode::string{ "xyz" }));
            REQUIRE(invariant.contains_any(std::string{ "xyz" }));
            REQUIRE(invariant.contains_any("xyz"));
            REQUIRE(unicode.contains_any(unicode::string{ u8"χιをστ" }));
            REQUIRE(unicode.contains_any(std::string{ u8"χιをστ" }));
            REQUIRE(unicode.contains_any(u8"χιをστ"));
        }
        THEN("checking if a string contains graphemes should handle normalization") {
            REQUIRE(decomposed.contains_any(unicode::string{ u8"\u00f1" }));
            REQUIRE(decomposed.contains_any(std::string{ u8"\u00f1" }));
            REQUIRE(decomposed.contains_any(u8"\u00f1"));
        }
    }
    WHEN("finding a substring") {
        unicode::string invariant{ "foo bar baz" };
        unicode::string unicode{ u8"それは私を傷つ tΆΧΙΣΤΗ" };
        unicode::string decomposed{ u8"no\u0303!" };

        auto invariant_eos = boost::make_iterator_range(invariant.eos(), invariant.eos());
        auto unicode_eos = boost::make_iterator_range(unicode.eos(), unicode.eos());
        auto decomposed_eos = boost::make_iterator_range(decomposed.eos(), decomposed.eos());

        THEN("finding an empty substring should always return npos") {
            REQUIRE(invariant.find(unicode::string{""}) == invariant_eos);
            REQUIRE(invariant.find(std::string{""}) == invariant_eos);
            REQUIRE(invariant.find("") == invariant_eos);
            REQUIRE(unicode.find(unicode::string{""}) == unicode_eos);
            REQUIRE(unicode.find(std::string{""}) == unicode_eos);
            REQUIRE(unicode.find("") == unicode_eos);
        }
        THEN("finding a substring returns the expected range") {
            REQUIRE(invariant.find(unicode::string{"bar"}) == boost::make_iterator_range(invariant.data() + 4, invariant.data() + 7));
            REQUIRE(invariant.find(std::string{"baz"}) == boost::make_iterator_range(invariant.data() + 8, invariant.data() + 11));
            REQUIRE(invariant.find("foo") == boost::make_iterator_range(invariant.data(), invariant.data() + 3));
            REQUIRE(unicode.find(unicode::string{u8"私"}) == boost::make_iterator_range(unicode.data() + 9, unicode.data() + 12));
            REQUIRE(unicode.find(std::string{u8"私を傷"}) == boost::make_iterator_range(unicode.data() + 9, unicode.data() + 18));
            REQUIRE(unicode.find(u8"それは") == boost::make_iterator_range(unicode.data(), unicode.data() + 9));
        }
        THEN("it will ignore case if requested") {
            REQUIRE(invariant.find(unicode::string{" BAZ"}, true) == boost::make_iterator_range(invariant.data() + 7, invariant.data() + 11));
            REQUIRE(invariant.find(std::string{"BAR"}, true) == boost::make_iterator_range(invariant.data() + 4, invariant.data() + 7));
            REQUIRE(invariant.find("O bAr", true) == boost::make_iterator_range(invariant.data() + 2, invariant.data() + 7));
            REQUIRE(unicode.find(unicode::string{u8"tάχιστη"}, true) == boost::make_iterator_range(unicode.data() + 22, unicode.data() + 35));
            REQUIRE(unicode.find(std::string{u8"tάχιστη"}, true) == boost::make_iterator_range(unicode.data() + 22, unicode.data() + 35));
            REQUIRE(unicode.find(u8"tάχιστη", true) == boost::make_iterator_range(unicode.data() + 22, unicode.data() + 35));
        }
        THEN("it should return npos if it doesn't find a matching substring") {
            REQUIRE(invariant.find(unicode::string{"jam"}) == invariant_eos);
            REQUIRE(invariant.find(std::string{"cake"}) == invariant_eos);
            REQUIRE(invariant.find("foo bar bat") == invariant_eos);
            REQUIRE(unicode.find(unicode::string{u8"私!"}) == unicode_eos);
            REQUIRE(unicode.find(std::string{u8"をχ"}) == unicode_eos);
            REQUIRE(unicode.find(u8"それは私を傷つ!") == unicode_eos);
        }
        THEN("it should handle decomposed graphemes ") {
            REQUIRE(decomposed.find(unicode::string{u8"o"}) == decomposed_eos);
            REQUIRE(decomposed.find(std::string{u8"o"}) == decomposed_eos);
            REQUIRE(decomposed.find(u8"o") == decomposed_eos);
            REQUIRE(decomposed.find(unicode::string{u8"\u0303"}) == decomposed_eos);
            REQUIRE(decomposed.find(std::string{u8"\u0303"}) == decomposed_eos);
            REQUIRE(decomposed.find(u8"\u0303") == decomposed_eos);
            REQUIRE(decomposed.find(unicode::string{u8"no\u0303"}) == boost::make_iterator_range(decomposed.data(), decomposed.data() + 4));
            REQUIRE(decomposed.find(std::string{u8"no\u0303"}) == boost::make_iterator_range(decomposed.data(), decomposed.data() + 4));
            REQUIRE(decomposed.find(u8"no\u0303") == boost::make_iterator_range(decomposed.data(), decomposed.data() + 4));
            REQUIRE(decomposed.find(unicode::string{u8"õ!"}) == boost::make_iterator_range(decomposed.data() + 1, decomposed.data() + 5));
            REQUIRE(decomposed.find(std::string{u8"õ!"}) == boost::make_iterator_range(decomposed.data() + 1, decomposed.data() + 5));
            REQUIRE(decomposed.find(u8"õ!") == boost::make_iterator_range(decomposed.data() + 1, decomposed.data() + 5));
        }
    }
    WHEN("splitting a string") {
        unicode::string invariant{ "foo  bar baz " };
        unicode::string unicode{ u8"χஸ்それχஸ்は私をχஸ்χஸ்傷つχஸ்" };
        unicode::string decomposed{ u8"no\u0303! nõ?!" };

        THEN("splitting on an empty string returns no values") {
            REQUIRE(invariant.split_begin(unicode::string{""}) == invariant.split_end());
            REQUIRE(invariant.split_begin(std::string{""}) == invariant.split_end());
            REQUIRE(invariant.split_begin("") == invariant.split_end());
            REQUIRE(unicode.split_begin(unicode::string{""}) == unicode.split_end());
            REQUIRE(unicode.split_begin(std::string{""}) == unicode.split_end());
            REQUIRE(unicode.split_begin("") == unicode.split_end());
        }
        THEN("splitting on an invariant string returns the expected substrings") {
            std::vector<std::string> substrings = {
                "foo",
                "",
                "bar",
                "baz"
            };

            size_t i = 0;
            for (auto it = invariant.split_begin(" "); it != invariant.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("splitting on an invariant string when ignoring case returns the expected substrings") {
            std::vector<std::string> substrings = {
                "foo",
                "baz"
            };

            size_t i = 0;
            for (auto it = invariant.split_begin("  BAR ", true); it != invariant.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("splitting on a unicode string returns the expected substrings") {
            std::vector<std::string> substrings = {
                u8"",
                u8"それ",
                u8"は私を",
                u8"",
                u8"傷つ"
            };

            size_t i = 0;
            for (auto it = unicode.split_begin(u8"χஸ்"); it != unicode.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("splitting on an invariant string when ignoring case returns the expected substrings") {
            std::vector<std::string> substrings = {
                u8"",
                u8"それ",
                u8"は私を",
                u8"",
                u8"傷つ"
            };

            size_t i = 0;
            for (auto it = unicode.split_begin(u8"χஸ்", true); it != unicode.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("splitting on an invariant string without a matching delimiter returns the entire string") {
            std::vector<std::string> substrings = {
                invariant.data()
            };

            size_t i = 0;
            for (auto it = invariant.split_begin(u8"そ"); it != invariant.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("splitting on a unicode string without a matching delimiter returns the entire string") {
            std::vector<std::string> substrings = {
                unicode.data()
            };

            size_t i = 0;
            for (auto it = unicode.split_begin("foo"); it != unicode.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());
        }
        THEN("it should handle decomposed graphemes") {
            std::vector<std::string> substrings = {
                "n",
                "! n",
                "?!"
            };

            std::vector<std::string> entire = {
                decomposed.data()
            };

            size_t i = 0;
            for (auto it = decomposed.split_begin(u8"o\u0303"); it != decomposed.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());

            i = 0;
            for (auto it = decomposed.split_begin(u8"õ"); it != decomposed.split_end(); ++it, ++i) {
                REQUIRE(i < substrings.size());
                REQUIRE(substrings[i] == *it);
            }
            REQUIRE(i == substrings.size());

            i = 0;
            for (auto it = decomposed.split_begin(u8"\u0303"); it != decomposed.split_end(); ++it, ++i) {
                REQUIRE(i < entire.size());
                REQUIRE(entire[i] == *it);
            }
            REQUIRE(i == entire.size());

            i = 0;
            for (auto it = decomposed.split_begin(u8"o"); it != decomposed.split_end(); ++it, ++i) {
                REQUIRE(i < entire.size());
                REQUIRE(entire[i] == *it);
            }
            REQUIRE(i == entire.size());
        }
    }
}
