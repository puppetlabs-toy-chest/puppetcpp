# Tests for alias

type A = Array[String]

unless A == Array[String] {
    fail incorrect
}

unless Array > A {
    fail incorrect
}

if A > Array[Integer] {
    fail incorrect
}

unless Array >= A {
    fail incorrect
}

if A >= Array[Integer] {
    fail incorrect
}

if Array < A {
    fail incorrect
}

if A < Array[Integer] {
    fail incorrect
}

unless Array >= A {
    fail incorrect
}

if A >= Array[Integer] {
    fail incorrect
}

type X = Variant[X, Y, Z]
type Y = Variant[X, String]
type Z = Variant[Integer]

unless X == X and X == Y {
    fail incorrect
}

unless X >= X and X >= Y and X >= Z {
    fail incorrect
}

unless X >= Variant[String, Integer] and X >= Variant[String] and X >= Variant[Integer] and X >= String and X >= Integer {
    fail incorrect
}

if X >= Variant[Regexp] or X >= Regexp {
    fail incorrect
}

# Tests for Any

unless Any == Any {
    fail incorrect
}

unless Any > String {
    fail incorrect
}

if Any > Any {
    fail incorrect
}

unless Any >= String {
    fail incorrect
}

unless Any >= Any {
    fail incorrect
}

if Any < String {
    fail incorrect
}

if Any < Any {
    fail incorrect
}

if Any <= String {
    fail incorrect
}

unless Any <= Any {
    fail incorrect
}

# Tests for Array

unless Array == Array {
    fail incorrect
}

unless Array[String] == Array[String] {
    fail incorrect
}

unless Array[String, 5, 10] == Array[String, 5, 10] {
    fail incorrect
}

if Array > String {
    fail incorrect
}

if Array > Array {
    fail incorrect
}

unless Array > Array[Integer] {
    fail incorrect
}

unless Array[String] > Tuple[String, String, String] {
    fail incorrect
}

if Array[String] > Array[Integer] {
    fail incorrect
}

if Array[String] > Tuple[String, Integer] {
    fail incorrect
}

if Array[String, 0, 5] > Array[String, 0, 5] {
    fail incorrect
}

unless Array[String, 0, 5] > Array[String, 1, 2] {
    fail incorrect
}

unless Array[String, 0, 5] > Tuple[String, 1, 2] {
    fail incorrect
}

if Array >= String {
    fail incorrect
}

unless Array >= Array {
    fail incorrect
}

unless Array >= Array[Integer] {
    fail incorrect
}

unless Array[String] >= Tuple[String] {
    fail incorrect
}

if Array[String] > Array[Integer] {
    fail incorrect
}

if Array[String] >= Tuple[String, Integer] {
    fail incorrect
}

unless Array[String, 0, 5] >= Array[String, 0, 5] {
    fail incorrect
}

unless Array[String, 0, 5] >= Array[String, 1, 2] {
    fail incorrect
}

unless Array[String, 0, 5] >= Tuple[String, 1, 2] {
    fail incorrect
}

if Array < String {
    fail incorrect
}

if Array < Array {
    fail incorrect
}

if Array < Array[Integer] {
    fail incorrect
}

if Array[String] < Tuple[String] {
    fail incorrect
}

if Array[String] < Array[Integer] {
    fail incorrect
}

if Array[String] < Tuple[String, Integer] {
    fail incorrect
}

if Array[String, 0, 5] < Array[String, 0, 5] {
    fail incorrect
}

if Array[String, 0, 5] < Array[String, 1, 2] {
    fail incorrect
}

if Array[String, 0, 5] < Tuple[String, 1, 2] {
    fail incorrect
}

if Array <= String {
    fail incorrect
}

unless Array <= Array {
    fail incorrect
}

if Array <= Array[Integer] {
    fail incorrect
}

if Array[String] <= Tuple[String] {
    fail incorrect
}

if Array[String] <= Tuple[String] {
    fail incorrect
}

if Array[String] <= Array[Integer] {
    fail incorrect
}

unless Array[String, 0, 5] <= Array[String, 0, 5] {
    fail incorrect
}

if Array[String, 0, 5] <= Array[String, 1, 2] {
    fail incorrect
}

if Array[String, 0, 5] <= Tuple[String, 1, 2] {
    fail incorrect
}

# Boolean tests

if Boolean > Integer {
    fail incorrect
}

if Boolean > Boolean {
    fail incorrect
}

if Boolean >= Integer {
    fail incorrect
}

unless Boolean >= Boolean {
    fail incorrect
}

if Boolean < Integer {
    fail incorrect
}

if Boolean < Boolean {
    fail incorrect
}

if Boolean <= Integer {
    fail incorrect
}

unless Boolean <= Boolean {
    fail incorrect
}

# Callable tests

unless Callable == Callable and Callable[String] == Callable[String] and Callable[String, Integer, 0, 2, Callable[String]] == Callable[String, Integer, 0, 2, Callable[String]] {
    fail incorrect
}

unless Callable > Callable[String] {
    fail incorrect
}

unless Callable >= Callable[String] {
    fail incorrect
}

if Callable < Callable[String] {
    fail incorrect
}

if Callable <= Callable[String] {
    fail incorrect
}

unless Callable[Integer] >= Callable[Numeric] {
    fail incorrect
}

if Callable[Integer, 0, 2, Callable[Integer]] >= Callable[Numeric, 0, 2, Callable[Numeric]] {
    fail incorrect
}

unless Callable[Integer] > Callable[Numeric] {
    fail incorrect
}

if Callable[Integer, 0, 2, Callable[Integer]] > Callable[Numeric, 0, 2, Callable[Numeric]] {
    fail incorrect
}

if Callable[Integer] <= Callable[Numeric] {
    fail incorrect
}

if Callable[Integer] < Callable[Numeric] {
    fail incorrect
}

if Callable[Integer, 0, 2, Callable[Integer]] < Callable[Numeric, 0, 2, Callable[Numeric]] {
    fail incorrect
}

if Callable[Integer] <= Callable[Numeric] {
    fail incorrect
}

if Callable[Integer, 0, 2, Callable[Integer]] <= Callable[Numeric, 0, 2, Callable[Numeric]] {
    fail incorrect
}

# CatalogEntry tests

unless CatalogEntry == CatalogEntry {
    fail incorrect
}

unless CatalogEntry > Resource and CatalogEntry > Class {
    fail incorrect
}

unless CatalogEntry >= Resource and CatalogEntry >= Class {
    fail incorrect
}

if CatalogEntry < Resource or CatalogEntry < Class {
    fail incorrect
}

if CatalogEntry <= Resource or CatalogEntry <= Class {
    fail incorrect
}

# Class tests

unless Class == Class and Class[foo] == Class[foo] and Class[foo] != Class[bar] {
    fail incorrect
}

