#include "gtest/gtest.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <string>

#include "big_integer.h"

TEST(correctness, two_plus_two) {
  EXPECT_EQ(big_integer(4), big_integer(2) + big_integer(2));
  EXPECT_EQ(4, big_integer(2) + 2); // implicit conversion from int must work
  EXPECT_EQ(4, 2 + big_integer(2));
}

TEST(correctness, default_ctor) {
  big_integer a;
  big_integer b = 0;
  EXPECT_EQ(0, a);
  EXPECT_EQ(b, a);
}

TEST(correctness, ctor_limits) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = std::numeric_limits<int>::max();
  EXPECT_EQ(-1, a + b);
}

TEST(correctness, copy_ctor) {
  big_integer a = 3;
  big_integer b = a;

  EXPECT_EQ(b, a);
  EXPECT_EQ(3, b);
}

TEST(correctness, copy_ctor_real_copy) {
  big_integer a = 3;
  big_integer b = a;
  a = 5;

  EXPECT_EQ(3, b);
}

TEST(correctness, copy_ctor_real_copy2) {
  big_integer a = 3;
  big_integer b = a;
  b = 5;

  EXPECT_EQ(3, a);
}

TEST(correctness, ctor_invalid_string) {
  EXPECT_THROW(big_integer("abc"), std::invalid_argument);
  EXPECT_THROW(big_integer("123x"), std::invalid_argument);
  EXPECT_THROW(big_integer(""), std::invalid_argument);
  EXPECT_THROW(big_integer("-"), std::invalid_argument);
  EXPECT_THROW(big_integer("-x"), std::invalid_argument);
  EXPECT_THROW(big_integer("123-456"), std::invalid_argument);
  EXPECT_THROW(big_integer("--5"), std::invalid_argument);
  EXPECT_THROW(big_integer("++5"), std::invalid_argument);
}

TEST(correctness, assignment_operator) {
  big_integer a = 4;
  big_integer b = 7;
  b = a;

  EXPECT_TRUE(a == b);
}

TEST(correctness, self_assignment) {
  big_integer a = 5;
  a = a;

  EXPECT_TRUE(a == 5);
}

TEST(correctness, assignment_return_value) {
  big_integer a = 4;
  big_integer b = 7;
  (a = b) = a;

  EXPECT_TRUE(a == 7);
  EXPECT_TRUE(b == 7);
}

TEST(correctness, comparisons) {
  big_integer a = 100;
  big_integer b = 100;
  big_integer c = 200;
  big_integer d = -1;

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a != c);
  EXPECT_TRUE(a < c);
  EXPECT_TRUE(c > a);
  EXPECT_TRUE(a <= a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a <= c);
  EXPECT_TRUE(c >= a);
  EXPECT_TRUE(d != 0);
}

TEST(correctness, compare_zero_and_minus_zero) {
  big_integer a;
  big_integer b = -a;

  EXPECT_TRUE(a == b);
}

TEST(correctness, add) {
  big_integer a = 5;
  big_integer b = 20;

  EXPECT_TRUE(a + b == 25);

  a += b;
  EXPECT_TRUE(a == 25);
}

TEST(correctness, add_signed) {
  big_integer a = 5;
  big_integer b = -20;
  EXPECT_TRUE(a + b == -15);

  a += b;
  EXPECT_TRUE(a == -15);
}

TEST(correctness, add_return_value) {
  big_integer a = 5;
  big_integer b = 1;

  (a += b) += b;
  EXPECT_EQ(7, a);
}

TEST(correctness, sub) {
  big_integer a = 20;
  big_integer b = 5;

  EXPECT_TRUE(a - b == 15);

  a -= b;
  EXPECT_TRUE(a == 15);
}

TEST(correctness, sub_signed) {
  big_integer a = 5;
  big_integer b = 20;

  EXPECT_TRUE(a - b == -15);

  a -= b;
  EXPECT_TRUE(a == -15);

  a -= -100;
  EXPECT_TRUE(a == 85);
}

TEST(correctness, sub_return_value) {
  big_integer a = 5;
  big_integer b = 1;

  (a -= b) -= b;
  EXPECT_EQ(3, a);
}

