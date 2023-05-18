// Copyright (c) Martin Ridgers
// License: http://opensource.org/licenses/MIT

#include "pch.h"
#include "env_fixture.h"

#include <core/str.h>
#include <Windows.h>

//------------------------------------------------------------------------------
EnvFixture::EnvFixture(const char** env)
{
    // Store the Process' current environment so it can be restored.
    _env_strings = GetEnvironmentStringsW();
    convert_eq_to_null(_env_strings);

    clear();

    // Apply the Test environment.
    while (*env)
    {
        REQUIRE(SetEnvironmentVariable(env[0], env[1]) != FALSE);
        env += 2;
    }
}

//------------------------------------------------------------------------------
EnvFixture::~EnvFixture()
{
    clear();

    // Restore previous environment.
    wchar_t* env_string = _env_strings;
    while (*env_string)
    {
        wchar_t* value = env_string + (wcslen(env_string) + 1);
        REQUIRE(SetEnvironmentVariableW(env_string, value) != FALSE);
        env_string = value + wcslen(value) + 1;
    }

    FreeEnvironmentStringsW(_env_strings);
}

//------------------------------------------------------------------------------
void EnvFixture::convert_eq_to_null(wchar_t* env_strings)
{
    wchar_t* env_string = env_strings;
    while (*env_string)
    {
        wchar_t* eq = wcschr(env_string, '=');
        if (eq == env_string)
            eq = wcschr(env_string + 1, '='); // skips cmd's hidden "=X:" vars

        env_string += wcslen(env_string) + 1;

        REQUIRE(eq != nullptr);
        *eq = '\0';
    }
}

//------------------------------------------------------------------------------
void EnvFixture::clear()
{
    wchar_t* env_strings = GetEnvironmentStringsW();
    convert_eq_to_null(env_strings);

    wchar_t* env_string = env_strings;
    while (*env_string)
    {
        REQUIRE(SetEnvironmentVariableW(env_string, nullptr) != FALSE);
        env_string += wcslen(env_string) + 1; // name
        env_string += wcslen(env_string) + 1; // value
    }

    FreeEnvironmentStringsW(env_strings);

    // Confirm it is empty.
    wchar_t* env_is_empty = GetEnvironmentStringsW();
    REQUIRE(*env_is_empty == 0);
    FreeEnvironmentStringsW(env_is_empty);
}