unless Class > Class[foo] {
    fail incorrect
}

if Class > Class or Class[foo] > Class[foo] {
    fail incorrect
}

unless Class >= Class[foo] and Class[foo] >= Class[foo] {
    fail incorrect
}

if Class[foo] >= Class[bar] {
    fail incorrect
}

if Class < Class[foo] {
    fail incorrect
}

if Class < Class or Class[foo] < Class[foo] {
    fail incorrect
}

if Class <= Class[foo] {
    fail incorrect
}

unless Class[foo] <= Class[foo] {
    fail incorrect
}

if Class[foo] <= Class[bar] {
    fail incorrect
}

# Collection tests

unless Collection == Collection and Collection[5] == Collection[5] and Collection[5] != Collection[10] and Collection[10, 100] == Collection[10, 100] and Collection[10, 100] != Collection[20, 200] {
    fail incorrect
}

unless Collection > Array and Collection > Tuple and Collection > Hash {
    fail incorrect
}

if Collection > Collection {
    fail incorrect
}

if Collection > String {
    fail incorrect
}

unless Collection[0, 10] > Array[String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] > Array[String, default, 10] {
    fail incorrect
}

unless Collection[0, 10] > Tuple[Integer, 0, 10] {
    fail incorrect
}

if Collection[5, 5] > Tuple[Integer, default, 10] {
    fail incorrect
}

unless Collection[0, 10] > Hash[String, String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] > Hash[String, String, default, 10] {
    fail incorrect
}

if Collection[0, 10] > Collection[0, 10] {
    fail incorrect
}

if Collection[5, 5] > Collection[0, 10] {
    fail incorrect
}

if Collection >= String {
    fail incorrect
}

unless Collection[0, 10] >= Array[String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] >= Array[String, default, 10] {
    fail incorrect
}

unless Collection[0, 10] >= Tuple[Integer, 0, 10] {
    fail incorrect
}

if Collection[5, 5] >= Tuple[Integer, default, 10] {
    fail incorrect
}

unless Collection[0, 10] >= Hash[String, String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] >= Hash[String, String, default, 10] {
    fail incorrect
}

unless Collection[0, 10] >= Collection[0, 10] {
    fail incorrect
}

if Collection[5, 5] >= Collection[0, 10] {
    fail incorrect
}

if Collection < String {
    fail incorrect
}

if Collection[0, 10] < Array[String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] < Array[String, default, 10] {
    fail incorrect
}

if Collection[0, 10] < Tuple[Integer, 0, 10] {
    fail incorrect
}

if Collection[5, 5] < Tuple[Integer, default, 10] {
    fail incorrect
}

if Collection[0, 10] < Hash[String, String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] < Hash[String, String, default, 10] {
    fail incorrect
}

if Collection[0, 10] < Collection[0, 10] {
    fail incorrect
}

unless Collection[5, 5] < Collection[0, 10] {
    fail incorrect
}

if Collection <= String {
    fail incorrect
}

if Collection[0, 10] <= Array[String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] <= Array[String, default, 10] {
    fail incorrect
}

if Collection[0, 10] <=Tuple[Integer, 0, 10] {
    fail incorrect
}

if Collection[5, 5] <= Tuple[Integer, default, 10] {
    fail incorrect
}

if Collection[0, 10] <= Hash[String, String, 0, 10] {
    fail incorrect
}

if Collection[5, 5] <= Hash[String, String, default, 10] {
    fail incorrect
}

unless Collection[0, 10] <= Collection[0, 10] {
    fail incorrect
}

unless Collection[5, 5] <= Collection[0, 10] {
    fail incorrect
}

# Data tests

unless Data == Data {
    fail incorrect
}

unless Data > Scalar and Data > Array and Data > Hash and Data > Undef and Data > Tuple {
    fail incorrect
}

if Data > Data {
    fail incorrect
}

if Data > Type {
    fail incorrect
}

unless Data >= Scalar and Data >= Array and Data >= Hash and Data >= Undef and Data >= Tuple {
    fail incorrect
}

unless Data >= Data {
    fail incorrect
}

if Data >= Type {
    fail incorrect
}

if Data < Scalar or Data < Array or Data < Hash or Data < Undef or Data < Tuple {
    fail incorrect
}

if Data < Data {
    fail incorrect
}

if Data < Type {
    fail incorrect
}

if Data <= Scalar or Data <= Array or Data <= Hash or Data <= Undef or Data <= Tuple {
    fail incorrect
}

unless Data <= Data {
    fail incorrect
}

if Data <= Type {
    fail incorrect
}

# Default tests

unless Default == Default {
    fail incorrect
}

if Default > Default {
    fail incorrect
}

if Default > String {
    fail incorrect
}

unless Default >= Default {
    fail incorrect
}

if Default > String {
    fail incorrect
}

if Default < Default {
    fail incorrect
}

if Default < String {
    fail incorrect
}

unless Default <= Default {
    fail incorrect
}

if Default <= String {
    fail incorrect
}

# Enum tests

unless Enum == Enum {
    fail incorrect
}

if Enum > Integer or Enum[foo] > String or Enum[foo] > Enum or Enum[foo] > Pattern {
    fail incorrect
}

if Enum > String or Enum > Pattern {
    fail incorrect
}

unless Enum > Enum[foo] {
    fail incorrect
}

if Enum[foo] > Enum[foo] {
    fail incorrect
}

unless Enum[foo, bar] > Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] > Enum[foo, baz] {
    fail incorrect
}

if Enum >= Integer or Enum[foo] >= String or Enum[foo] >= Enum or Enum[foo] >= Pattern {
    fail incorrect
}

unless Enum >= String and Enum >= Pattern {
    fail incorrect
}

unless Enum >= Enum[foo] {
    fail incorrect
}

unless Enum[foo] >= Enum[foo] {
    fail incorrect
}

unless Enum[foo, bar] >= Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] >= Enum[foo, baz] {
    fail incorrect
}

if Enum < Integer {
    fail incorrect
}

unless Enum[foo] < String and Enum[foo] < Enum and Enum[foo] < Pattern {
    fail incorrect
}

if Enum < String or Enum < Pattern {
    fail incorrect
}

if Enum < Enum[foo] {
    fail incorrect
}

if Enum[foo] < Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] < Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] < Enum[foo, baz] {
    fail incorrect
}

if Enum <= Integer {
    fail incorrect
}

unless Enum[foo] <= String and Enum[foo] <= Enum and Enum[foo] <= Pattern {
    fail incorrect
}

unless Enum <= String and Enum <= Pattern {
    fail incorrect
}

if Enum <= Enum[foo] {
    fail incorrect
}

unless Enum[foo] <= Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] <= Enum[foo] {
    fail incorrect
}

if Enum[foo, bar] <= Enum[foo, baz] {
    fail incorrect
}

# Float tests

