// Copyright (c) Martin Ridgers
// License: http://opensource.org/licenses/MIT

#include "pch.h"
#include "env_fixture.h"
#include "fs_fixture.h"
#include "line_editor_tester.h"

#include <core/str_compare.h>
#include <lua/lua_match_generator.h>
#include <lua/lua_script_loader.h>
#include <lua/lua_state.h>

//------------------------------------------------------------------------------
TEST_CASE("Set command.")
{
    static const char* env_desc[] = {
        "simple",   "value",
        "case_map", "value",
        "dash-1",   "value",
        "dash_2",   "value",
        nullptr
    };

    FsFixture fs;
    EnvFixture env(env_desc);

    LuaState lua;
    LuaMatchGenerator lua_generator(lua);
    lua_load_script(lua, app, set);

    LineEditor::Desc desc;
    desc.word_delims = " =";
    LineEditorTester tester(desc);
    tester.get_editor()->add_generator(lua_generator);

    SECTION("Matches")
    {
        tester.set_input("set simp");
        tester.set_expected_matches("simple");
        tester.run();
    }

    SECTION("Suffixed output")
    {
        tester.set_input("set simp" DO_COMPLETE);
        tester.set_expected_output("set simple=");
        tester.run();
    }

    SECTION("Case mapped")
    {
        StrCompareScope _(StrCompareScope::relaxed);

        SECTION("One match")
        {
            tester.set_input("set case_m");
            tester.set_expected_matches("case_map");
            tester.run();
        }

        SECTION("Many Matches")
        {
            tester.set_input("set dash-");
            tester.set_expected_matches("dash-1", "dash_2");
            tester.run();
        }
    }

    SECTION("File matching after '='")
    {
        tester.get_editor()->add_generator(file_match_generator());

        SECTION("*")
        {
            tester.set_input("set simple=");
            tester.set_expected_matches("file1", "file2", "case_map-1", "case_map_2",
                "dir1\\", "dir2\\");
            tester.run();
        }

        SECTION("dir*")
        {
            tester.set_input("set simple=dir");
            tester.set_expected_matches("dir1\\", "dir2\\");
            tester.run();
        }

        SECTION("file*")
        {
            tester.set_input("set simple=dir1\\file");
            tester.set_expected_matches("file1", "file2");
            tester.run();
        }
    }
}