TEST(correctness, mul) {
  big_integer a = 5;
  big_integer b = 20;
  EXPECT_TRUE(a * b == 100);

  a *= b;
  EXPECT_TRUE(a == 100);
}

TEST(correctness, mul_signed) {
  big_integer a = -5;
  big_integer b = 20;

  EXPECT_TRUE(a * b == -100);

  a *= b;
  EXPECT_TRUE(a == -100);
}

TEST(correctness, mul_return_value) {
  big_integer a = 5;
  big_integer b = 2;

  (a *= b) *= b;
  EXPECT_EQ(20, a);
}

TEST(correctness, div_) {
  big_integer a = 20;
  big_integer b = 5;
  big_integer c = 20;
  EXPECT_EQ(0, b / c);
  EXPECT_TRUE(a / b == 4);
  EXPECT_TRUE(a % b == 0);

  a /= b;
  EXPECT_TRUE(a == 4);

  c %= b;
  EXPECT_TRUE(c == 0);
}

TEST(correctness, div_int_min) {
  big_integer a = std::numeric_limits<int>::min();
  EXPECT_TRUE((a / a) == (a / std::numeric_limits<int>::min()));
}

TEST(correctness, div_int_min_2) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = -1;
  big_integer c = a / b;
  EXPECT_TRUE(c == (a / -1));
  EXPECT_TRUE((c - std::numeric_limits<int>::max()) == 1);
}

TEST(correctness, div_signed) {
  big_integer a = 0;
  big_integer b = 5;

  EXPECT_TRUE(a / b == 0);
 // EXPECT_TRUE(a % b == 0);
}

TEST(correctness, binary_operations) {
  big_integer x = big_integer("80834881818236391177723050839925923675652324008047822995592133407801164689064475307470399");
  big_integer y = big_integer("-58544110951486270123346421121930492891693376010340507820033559925280205540293865007676233753363701814911795445385503665751639533072281529933869741926273332823551598776741271815273181082351346809536113341068833967430584423025923271188504565373217782363152993162977528400394311825809106618940786333238274944071877141963339349476481090750104126106334602914922122015788434844464839955019115688580389390932070720376067429738759517219269673908335503688515081915738467663427645428855654899899713014937202720354952944213731509622046145758542588595411891450291011716726507309495013699630341911493564112745466696829341127748996170237569474388467168936699638007470678279028148452829670501868388512844102104122521867865207254682974599422099432467007708430323714432692374538460161030455157579655841188848734202888553142651711096270518107651871546942718299953605965699561698702604199662518798811293414482073238092296075811923555286953360214319493816185128832789029029210979511662484978472702652842486515981404899395927085833350839931673469170022111148025836497747292406999603981405819293663284124398904384937891830908749475790660393532531665199134851212807430036961");
  big_integer z = big_integer("-987654321");
  big_integer f = big_integer("-18765432123456");
  EXPECT_EQ(x & y, big_integer("63925584384682705619550611873571322645213317817058774333347956771491932085653040873513503"));
  EXPECT_EQ(x | y, big_integer("-58544110951486270123346421121930492891693376010340507820033559925280205540293865007676233753363701814911795445385503665751639533072281529933869741926273332823551598776741271815273181082351346809536113341068833967430584423025923271188504565373217782363152993162977528400394311825809106618940786333238274944071877141963339349476481090750104126106334602914922122015788434844464839955019115688580389390932070720376067429738759517219269673908335503688515081915738467663427645428855654899899713014937202720354952944213731509622046145758542588595411891450291011716726507309495013699630341911493564112745466696829341127748996170237569474388467168936699638007470678279028148452829670501868388512844102104122521867865207254682974599422099432467007708430323714432692374538460161030455157579655841188848734202888553142651711096270518107651871546942718299953605965699561698702604199662518798811293414482073238092296075811923555286953360214319493816185128832789029029210979511662484978472702652842486515981404899395927085833350839931673469170022111148025836497747292406999603964496521860109598566226465418583290800469743284801611731288355028889902247801372996080065"));
  EXPECT_EQ(x ^ y, big_integer("-58544110951486270123346421121930492891693376010340507820033559925280205540293865007676233753363701814911795445385503665751639533072281529933869741926273332823551598776741271815273181082351346809536113341068833967430584423025923271188504565373217782363152993162977528400394311825809106618940786333238274944071877141963339349476481090750104126106334602914922122015788434844464839955019115688580389390932070720376067429738759517219269673908335503688515081915738467663427645428855654899899713014937202720354952944213731509622046145758542588595411891450291011716726507309495013699630341911493564112745466696829341127748996170237569474388467168936699638007470678279028148452829670501868388512844102104122521867865207254682974599422099432467007708430323714432692374538460161030455157579655841188848734202888553142651711096270518107651871546942718299953605965699561698702604199662518798811293414482073238092296075811923555286953360214319493816185128832789029029210979511662484978472702652842486515981404899395927085833350839931673469170022111148025836497747292406999604028422106244792304185777077292154613445683061101860386064636311800381834333454413869593568"));
  EXPECT_EQ(z ^ f, 18764983965839LL);
  EXPECT_EQ(f ^ z, 18764983965839LL);
  EXPECT_EQ(f ^ f, 0);
}
TEST(correctness, my_div_1) {
  big_integer a =
      big_integer("1919191919191919191827354545366364536363636363636633");
  big_integer b = big_integer("1");
  big_integer c =
      big_integer("111111111111111111111111111111111111111111111111");
  big_integer d = big_integer("4352617853627185362781");
  big_integer e = big_integer("10198191881187654567898765434567654345678765");

  EXPECT_EQ(a / b, a);
  EXPECT_EQ(a / c, big_integer(17272));
  EXPECT_EQ(a / d, big_integer("440928191661161089296294564582"));
  EXPECT_EQ(
      a * b * c / e,
      big_integer("20909936688899221651043718820282488964465655023366654804"));
}