unless Float == Float and Float[3] == Float[3] and Float[0, 100] == Float[0, 100] {
    fail incorrect
}

if Float > Float {
    fail incorrect
}

unless Float > Float[3] and Float > Float[0, 100] {
    fail incorrect
}

unless Float[0, 10] > Float[5, 5] {
    fail incorrect
}

if Float[5, 5] > Float[0, 10] {
    fail incorrect
}

unless Float >= Float {
    fail incorrect
}

unless Float >= Float[3] and Float >= Float[0, 100] {
    fail incorrect
}

unless Float[0, 10] >= Float[5, 5] {
    fail incorrect
}

if Float[5, 5] >= Float[0, 10] {
    fail incorrect
}

if Float < Float {
    fail incorrect
}

if Float < Float[3] or Float < Float[0, 100] {
    fail incorrect
}

if Float[0, 10] < Float[5, 5] {
    fail incorrect
}

unless Float[5, 5] < Float[0, 10] {
    fail incorrect
}

unless Float <= Float {
    fail incorrect
}

if Float <= Float[3] or Float <= Float[0, 100] {
    fail incorrect
}

if Float[0, 10] <= Float[5, 5] {
    fail incorrect
}

unless Float[5, 5] <= Float[0, 10] {
    fail incorrect
}

# Hash tests

unless Hash == Hash and Hash[String, Integer] == Hash[String, Integer] and
       Hash[String, Integer, 0] == Hash[String, Integer, 0] and Hash[String, Integer, 0, 10] == Hash[String, Integer, 0, 10] {
    fail incorrect
}

unless Hash[String, Integer] != Hash[String, String] and Hash[String, Integer, 0] != Hash[String, Integer, 10] and Hash[String, Integer, 0, 10] != Hash[String, Integer, 0, 20] {
    fail incorrect
}

if Hash > Hash {
    fail incorrect
}

if Hash > String {
    fail incorrect
}

unless Hash > Hash[String, String] {
    fail incorrect
}

unless Hash[Numeric, Numeric] > Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] > Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] > Hash[String, String] {
    fail incorrect
}

unless Hash[Numeric, String, 0, 10] > Hash[Integer, String, 5, 5] {
    fail incorrect
}

if Hash[Numeric, String, 5, 5] > Hash[Integer, String, 0, 10] {
    fail incorrect
}

unless Hash[String, String] > Struct[{ foo => String, bar => String }] {
    fail incorrectqqq
}

if Hash[Integer, String] > Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, Integer] > Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, String, 2, 2] > Struct[{ foo => String, bar => String }] {
    fail incorrect
}

unless Hash[String, String] > Struct[{ Optional[foo] => String, NotUndef[bar] => String }] {
    fail incorrect
}

unless Hash >= Hash {
    fail incorrect
}

if Hash >= String {
    fail incorrect
}

unless Hash >= Hash[String, String] {
    fail incorrect
}

unless Hash[Numeric, Numeric] >= Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] >= Hash[Integer, Integer] {
    fail incorrect
}

unless Hash[String, String] >= Hash[String, String] {
    fail incorrect
}

unless Hash[Numeric, String, 0, 10] >= Hash[Integer, String, 5, 5] {
    fail incorrect
}

if Hash[Numeric, String, 5, 5] >= Hash[Integer, String, 0, 10] {
    fail incorrect
}

unless Hash[String, String] >= Struct[{ foo => String, bar => String }] {
    fail incorrectqqq
}

if Hash[Integer, String] >= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, Integer] >= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

unless Hash[String, String, 2, 2] >= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

unless Hash[String, String] >= Struct[{ Optional[foo] => String, NotUndef[bar] => String }] {
    fail incorrect
}

if Hash < Hash {
    fail incorrect
}

if Hash < String {
    fail incorrect
}

if Hash < Hash[String, String] {
    fail incorrect
}

if Hash[Numeric, Numeric] < Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] < Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] < Hash[String, String] {
    fail incorrect
}

if Hash[Numeric, String, 0, 10] < Hash[Integer, String, 5, 5] {
    fail incorrect
}

unless Hash[Integer, String, 5, 5] < Hash[Numeric, String, 0, 10] {
    fail incorrect
}

if Hash[String, String] < Struct[{ foo => String, bar => String }] {
    fail incorrectqqq
}

if Hash[Integer, String] < Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, Integer] < Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, String, 2, 2] < Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, String] < Struct[{ Optional[foo] => String, NotUndef[bar] => String }] {
    fail incorrect
}

unless Hash <= Hash {
    fail incorrect
}

if Hash <= String {
    fail incorrect
}

if Hash <= Hash[String, String] {
    fail incorrect
}

if Hash[Numeric, Numeric] <= Hash[Integer, Integer] {
    fail incorrect
}

if Hash[String, String] <= Hash[Integer, Integer] {
    fail incorrect
}

unless Hash[String, String] <= Hash[String, String] {
    fail incorrect
}

if Hash[Numeric, String, 0, 10] <= Hash[Integer, String, 5, 5] {
    fail incorrect
}

unless Hash[Integer, String, 5, 5] <= Hash[Numeric, String, 0, 10] {
    fail incorrect
}

if Hash[String, String] <= Struct[{ foo => String, bar => String }] {
    fail incorrectqqq
}

if Hash[Integer, String] <= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, Integer] <= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

unless Hash[String, String, 2, 2] <= Struct[{ foo => String, bar => String }] {
    fail incorrect
}

if Hash[String, String] <= Struct[{ Optional[foo] => String, NotUndef[bar] => String }] {
    fail incorrect
}

# Integer tests

unless Integer == Integer and Integer[3] == Integer[3] and Integer[0, 100] == Integer[0, 100] {
    fail incorrect
}

if Integer > Integer {
    fail incorrect
}

unless Integer > Integer[3] and Integer > Integer[0, 100] {
    fail incorrect
}

unless Integer[0, 10] > Integer[5, 5] {
    fail incorrect
}

if Integer[5, 5] > Integer[0, 10] {
    fail incorrect
}

unless Integer >= Integer {
    fail incorrect
}

unless Integer >= Integer[3] and Integer >= Integer[0, 100] {
    fail incorrect
}

unless Integer[0, 10] >= Integer[5, 5] {
    fail incorrect
}

if Integer[5, 5] >= Integer[0, 10] {
    fail incorrect
}

if Integer < Integer {
    fail incorrect
}

if Integer < Integer[3] or Integer < Integer[0, 100] {
    fail incorrect
}

if Integer[0, 10] < Integer[5, 5] {
    fail incorrect
}

unless Integer[5, 5] < Integer[0, 10] {
    fail incorrect
}

unless Integer <= Integer {
    fail incorrect
}

if Integer <= Integer[3] or Integer <= Integer[0, 100] {
    fail incorrect
}

if Integer[0, 10] <= Integer[5, 5] {
    fail incorrect
}

