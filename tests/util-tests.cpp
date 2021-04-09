#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "util.h"
using namespace std;

TEST_CASE("Utility Functions - Misc", "[misc]") {
    REQUIRE( is_number("123") );
    REQUIRE( is_number("123.123") );

    string x =  " asd";
    REQUIRE( ltrim(x) == "asd" );
    x = " ";
    REQUIRE( ltrim(x) == "" );
}
