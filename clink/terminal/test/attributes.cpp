// Copyright (c) Martin Ridgers
// License: http://opensource.org/licenses/MIT

#include "pch.h"

#include <terminal/attributes.h>

//------------------------------------------------------------------------------
TEST_CASE("Attributes : empty")
{
    Attributes attr;
    REQUIRE(bool(attr.get_fg()) == false);
    REQUIRE(bool(attr.get_bg()) == false);
    REQUIRE(bool(attr.get_bold()) == false);
    REQUIRE(bool(attr.get_underline()) == false);
}

TEST_CASE("Attributes : default")
{
    Attributes attr = Attributes::defaults;
    REQUIRE(attr.get_fg());
    REQUIRE(attr.get_fg().is_default);
    REQUIRE(attr.get_bg());
    REQUIRE(attr.get_bg().is_default);
    REQUIRE(attr.get_bold());
    REQUIRE(attr.get_bold().value == 0);
    REQUIRE(attr.get_underline());
    REQUIRE(attr.get_underline().value == 0);
}

TEST_CASE("Attributes : merge")
{
    Attributes attr_a, attr_b;

    auto test_merge = [&] (void (*tester)(int32, Attributes)) {
        SECTION("merge(a,b)")
            tester(0, Attributes::merge(attr_a, attr_b));

        SECTION("merge(b,a)")
            tester(1, Attributes::merge(attr_b, attr_a));
    };

    attr_a = Attributes();
    attr_b = Attributes::defaults;
    test_merge([] (int32 pass, Attributes merged) {
        REQUIRE(merged.get_fg());
        REQUIRE(merged.get_fg().is_default);
        REQUIRE(merged.get_bg());
        REQUIRE(merged.get_bg().is_default);
    });

    attr_a = attr_b = Attributes();
    attr_a.set_fg(5);
    test_merge([] (int32 pass, Attributes merged) {
        REQUIRE(merged.get_fg());
        REQUIRE(merged.get_fg()->value == 5);
        REQUIRE(!merged.get_bg());
    });

    attr_a = attr_b = Attributes();
    attr_a.set_bg(6);
    test_merge([] (int32 pass, Attributes merged) {
        REQUIRE(merged.get_bg());
        REQUIRE(merged.get_bg()->value == 6);
        REQUIRE(!merged.get_fg());
    });

    attr_a = attr_b = Attributes();
    attr_a.set_bold(true);
    attr_a.set_underline(false);
    test_merge([] (int32 pass, Attributes merged) {
        REQUIRE(merged.get_bold());
        REQUIRE(merged.get_bold().value == true);
        REQUIRE(merged.get_underline());
        REQUIRE(merged.get_underline().value == false);
    });

    attr_a = attr_b = Attributes();
    attr_a.set_fg(14);
    attr_a.set_bold(false);
    attr_b.reset_bg();
    test_merge([] (int32 pass, Attributes merged) {
        REQUIRE(merged.get_fg());
        REQUIRE(merged.get_fg()->value == 14);
        REQUIRE(merged.get_bg());
        REQUIRE(merged.get_bg().is_default);
        REQUIRE(merged.get_bold());
        REQUIRE(merged.get_bold().value == false);
    });

    attr_a = attr_b = Attributes();
    attr_a.set_fg(12);
    attr_a.set_bold(true);
    attr_b.set_fg(13);
    attr_b.set_bold(false);
    test_merge([] (int32 pass, Attributes merged) {
        auto fg = merged.get_fg();
        REQUIRE(fg);
        REQUIRE(fg->value == (pass ? 12 : 13));

        auto bold = merged.get_bold();
        REQUIRE(bold);
        REQUIRE(bold.value == !!pass);
    });

    uint8 rgb[] = { 0, 127, 255, 0 };
    attr_a = attr_b = Attributes::defaults;
    attr_a.set_fg(rgb[0], rgb[1], rgb[2]);
    attr_a.set_bg(rgb[1], rgb[2], rgb[3]);

    test_merge([] (int32 pass, Attributes merged) {
        const auto fg = merged.get_fg(), bg = merged.get_bg();

        REQUIRE(fg);
        REQUIRE(fg.is_default == !pass);
        REQUIRE(bg);
        REQUIRE(bg.is_default == !pass);

        if (pass)
        {
            uint8 rgb[] = { 0, 127, 255, 0 };
            REQUIRE(fg->r == rgb[0] >> 3);
            REQUIRE(fg->g == rgb[1] >> 3);
            REQUIRE(fg->b == rgb[2] >> 3);
            REQUIRE(bg->r == rgb[1] >> 3);
            REQUIRE(bg->g == rgb[2] >> 3);
            REQUIRE(bg->b == rgb[3] >> 3);
        }
    });
}

TEST_CASE("Attributes : diff")
{
    Attributes attr_a, attr_b, diff;

    diff = Attributes::diff(Attributes::defaults, Attributes::defaults);
    REQUIRE(diff == Attributes());

    attr_a.set_fg(1);
    attr_a.reset_bg();
    diff = Attributes::diff(Attributes::defaults, attr_a);
    REQUIRE(diff != Attributes::defaults);
    REQUIRE(diff.get_fg());
    REQUIRE(diff.get_fg()->value == 1);
    REQUIRE(!diff.get_bg());
}