unless Integer[5, 5] <= Integer[0, 10] {
    fail incorrect
}

unless Iterable == Iterable and Iterable[String] == Iterable[String] {
    fail incorrect
}

unless Iterable != Iterable[String] and Iterable[String] != Iterable[Integer] {
    fail incorrect
}

if Iterable > Iterable or Iterable[String] > Iterable[String] {
    fail incorrect
}

if Iterable > Float {
    fail incorrect
}

unless Iterable > String and Iterable[String] > String {
    fail incorrect
}

unless Iterable > Array {
    fail incorrect
}

if Iterable[Integer] > Array or Iterable[Integer] > Array[String] {
    fail incorrect
}

unless Iterable[String] > Array[String] {
    fail incorrect
}

unless Iterable > Hash and Iterable[Tuple[String, Integer]] > Hash[String, Integer] {
    fail incorrect
}

if Iterable[String] > Hash or Iterable[Tuple[String]] > Hash[String, String] or Iterable[Tuple[String, Integer]] > Hash[Integer, String] {
    fail incorrect
}

if Iterable > Integer or Iterable > Integer[0, default] or Iterable > Integer[default, 10] {
    fail incorrect
}

unless Iterable > Integer[0, 10] and Iterable[Integer] > Integer[0, 10] {
    fail incorrect
}

if Iterable[String] > Integer[0, 10] or Iterable[Integer[0, 10]] > Integer[20, 30] {
    fail incorrect
}

unless Iterable > Enum {
    fail incorrect
}

unless Iterable[String] > Enum[foo] {
    fail incorrect
}

if Iterable[Integer] > Enum[foo] {
    fail incorrect
}

unless Iterable > Iterator {
    fail incorrect
}

unless Iterable[Numeric] > Iterator[Integer] {
    fail incorrect
}

if Iterable[Integer] > Iterator[String] {
    fail incorrect
}

unless Iterable >= Iterable and Iterable[String] >= Iterable[String] {
    fail incorrect
}

if Iterable >= Float {
    fail incorrect
}

unless Iterable >= String and Iterable[String] >= String {
    fail incorrect
}

unless Iterable >= Array {
    fail incorrect
}

if Iterable[Integer] >= Array or Iterable[Integer] >= Array[String] {
    fail incorrect
}

unless Iterable[String] >= Array[String] {
    fail incorrect
}

unless Iterable >= Hash and Iterable[Tuple[String, Integer]] >= Hash[String, Integer] {
    fail incorrect
}

if Iterable[String] >= Hash or Iterable[Tuple[String]] >= Hash[String, String] or Iterable[Tuple[String, Integer]] >= Hash[Integer, String] {
    fail incorrect
}

if Iterable >= Integer or Iterable >= Integer[0, default] or Iterable >= Integer[default, 10] {
    fail incorrect
}

unless Iterable >= Integer[0, 10] and Iterable[Integer] >= Integer[0, 10] {
    fail incorrect
}

if Iterable[String] >= Integer[0, 10] or Iterable[Integer[0, 10]] >= Integer[20, 30] {
    fail incorrect
}

unless Iterable >= Enum {
    fail incorrect
}

unless Iterable[String] >= Enum[foo] {
    fail incorrect
}

if Iterable[Integer] >= Enum[foo] {
    fail incorrect
}

unless Iterable >= Iterator {
    fail incorrect
}

unless Iterable[Numeric] >= Iterator[Integer] {
    fail incorrect
}

if Iterable[Integer] >= Iterator[String] {
    fail incorrect
}

if Iterable < Iterable or Iterable[String] < Iterable[String] {
    fail incorrect
}

if Iterable < Float {
    fail incorrect
}

if Iterable < String or Iterable[String] < String {
    fail incorrect
}

if Iterable < Array {
    fail incorrect
}

if Iterable[Integer] < Array or Iterable[Integer] < Array[String] {
    fail incorrect
}

if Iterable[String] < Array[String] {
    fail incorrect
}

if Iterable < Hash or Iterable[Tuple[String, Integer]] < Hash[String, Integer] {
    fail incorrect
}

if Iterable[String] < Hash or Iterable[Tuple[String]] < Hash[String, String] or Iterable[Tuple[String, Integer]] < Hash[Integer, String] {
    fail incorrect
}

if Iterable < Integer or Iterable < Integer[0, default] or Iterable < Integer[default, 10] {
    fail incorrect
}

if Iterable < Integer[0, 10] or Iterable[Integer] < Integer[0, 10] {
    fail incorrect
}

if Iterable[String] < Integer[0, 10] or Iterable[Integer[0, 10]] < Integer[20, 30] {
    fail incorrect
}

if Iterable < Enum {
    fail incorrect
}

if Iterable[String] < Enum[foo] {
    fail incorrect
}

if Iterable[Integer] < Enum[foo] {
    fail incorrect
}

if Iterable < Iterator {
    fail incorrect
}

if Iterable[Numeric] < Iterator[Integer] {
    fail incorrect
}

if Iterable[Integer] < Iterator[String] {
    fail incorrect
}

unless Iterable <= Iterable and Iterable[String] <= Iterable[String] {
    fail incorrect
}

if Iterable <= Float {
    fail incorrect
}

if Iterable <= String or Iterable[String] <= String {
    fail incorrect
}

if Iterable <= Array {
    fail incorrect
}

if Iterable[Integer] <= Array or Iterable[Integer] <= Array[String] {
    fail incorrect
}

if Iterable[String] <= Array[String] {
    fail incorrect
}

if Iterable <= Hash or Iterable[Tuple[String, Integer]] <= Hash[String, Integer] {
    fail incorrect
}

if Iterable[String] <= Hash or Iterable[Tuple[String]] <= Hash[String, String] or Iterable[Tuple[String, Integer]] <= Hash[Integer, String] {
    fail incorrect
}

if Iterable <= Integer or Iterable <= Integer[0, default] or Iterable <= Integer[default, 10] {
    fail incorrect
}

if Iterable <= Integer[0, 10] or Iterable[Integer] <= Integer[0, 10] {
    fail incorrect
}

if Iterable[String] <= Integer[0, 10] or Iterable[Integer[0, 10]] <= Integer[20, 30] {
    fail incorrect
}

if Iterable <= Enum {
    fail incorrect
}

if Iterable[String] <= Enum[foo] {
    fail incorrect
}

if Iterable[Integer] <= Enum[foo] {
    fail incorrect
}

if Iterable <= Iterator {
    fail incorrect
}

if Iterable[Numeric] <= Iterator[Integer] {
    fail incorrect
}

if Iterable[Integer] <= Iterator[String] {
    fail incorrect
}

# TODO: add Iterator tests

# Numeric tests

unless Numeric == Numeric {
    fail incorrect
}