TEST(correctness, my_mod_1) {
  big_integer a =
      big_integer("1919191919191919191827354545366364536363636363636633");
  big_integer b = big_integer("1");
  big_integer c =
      big_integer("111111111111111111111111111111111111111111111111");
  big_integer d = big_integer("4352617853627185362781");
  big_integer e = big_integer("10198191881187654567898765434567654345678765");

  EXPECT_EQ(e % d, big_integer("2103848651706646735320"));
  EXPECT_EQ(e / 4, big_integer("2549547970296913641974691358641913586419691"));
  EXPECT_EQ(
      a * c * c * a / (e * e),
      big_integer("437225452333773744928414578882067580664581351939370890979232"
                  "441660673637826672292038761350352143570137899453264"));
  EXPECT_EQ(a * c * c * a * d % (e * e),
            big_integer("339997279209697187565075004022094401556352324798912886"
                        "33560734433968795825317240012589"));
}

TEST(correctness, performance_division) {
  big_integer x = big_integer("1" + std::string(10000, '0') + "0");
  big_integer y = big_integer("1" + std::string(10000, '0'));
  EXPECT_EQ(x / y, 10);
  EXPECT_EQ(x % y, 0);
}

TEST(correctness, performance_division_2) {
  big_integer x = big_integer("1" + std::string(100000, '0'));
  big_integer y = big_integer("1" + std::string(10000, '0'));
  big_integer z = x / y;
}

TEST(correctness, performance_division_3) {
  big_integer x = big_integer("1" + std::string(100000, '0'));
  big_integer y = big_integer("1" + std::string(12, '0'));
  big_integer z = x / y;
}

TEST(correctness, division_signed) {
  big_integer x = 10;
  EXPECT_EQ(x / (-5), -2);
  EXPECT_EQ(-x / 5, -2);
  EXPECT_EQ(-x / -5, 2);
  EXPECT_EQ(big_integer("-9876543456787654567") / big_integer("5434567"),
            big_integer("-1817356094199"));
}