if Numeric > Numeric {
    fail incorrect
}

unless Numeric > Integer and Numeric > Integer[0, 10] and Numeric > Float and Numeric > Float[0, 10] {
    fail incorrect
}

if Numeric > String {
    fail incorrect
}

unless Numeric >= Numeric {
    fail incorrect
}

unless Numeric >= Integer and Numeric >= Integer[0, 10] and Numeric >= Float and Numeric >= Float[0, 10] {
    fail incorrect
}

if Numeric >= String {
    fail incorrect
}

if Numeric < Numeric {
    fail incorrect
}

if Numeric < Integer or Numeric < Integer[0, 10] or Numeric < Float or Numeric < Float[0, 10] {
    fail incorrect
}

if Numeric < String {
    fail incorrect
}

unless Numeric <= Numeric {
    fail incorrect
}

if Numeric <= Integer or Numeric <= Integer[0, 10] or Numeric <= Float or Numeric <= Float[0, 10] {
    fail incorrect
}

if Numeric <= String {
    fail incorrect
}

# Optional tests

unless Optional == Optional and Optional[String] == Optional[String] and Optional == Optional[Undef] {
    fail incorrect
}

unless Optional[Integer] != Optional[String] {
    fail incorrect
}

unless Optional > Undef and Optional[String] > Undef {
    fail incorrect
}

if Optional > Optional or Optional[String] > Optional[String] {
    fail incorrect
}

unless Optional > Undef {
    fail incorrect
}

if Optional > Optional[String] {
    fail incorrect
}

unless Optional[String] > String {
    fail incorrect
}

unless Optional >= Undef and Optional[String] >= Undef {
    fail incorrect
}

unless Optional >= Optional and Optional[String] >= Optional[String] {
    fail incorrect
}

unless Optional >= Undef {
    fail incorrect
}

if Optional >= Optional[String] {
    fail incorrect
}

unless Optional[String] >= String {
    fail incorrect
}

if Optional < Undef or Optional[String] < Undef {
    fail incorrect
}

if Optional < Optional or Optional[String] < Optional[String] {
    fail incorrect
}

if Optional < Undef {
    fail incorrect
}

if Optional < Optional[String] {
    fail incorrect
}

if Optional[String] < String {
    fail incorrect
}

if Optional <= Undef or Optional[String] <= Undef {
    fail incorrect
}

unless Optional <= Optional and Optional[String] <= Optional[String] {
    fail incorrect
}

if Optional <= Undef {
    fail incorrect
}

if Optional <= Optional[String] {
    fail incorrect
}

if Optional[String] <= String {
    fail incorrect
}

# Pattern tests

unless Pattern == Pattern and Pattern[foo] == Pattern[foo] and Pattern[foo, /bar/] == Pattern[foo, /bar/] {
    fail incorrect
}

if Pattern > Pattern {
    fail incorrect
}

if Pattern[foo] > Pattern {
    fail incorrect
}

if Pattern > String {
    fail incorrect
}

if Pattern[foo] > String {
    fail incorrect
}

if Pattern > Enum {
    fail incorrect
}

if Pattern[/^foo$/] > Enum {
    fail incorrect
}

unless Pattern[/^foo$/] > Enum[foo] {
    fail incorrect
}

if Pattern[/^foo$/] > Enum[bar] {
    fail incorrect
}

unless Pattern >= Pattern {
    fail incorrect
}

if Pattern[foo] >= Pattern {
    fail incorrect
}

unless Pattern >= String {
    fail incorrect
}

if Pattern[foo] >= String {
    fail incorrect
}

unless Pattern >= Enum {
    fail incorrect
}

if Pattern[/^foo$/] >= Enum {
    fail incorrect
}

unless Pattern[/^foo$/] >= Enum[foo] {
    fail incorrect
}

if Pattern[/^foo$/] >= Enum[bar] {
    fail incorrect
}

if Pattern < Pattern {
    fail incorrect
}

unless Pattern[foo] < Pattern {
    fail incorrect
}

if Pattern < String {
    fail incorrect
}

unless Pattern[foo] < String {
    fail incorrect
}

if Pattern < Enum {
    fail incorrect
}

unless Pattern[/^foo$/] < Enum {
    fail incorrect
}

if Pattern[/^foo$/] < Enum[foo] {
    fail incorrect
}

if Pattern[/^foo$/] < Enum[bar] {
    fail incorrect
}

unless Pattern <= Pattern {
    fail incorrect
}

unless Pattern[foo] <= Pattern {
    fail incorrect
}

unless Pattern <= String {
    fail incorrect
}

unless Pattern[foo] <= String {
    fail incorrect
}

unless Pattern <= Enum {
    fail incorrect
}

unless Pattern[/^foo$/] <= Enum {
    fail incorrect
}

if Pattern[/^foo$/] <= Enum[foo] {
    fail incorrect
}

if Pattern[/^foo$/] <= Enum[bar] {
    fail incorrect
}

# Regexp tests

unless Regexp == Regexp and Regexp[foo] == Regexp[/foo/] and Regexp[foo] != Regexp[bar] {
    fail incorrect
}

if Regexp > String {
    fail incorrect
}

if Regexp > Regexp {
    fail incorrect
}

unless Regexp > Regexp[foo] {
    fail incorrect
}

if Regexp[foo] > Regexp[/foo/] {
    fail incorrect
}

if Regexp[/foo/] > Regexp[bar] {
    fail incorrect
}

if Regexp >= String {
    fail incorrect
}

unless Regexp >= Regexp {
    fail incorrect
}

unless Regexp >= Regexp[foo] {
    fail incorrect
}

unless Regexp[foo] >= Regexp[/foo/] {
    fail incorrect
}

if Regexp[/foo/] >= Regexp[bar] {
    fail incorrect
}

if Regexp < String {
    fail incorrect
}

if Regexp < Regexp {
    fail incorrect
}

if Regexp < Regexp[foo] {
    fail incorrect
}

if Regexp[foo] < Regexp[/foo/] {
    fail incorrect
}

if Regexp[/foo/] < Regexp[bar] {
    fail incorrect
}

if Regexp <= String {
    fail incorrect
}

unless Regexp <= Regexp {
    fail incorrect
}

if Regexp <= Regexp[foo] {
    fail incorrect
}

unless Regexp[foo] <= Regexp[/foo/] {
    fail incorrect
}

if Regexp[/foo/] <= Regexp[bar] {
    fail incorrect
}

# Resource tests

unless Resource == Resource and Resource[foo] == Resource[foo] and Resource[foo] != Resource[bar] and Resource[foo, bar] == Resource[foo, bar] and Resource[foo, bar] != Resource[bar, foo] {
    fail incorrect
}

if Resource > String {
    fail incorrect
}

if Resource > Resource {
    fail incorrect
}

unless Resource > Resource[foo] {
    fail incorrect
}

unless Resource > Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] > Resource[foo] {
    fail incorrect
}

if Resource[foo] > Resource[bar] {
    fail incorrect
}

unless Resource[foo] > Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] > Resource[bar, baz] {
    fail incorrect
}

if Resource[foo, bar] > Resource[foo, bar] {
    fail incorrect
}

if Resource[foo, bar] > Resource[foo, baz] {
    fail incorrect
}

if Resource >= String {
    fail incorrect
}

unless Resource >= Resource {
    fail incorrect
}

unless Resource >= Resource[foo] {
    fail incorrect
}

unless Resource >= Resource[foo, bar] {
    fail incorrect
}

unless Resource[foo] >= Resource[foo] {
    fail incorrect
}

if Resource[foo] >= Resource[bar] {
    fail incorrect
}

unless Resource[foo] >= Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] >= Resource[bar, baz] {
    fail incorrect
}

unless Resource[foo, bar] >= Resource[foo, bar] {
    fail incorrect
}

if Resource[foo, bar] >= Resource[foo, baz] {
    fail incorrect
}

if Resource < String {
    fail incorrect
}

if Resource < Resource {
    fail incorrect
}

if Resource < Resource[foo] {
    fail incorrect
}

if Resource < Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] < Resource[foo] {
    fail incorrect
}

if Resource[foo] < Resource[bar] {
    fail incorrect
}

if Resource[foo] < Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] < Resource[bar, baz] {
    fail incorrect
}

if Resource[foo, bar] < Resource[foo, bar] {
    fail incorrect
}

if Resource[foo, bar] < Resource[foo, baz] {
    fail incorrect
}

if Resource <= String {
    fail incorrect
}

unless Resource <= Resource {
    fail incorrect
}

if Resource <= Resource[foo] {
    fail incorrect
}

if Resource <= Resource[foo, bar] {
    fail incorrect
}

unless Resource[foo] <= Resource[foo] {
    fail incorrect
}

if Resource[foo] <= Resource[bar] {
    fail incorrect
}

if Resource[foo] <= Resource[foo, bar] {
    fail incorrect
}

if Resource[foo] <= Resource[bar, baz] {
    fail incorrect
}

unless Resource[foo, bar] <= Resource[foo, bar] {
    fail incorrect
}

if Resource[foo, bar] <= Resource[foo, baz] {
    fail incorrect
}

# Runtime tests

unless Runtime == Runtime and Runtime[foo] == Runtime[foo] and Runtime[foo] != Runtime[bar] and Runtime[foo, bar] == Runtime[foo, bar] and Runtime[foo, bar] != Runtime[bar, foo] {
    fail incorrect
}

if Runtime > String {
    fail incorrect
}

if Runtime > Runtime {
    fail incorrect
}

unless Runtime > Runtime[foo] {
    fail incorrect
}

unless Runtime > Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] > Runtime[foo] {
    fail incorrect
}

if Runtime[foo] > Runtime[bar] {
    fail incorrect
}

unless Runtime[foo] > Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] > Runtime[bar, baz] {
    fail incorrect
}

if Runtime[foo, bar] > Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo, bar] > Runtime[foo, baz] {
    fail incorrect
}

if Runtime >= String {
    fail incorrect
}

unless Runtime >= Runtime {
    fail incorrect
}

unless Runtime >= Runtime[foo] {
    fail incorrect
}

unless Runtime >= Runtime[foo, bar] {
    fail incorrect
}

unless Runtime[foo] >= Runtime[foo] {
    fail incorrect
}

if Runtime[foo] >= Runtime[bar] {
    fail incorrect
}

unless Runtime[foo] >= Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] >= Runtime[bar, baz] {
    fail incorrect
}

unless Runtime[foo, bar] >= Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo, bar] >= Runtime[foo, baz] {
    fail incorrect
}

if Runtime < String {
    fail incorrect
}

if Runtime < Runtime {
    fail incorrect
}

if Runtime < Runtime[foo] {
    fail incorrect
}

if Runtime < Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] < Runtime[foo] {
    fail incorrect
}

if Runtime[foo] < Runtime[bar] {
    fail incorrect
}

if Runtime[foo] < Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] < Runtime[bar, baz] {
    fail incorrect
}

if Runtime[foo, bar] < Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo, bar] < Runtime[foo, baz] {
    fail incorrect
}

if Runtime <= String {
    fail incorrect
}

unless Runtime <= Runtime {
    fail incorrect
}

if Runtime <= Runtime[foo] {
    fail incorrect
}

if Runtime <= Runtime[foo, bar] {
    fail incorrect
}

unless Runtime[foo] <= Runtime[foo] {
    fail incorrect
}

if Runtime[foo] <= Runtime[bar] {
    fail incorrect
}

if Runtime[foo] <= Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo] <= Runtime[bar, baz] {
    fail incorrect
}

unless Runtime[foo, bar] <= Runtime[foo, bar] {
    fail incorrect
}

if Runtime[foo, bar] <= Runtime[foo, baz] {
    fail incorrect
}

# Scalar tests

unless Scalar == Scalar {
    fail incorrect
}

if Scalar > Scalar {
    fail incorrect
}

if Scalar > Array {
    fail incorrect
}

unless Scalar > Numeric {
    fail incorrect
}

unless Scalar > Integer and Scalar > Integer[0, 10] {
    fail incorrect
}

unless Scalar > Float and Scalar > Float[0, 10] {
    fail incorrect
}

unless Scalar > String and Scalar > String[0, 10] {
    fail incorrect
}

unless Scalar > Boolean {
    fail incorrect
}

unless Scalar > Regexp and Scalar > Regexp[foo] {
    fail incorrect
}

unless Scalar >= Scalar {
    fail incorrect
}

if Scalar >= Array {
    fail incorrect
}

unless Scalar >= Numeric {
    fail incorrect
}

unless Scalar >= Integer and Scalar >= Integer[0, 10] {
    fail incorrect
}

unless Scalar >= Float and Scalar >= Float[0, 10] {
    fail incorrect
}

unless Scalar >= String and Scalar >= String[0, 10] {
    fail incorrect
}

unless Scalar >= Boolean {
    fail incorrect
}

unless Scalar >= Regexp and Scalar >= Regexp[foo] {
    fail incorrect
}

if Scalar < Scalar {
    fail incorrect
}

if Scalar < Array {
    fail incorrect
}

if Scalar < Numeric {
    fail incorrect
}

if Scalar < Integer or Scalar < Integer[0, 10] {
    fail incorrect
}

if Scalar < Float or Scalar < Float[0, 10] {
    fail incorrect
}

if Scalar < String or Scalar < String[0, 10] {
    fail incorrect
}

if Scalar < Boolean {
    fail incorrect
}

if Scalar < Regexp or Scalar < Regexp[foo] {
    fail incorrect
}