TEST(correctness, complex_sum_add_mul_division_test) {
  big_integer x =
      big_integer("9876543234567898765434567898765432345678987654345678");
  big_integer y = big_integer("822228282828288282822888282");
  EXPECT_EQ(x / y * y * x - x - y + x + y * y - x * x + x / y,
            big_integer("-13983804262066374872724677270537764709029877699237268"
                        "66237556813577236073205227"));
}

TEST(correctness, small_division) {
  EXPECT_EQ(big_integer("10") / 5, big_integer("2"));
  EXPECT_EQ(big_integer("1000000000000000000000000000") / 3,
            big_integer("333333333333333333333333333"));
  EXPECT_EQ(big_integer("1") / 20000000, big_integer("0"));
}

TEST(correctness, div_rounding) {
  big_integer a = 23;
  big_integer b = 5;

  EXPECT_TRUE(a / b == 4);
  EXPECT_TRUE(a % b == 3);
}

TEST(correctness, div_rounding_negative) {
  big_integer a = 23;
  big_integer b = -5;
  big_integer c = -23;
  big_integer d = 5;

  EXPECT_TRUE(a / b == -4);
  EXPECT_TRUE(c / d == -4);
  EXPECT_TRUE(a % b == 3);
  EXPECT_TRUE(c % d == -3);
}

TEST(correctness, div_return_value) {
  big_integer a = 100;
  big_integer b = 2;

  (a /= b) /= b;
  EXPECT_EQ(25, a);
}

TEST(correctness, unary_plus) {
  big_integer a = 123;
  big_integer b = +a;

  EXPECT_TRUE(a == b);

  // this code should not compile:
  // &+a;
}

TEST(correctness, negation) {
  big_integer a = 666;
  big_integer b = -a;

  EXPECT_TRUE(b == -666);
}

TEST(correctness, negation_int_min) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = -a;

  EXPECT_EQ(b - 1, std::numeric_limits<int>::max());
}

TEST(correctness, increment) {
  big_integer a = 42;
  big_integer pre = ++a;
  big_integer post = a++;

  EXPECT_EQ(43, pre);
  EXPECT_EQ(43, post);
}

TEST(correctness, decrement) {
  big_integer a = 42;
  big_integer pre = --a;
  big_integer post = a--;

  EXPECT_EQ(41, pre);
  EXPECT_EQ(41, post);
}

TEST(correctness, and_) {
  big_integer a = 0x55;
  big_integer b = 0xaa;

  EXPECT_TRUE((a & b) == 0);
  EXPECT_TRUE((a & 0xcc) == 0x44);
  a &= b;
  EXPECT_TRUE(a == 0);
}

TEST(correctness, and_signed) {
  big_integer a = 0x55;
  big_integer b = 0xaa;
  big_integer x = a & (0xaa - 256);

  EXPECT_TRUE((b & -1) == 0xaa);
  EXPECT_TRUE((a & (0xaa - 256)) == 0);
  EXPECT_TRUE((a & (0xcc - 256)) == 0x44);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c & d, big_integer(0x44));
}

TEST(correctness, and_signed2) {
  big_integer a("-18446744073709551615"); // -((1 << 64) - 1)
  big_integer b("147573952589676412929"); // (1 << 67)

  EXPECT_EQ(b, a & b);
}

TEST(correctness, and_return_value) {
  big_integer a = 7;

  (a &= 3) &= 6;
  EXPECT_EQ(2, a);
}

TEST(correctness, or_) {
  big_integer a = 0x55;
  big_integer b = 0xaa;

  EXPECT_TRUE((a | b) == 0xff);
  a |= b;
  EXPECT_TRUE(a == 0xff);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c | d, big_integer(0xdd));
}

TEST(correctness, or_signed) {
  big_integer a = 0x55;
  big_integer b = 0xaa;
  EXPECT_TRUE((a | (b - 256)) == -1);
}

TEST(correctness, or_signed2) {
  big_integer a("-36893488147419103232"); // -(1 << 65)
  big_integer b("147573952589676412928"); //  (1 << 67)

  EXPECT_EQ(a, a | b);
}

TEST(correctness, or_return_value) {
  big_integer a = 1;

  (a |= 2) |= 4;
  EXPECT_EQ(7, a);
}