unless Scalar <= Scalar {
    fail incorrect
}

if Scalar <= Array {
    fail incorrect
}

if Scalar <= Numeric {
    fail incorrect
}

if Scalar <= Integer or Scalar <= Integer[0, 10] {
    fail incorrect
}

if Scalar <= Float or Scalar <= Float[0, 10] {
    fail incorrect
}

if Scalar <= String or Scalar <= String[0, 10] {
    fail incorrect
}

if Scalar <= Boolean {
    fail incorrect
}

if Scalar <= Regexp or Scalar <= Regexp[foo] {
    fail incorrect
}

# String tests

unless String == String and String[0] == String[0] and String[0] != String[10] and String[0, 10] == String[0, 10] and String[0, 10] != String[1, 2] {
    fail incorrect
}

if String > Integer {
    fail incorrect
}

if String > String {
    fail incorrect
}

if String > String[0] {
    fail incorrect
}

unless String[0] > String[1] {
    fail incorrect
}

if String[5] > String[3] {
    fail incorrect
}

unless String > String[0, 10] {
    fail incorrect
}

unless String[0] > String[0, 10] {
    fail incorrect
}

unless String[0, 10] > String[5, 6] {
    fail incorrect
}

if String[0, 10] > String[20, 30] {
    fail incorrect
}

if String >= Integer {
    fail incorrect
}

unless String >= String {
    fail incorrect
}

unless String >= String[0] {
    fail incorrect
}

unless String[0] >= String[1] {
    fail incorrect
}

if String[5] >= String[3] {
    fail incorrect
}

unless String >= String[0, 10] {
    fail incorrect
}

unless String[0] >= String[0, 10] {
    fail incorrect
}

unless String[0, 10] >= String[5, 6] {
    fail incorrect
}

if String[0, 10] >= String[20, 30] {
    fail incorrect
}

if String < Integer {
    fail incorrect
}

if String < String {
    fail incorrect
}

if String < String[0] {
    fail incorrect
}

if String[0] < String[1] {
    fail incorrect
}

unless String[5] < String[3] {
    fail incorrect
}

if String < String[0, 10] {
    fail incorrect
}

if String[0] < String[0, 10] {
    fail incorrect
}

if String[0, 10] < String[5, 6] {
    fail incorrect
}

if String[0, 10] < String[20, 30] {
    fail incorrect
}

if String <= Integer {
    fail incorrect
}

unless String <= String {
    fail incorrect
}

unless String <= String[0] {
    fail incorrect
}

if String[0] <= String[1] {
    fail incorrect
}

unless String[5] <= String[3] {
    fail incorrect
}

if String <= String[0, 10] {
    fail incorrect
}

if String[0] <= String[0, 10] {
    fail incorrect
}

if String[0, 10] <= String[5, 6] {
    fail incorrect
}

if String[0, 10] <= String[20, 30] {
    fail incorrect
}

# Struct tests

unless Struct == Struct and Struct[{foo => String, bar => String }] == Struct[{bar => String, foo => String}] {
    fail incorrect
}

if Struct > Integer {
    fail incorrect
}

if Struct > Struct {
    fail incorrect
}

if Struct > Struct[{ foo => Integer }] {
    fail incorrect
}

unless Struct[{ foo => Numeric }] > Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Integer }] > Struct[{ bar => Integer }] {
    fail incorrect
}

unless Struct[{ Optional[foo] => Integer, bar => Integer }] > Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => Integer, bar => Integer }] > Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct > Hash {
    fail incorrect
}

unless Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] > Hash[String, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] > Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, baz => String }] > Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct >= Integer {
    fail incorrect
}

unless Struct >= Struct {
    fail incorrect
}

if Struct >= Struct[{ foo => Integer }] {
    fail incorrect
}

unless Struct[{ foo => Numeric }] >= Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Integer }] >= Struct[{ bar => Integer }] {
    fail incorrect
}

unless Struct[{ Optional[foo] => Integer, bar => Integer }] >= Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => Integer, bar => Integer }] >= Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct >= Hash {
    fail incorrect
}

unless Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] >= Hash[String, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] >= Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, baz => String }] >= Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct < Integer {
    fail incorrect
}

if Struct < Struct {
    fail incorrect
}

if Struct < Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Numeric }] < Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Integer }] < Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ Optional[foo] => Integer, bar => Integer }] < Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => Integer, bar => Integer }] < Struct[{ bar => Integer }] {
    fail incorrect
}

unless Struct < Hash {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] < Hash[String, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] < Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, baz => String }] < Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct <= Integer {
    fail incorrect
}

unless Struct <= Struct {
    fail incorrect
}

if Struct <= Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Numeric }] <= Struct[{ foo => Integer }] {
    fail incorrect
}

if Struct[{ foo => Integer }] <= Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ Optional[foo] => Integer, bar => Integer }] <= Struct[{ bar => Integer }] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => Integer, bar => Integer }] <= Struct[{ bar => Integer }] {
    fail incorrect
}

unless Struct <= Hash {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] <= Hash[String, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, Optional[baz] => String }] <= Hash[Integer, String, 2, 2] {
    fail incorrect
}

if Struct[{ NotUndef[foo] => String, bar => String, baz => String }] <= Hash[Integer, String, 2, 2] {
    fail incorrect
}

# Tuple tests

unless Tuple == Tuple and Tuple[String, Integer] == Tuple[String, Integer] and Tuple[String, Numeric] != Tuple[String, Integer] and
       Tuple[String, Numeric, 5] == Tuple[String, Numeric, 5] and Tuple[String, Numeric, 5, 10] == Tuple[String, Numeric, 5, 10] and
       Tuple[String, Numeric, 5, 10] != Tuple[String, Numeric, 20, 100] {
   fail incorrect
}

if Tuple > String {
    fail incorrect
}

unless Tuple > Array {
    fail incorrect
}

unless Tuple[Numeric, Integer, Numeric] > Array[Integer, 3, 3] {
    fail incorrect
}

if Tuple[String] > Array[Integer, 1, 1] {
    fail incorrect
}

unless Tuple[Numeric, 0, 10] > Array[Integer, 0, 5] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] > Array[Integer, 10, 20] {
    fail incorrect
}

if Tuple > Tuple {
    fail incorrect
}

if Tuple[String] > Tuple {
    fail incorrect
}

if Tuple[String] > Tuple[String] {
    fail incorrect
}

unless Tuple[Numeric, String] > Tuple[Integer, String] {
    fail incorrect
}

unless Tuple[Numeric, String, 3, 3] > Tuple[Integer, String, String] {
    fail incorrect
}

if Tuple >= String {
    fail incorrect
}

unless Tuple >= Array {
    fail incorrect
}

unless Tuple[Numeric, Integer, Numeric] >= Array[Integer, 3, 3] {
    fail incorrect
}

if Tuple[String] >= Array[Integer, 1, 1] {
    fail incorrect
}

unless Tuple[Numeric, 0, 10] >= Array[Integer, 0, 5] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] >= Array[Integer, 10, 20] {
    fail incorrect
}

unless Tuple >= Tuple {
    fail incorrect
}

if Tuple[String] >= Tuple {
    fail incorrect
}

unless Tuple[String] >= Tuple[String] {
    fail incorrect
}

unless Tuple[Numeric, String] >= Tuple[Integer, String] {
    fail incorrect
}

unless Tuple[Numeric, String, 3, 3] >= Tuple[Integer, String, String] {
    fail incorrect
}

if Tuple < String {
    fail incorrect
}

if Tuple < Array {
    fail incorrect
}

if Tuple[Numeric, Integer, Numeric] < Array[Integer, 3, 3] {
    fail incorrect
}

if Tuple[String] < Array[Integer, 1, 1] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] < Array[Integer, 0, 5] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] < Array[Integer, 10, 20] {
    fail incorrect
}

if Tuple < Tuple {
    fail incorrect
}

unless Tuple[String] < Tuple {
    fail incorrect
}

if Tuple[String] < Tuple[String] {
    fail incorrect
}

if Tuple[Numeric, String] < Tuple[Integer, String] {
    fail incorrect
}

if Tuple[Numeric, String, 3, 3] < Tuple[Integer, String, String] {
    fail incorrect
}

if Tuple <= String {
    fail incorrect
}

if Tuple <= Array {
    fail incorrect
}

if Tuple[Numeric, Integer, Numeric] <= Array[Integer, 3, 3] {
    fail incorrect
}

if Tuple[String] <= Array[Integer, 1, 1] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] <= Array[Integer, 0, 5] {
    fail incorrect
}

if Tuple[Numeric, 0, 10] <= Array[Integer, 10, 20] {
    fail incorrect
}

unless Tuple <= Tuple {
    fail incorrect
}

unless Tuple[String] <= Tuple {
    fail incorrect
}

unless Tuple[String] <= Tuple[String] {
    fail incorrect
}

if Tuple[Numeric, String] <= Tuple[Integer, String] {
    fail incorrect
}

if Tuple[Numeric, String, 3, 3] <= Tuple[Integer, String, String] {
    fail incorrect
}

# Type tests

unless Type == Type and Type[String] == Type[String] and Type[Numeric] != Type[Integer] {
    fail incorrect
}

if Type > String {
    fail incorrect
}

if Type > Type {
    fail incorrect
}

unless Type > Type[String] {
    fail incorrect
}

unless Type[Numeric] > Type[Integer] {
    fail incorrect
}

if Type[Numeric] > Type[String] {
    fail incorrect
}

if Type >= String {
    fail incorrect
}

unless Type >= Type {
    fail incorrect
}

unless Type >= Type[String] {
    fail incorrect
}

unless Type[Numeric] >= Type[Integer] {
    fail incorrect
}

if Type[Numeric] >= Type[String] {
    fail incorrect
}

if Type < String {
    fail incorrect
}

if Type < Type {
    fail incorrect
}

if Type < Type[String] {
    fail incorrect
}

if Type[Numeric] < Type[Integer] {
    fail incorrect
}

if Type[Numeric] < Type[String] {
    fail incorrect
}

if Type <= String {
    fail incorrect
}

unless Type <= Type {
    fail incorrect
}

if Type <= Type[String] {
    fail incorrect
}

if Type[Numeric] <= Type[Integer] {
    fail incorrect
}

if Type[Numeric] <= Type[String] {
    fail incorrect
}

# Undef tests

unless Undef == Undef and Undef != Integer {
    fail incorrect
}

if Undef > String {
    fail incorrect
}

if Undef > Undef {
    fail incorrect
}

if Undef >= String {
    fail incorrect
}

unless Undef >= Undef {
    fail incorrect
}

if Undef < String {
    fail incorrect
}

if Undef < Undef {
    fail incorrect
}

if Undef <= String {
    fail incorrect
}

unless Undef <= Undef {
    fail incorrect
}

# Variant tests

unless Variant == Variant and Variant[String] == Variant[String] and Variant[String, Integer] == Variant[Integer, String] and Variant[String, Numeric] != Variant[Integer, String] {
    fail incorrect
}

if Variant > Variant {
    fail incorrect
}

if Variant > Variant[String] {
    fail incorrect
}

unless Variant[Numeric] > Variant[Integer] {
    fail incorrect
}

unless Variant[Numeric, String] > Variant[String, Float] {
    fail incorrect
}

unless Variant[Numeric, String] > String {
    fail incorrect
}

unless Variant[Numeric, String] > Float {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Integer]] > String {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Regexp]] > Variant[Integer, Float] {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Regexp]] > Regexp {
    fail incorrect
}

unless Variant >= Variant {
    fail incorrect
}

if Variant >= Variant[String] {
    fail incorrect
}

unless Variant[Numeric] >= Variant[Integer] {
    fail incorrect
}

unless Variant[Numeric, String] >= Variant[String, Float] {
    fail incorrect
}

unless Variant[Numeric, String] >= String {
    fail incorrect
}

unless Variant[Numeric, String] >= Float {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Integer]] >= String {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Regexp]] >= Variant[Integer, Float] {
    fail incorrect
}

unless Variant[Numeric, Variant[String, Regexp]] >= Regexp {
    fail incorrect
}

if Variant < Variant {
    fail incorrect
}

unless Variant < Variant[String] {
    fail incorrect
}

if Variant[Numeric] < Variant[Integer] {
    fail incorrect
}

if Variant[Numeric, String] < Variant[String, Float] {
    fail incorrect
}

if Variant[Numeric, String] < String {
    fail incorrect
}

if Variant[Numeric, String] < Float {
    fail incorrect
}

if Variant[Numeric, Variant[String, Integer]] < String {
    fail incorrect
}

if Variant[Numeric, Variant[String, Regexp]] < Variant[Integer, Float] {
    fail incorrect
}

if Variant[Numeric, Variant[String, Regexp]] < Regexp {
    fail incorrect
}

unless Variant <= Variant {
    fail incorrect
}

unless Variant <= Variant[String] {
    fail incorrect
}

if Variant[Numeric] <= Variant[Integer] {
    fail incorrect
}

if Variant[Numeric, String] <= Variant[String, Float] {
    fail incorrect
}

if Variant[Numeric, String] <= String {
    fail incorrect
}

if Variant[Numeric, String] <= Float {
    fail incorrect
}

if Variant[Numeric, Variant[String, Integer]] <= String {
    fail incorrect
}

if Variant[Numeric, Variant[String, Regexp]] <= Variant[Integer, Float] {
    fail incorrect
}

if Variant[Numeric, Variant[String, Regexp]] <= Regexp {
    fail incorrect
}