TEST(correctness, xor_) {
  big_integer a = 0xaa;
  big_integer b = 0xcc;

  EXPECT_TRUE((a ^ b) == 0x66);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c ^ d, big_integer(0x99));
}

TEST(correctness, xor_signed) {
  big_integer a = 0xaa;
  big_integer b = 0xcc;

  EXPECT_TRUE((a ^ (b - 256)) == (0x66 - 256));
}

TEST(correctness, xor_return_value) {
  big_integer a = 1;

  (a ^= 2) ^= 1;
  EXPECT_EQ(2, a);
}

TEST(correctness, not_) {
  big_integer a = 0xaa;
  big_integer b = ~a;
  big_integer c = (-a - 1);
  EXPECT_TRUE(~a == (-a - 1));
}

TEST(correctness, shl_) {
  big_integer a = 23;

  EXPECT_TRUE((a << 5) == 23 * 32);

  a <<= 5;
  EXPECT_TRUE(a == 23 * 32);
}

TEST(correctness, shl_return_value) {
  big_integer a = 1;

  (a <<= 2) <<= 1;
  EXPECT_EQ(8, a);
}

TEST(correctness, shr_) {
  big_integer a = 23;

  EXPECT_EQ(5, a >> 2);

  a >>= 2;
  EXPECT_EQ(5, a);
}

TEST(correctness, shr_31) {
  big_integer a = 65536;

  EXPECT_EQ(2, (a * a) >> 31);
}

TEST(correctness, shr_32) {
  big_integer a = 1;
  EXPECT_EQ(0, a >> 32);
}

TEST(correctness, shr_signed) {
  big_integer a = -1234;

  EXPECT_EQ(-155, a >> 3);

  a >>= 3;
  EXPECT_EQ(-155, a);
}

TEST(correctness, shr_return_value) {
  big_integer a = 64;

  (a >>= 2) >>= 1;
  EXPECT_EQ(8, a);
}

TEST(correctness, add_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("10000000000000000000000000000000000000000000000000000100000000"
                "000000000000000000000000000000");

  EXPECT_EQ(c, a + b);
}

TEST(correctness, add_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("10000000000000000000000000000000000000000000000000000000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(0, a + b);
}

TEST(correctness, add_long_signed2) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-9999999999999999999999999999999999999999999999999999000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(c, a + b);
}

TEST(correctness, add_long_pow2) {
  big_integer a("18446744073709551616");
  big_integer b("-18446744073709551616");
  big_integer c("36893488147419103232");

  EXPECT_EQ(c, a + a);
  EXPECT_EQ(a, b + c);
  EXPECT_EQ(a, c + b);
}

TEST(correctness, sub_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("99999999999999999999999999999999999999999999999999999000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(c, a - b);
}

TEST(correctness, sub_long_pow2) {
  big_integer a("36893488147419103232");
  big_integer b("36893488147419103231");

  EXPECT_EQ(1, a - b);
}

TEST(correctness, mul_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000"
                "00000000000000000000000000000000000000");

  EXPECT_EQ(c, a * b);
}

TEST(correctness, mul_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000"
                "00000000000000000000000000000000000000");

  EXPECT_EQ(c, a * b);
}

TEST(correctness, mul_long_signed2) {
  big_integer a("-100000000000000000000000000");
  big_integer c("100000000000000000000000000"
                "00000000000000000000000000");

  EXPECT_EQ(c, a * a);
}

TEST(correctness, mul_long_pow2) {
  big_integer a("18446744073709551616");
  big_integer b("340282366920938463463374607431768211456");
  big_integer c("11579208923731619542357098500868790785326998466564056403945758"
                "4007913129639936");

  EXPECT_EQ(b, a * a);
  EXPECT_EQ(c, b * b);
}

TEST(correctness, div_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, div_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, div_long_signed2) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000");
  big_integer b("-100000000000000000000000000000000000000");
  big_integer c("100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, negation_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000");
  big_integer c("-10000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(-a, c);
  EXPECT_EQ(a, -c);
}

TEST(correctness, shl_long) {
  EXPECT_EQ(
      big_integer("1091951238831590836520041079875950759639875963123939936"),
      big_integer("34123476213487213641251283746123461238746123847623123")
          << 5);

  EXPECT_EQ(
      big_integer(
          "-104637598388784443044449444577438556334703518260785595038524928"),
      big_integer(
          "-817481237412378461284761285761238721364871236412387461238476")
          << 7);

  EXPECT_EQ(
      big_integer("265026033927139132419699023286961165415504134688699829142473"
                  "84891392"),
      big_integer("12341236412857618761234871264871264128736412836643859238479")
          << 31);
}

TEST(correctness, shr_long) {
  EXPECT_EQ(
      big_integer(
          "4730073393008085198307104580698364137020387111323398632330851"),
      big_integer(
          "151362348576258726345827346582347652384652387562348756234587245") >>
          5);

  EXPECT_EQ(
      big_integer("1118311528397465815295799577134738919815767762822175104787"),
      big_integer(
          "143143875634875624357862345873246581736418273641238413412741") >>
          7);

  EXPECT_EQ(
      big_integer("-1591563309890326054125627839548891585559049824963"),
      big_integer(
          "-3417856182746231874623148723164812376512852437523846123876") >>
          31);
}

TEST(correctness, string_conv) {
  EXPECT_EQ("100", to_string(big_integer("100")));
  EXPECT_EQ("100", to_string(big_integer("0100")));
  EXPECT_EQ("0", to_string(big_integer("0")));
  EXPECT_EQ("0", to_string(big_integer("-0")));
  EXPECT_EQ("-1000000000000000", to_string(big_integer("-1000000000000000")));

  big_integer lim = std::numeric_limits<int32_t>::max();
  EXPECT_EQ("2147483647", to_string(lim));
  lim++;
  EXPECT_EQ("2147483648", to_string(lim));
  lim = std::numeric_limits<int32_t>::min();
  EXPECT_EQ("-2147483648", to_string(lim));
  lim--;
  EXPECT_EQ("-2147483649", to_string(lim));
}

namespace {
template <typename T>
void test_converting_ctor(T value) {
  using std::to_string;

  big_integer bi = value;
  EXPECT_EQ(to_string(value), to_string(bi));
}
} // namespace

TEST(correctness, converting_ctor) {
  using std::numeric_limits;

  test_converting_ctor(numeric_limits<short>::min());
  test_converting_ctor(numeric_limits<short>::max());
  test_converting_ctor(numeric_limits<unsigned short>::min());
  test_converting_ctor(numeric_limits<unsigned short>::max());

  test_converting_ctor(numeric_limits<int>::min());
  test_converting_ctor(numeric_limits<int>::max());
  test_converting_ctor(numeric_limits<unsigned>::min());
  test_converting_ctor(numeric_limits<unsigned>::max());

  test_converting_ctor(numeric_limits<long>::min());
  test_converting_ctor(numeric_limits<long>::max());
  test_converting_ctor(numeric_limits<unsigned long>::min());
  test_converting_ctor(numeric_limits<unsigned long>::max());

  test_converting_ctor(numeric_limits<long long>::min());
  test_converting_ctor(numeric_limits<long long>::max());
  test_converting_ctor(numeric_limits<unsigned long long>::min());
  test_converting_ctor(numeric_limits<unsigned long long>::max());
}

TEST(correctness, converting_ctor2) {
  big_integer a(1);
  big_integer b(1U);
  big_integer c(1L);
  big_integer d(1UL);
  big_integer e(1LL);
  big_integer f(1ULL);

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a == d);
  EXPECT_TRUE(a == e);
  EXPECT_TRUE(a == f);
}

TEST(correctness, converting_ctor3) {
  big_integer a(-1);
  big_integer b(-1L);
  big_integer c(-1LL);

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == c);
}

TEST(correctness, converting_ctor4) {
  auto num = static_cast<long long>(-1U) << 32;
  big_integer bignum(num);

  EXPECT_EQ(to_string(bignum), std::to_string(num));
}
