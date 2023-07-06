// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>

using namespace jsoncons;

TEST_CASE("ojson insert(first,last) test")
{
    SECTION("copy map into ojson")
    {
        std::map<std::string,double> m1 = {{"f",4},{"e",5},{"d",6}};
        std::map<std::string,double> m2 = {{"c",1},{"b",2},{"a",3}};

        ojson doc;
        doc.insert(m1.begin(),m1.end());
        doc.insert(m2.begin(),m2.end());

        //std::cout << doc << "\n";

        REQUIRE(doc.size() == 6);
        auto it = doc.object_range().begin();
        CHECK(it++->key() == "d");
        CHECK(it++->key() == "e");
        CHECK(it++->key() == "f");
        CHECK(it++->key() == "a");
        CHECK(it++->key() == "b");
        CHECK(it++->key() == "c");
    }
}

TEST_CASE("ojson parse_duplicate_names")
{
    ojson oj1 = ojson::parse(R"({"first":1,"second":2,"third":3})");
    CHECK(3 == oj1.size());
    CHECK(1 == oj1["first"].as<int>());
    CHECK(2 == oj1["second"].as<int>());
    CHECK(3 == oj1["third"].as<int>());

    ojson oj2 = ojson::parse(R"({"first":1,"second":2,"first":3})");
    CHECK(2 == oj2.size());
    CHECK(1 == oj2["first"].as<int>());
    CHECK(2 == oj2["second"].as<int>());
}

TEST_CASE("ojson object erase with iterator")
{
    SECTION("ojson erase with iterator")
    {
        ojson doc(jsoncons::json_object_arg);

        doc.try_emplace("a", 1);
        doc.try_emplace("b", 2);
        doc.try_emplace("c", 3);

        auto it = doc.object_range().begin();
        while (it != doc.object_range().end())
        {
            if (it->key() == "a" || it->key() == "c")
            {
                it = doc.erase(it);
            }
            else
            {
                it++;
            }
        }

        CHECK(doc.size() == 1);
        CHECK(doc.at("b") == 2);
        CHECK(doc["b"] == 2);
    }

    SECTION("ojson erase with iterator 2")
    {
        ojson doc(jsoncons::json_object_arg);

        doc.try_emplace("a", 1);
        doc.try_emplace("b", 2);
        doc.try_emplace("c", 3);

        auto it = doc.object_range().begin();
        while (it != doc.object_range().end())
        {
            if (it->key() == "a")
            {
                it = doc.erase(it, it+2);
            }
            else
            {
                it++;
            }
        }

        CHECK(doc.size() == 1);
        CHECK(doc.at("c") == 3);
        CHECK(doc["c"] == 3);
    }

    SECTION("ojson erase with iterator 3")
    {
        ojson doc(jsoncons::json_object_arg);

        doc.try_emplace("c", 1);
        doc.try_emplace("b", 2);
        doc.try_emplace("a", 3);

        auto it = doc.object_range().begin();
        while (it != doc.object_range().end())
        {
            if (it->key() == "c")
            {
                it = doc.erase(it, it+2);
            }
            else
            {
                it++;
            }
        }

        CHECK(doc.size() == 1);
        CHECK(doc.at("a") == 3);
        CHECK(doc["a"] == 3);
    }
}


TEST_CASE("test_ojson_merge")
{
    ojson doc = ojson::parse(R"(
    {
        "a" : 1,
        "b" : 2
    }
    )");

    const ojson source = ojson::parse(R"(
    {
        "a" : 2,
        "c" : 3,
        "d" : 4,
        "b" : 5,
        "e" : 6
    }
    )");

    SECTION("merge doc with source")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 1,
            "b" : 2,
            "c" : 3,
            "d" : 4,
            "e" : 6
        }
        )");
        doc.merge(source);
        CHECK(expected == doc);
    }

    SECTION("merge doc")
    {
        const ojson expected = ojson::parse(R"(
{"a":1,"b":2,"c":3,"d":4,"e":6}
        )");
        doc.merge(doc.object_range().begin()+1,source);
        CHECK(expected == doc);
    }

    //std::cout << doc << std::endl;
}

TEST_CASE("test_ojson_merge_move")
{
    ojson doc = ojson::parse(R"(
    {
        "a" : "1",
        "d" : [1,2,3]
    }
    )");

    ojson source = ojson::parse(R"(
    {
        "a" : "2",
        "c" : [4,5,6]
    }
    )");

    SECTION("merge source into doc")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "1",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        doc.merge(std::move(source));
        CHECK(expected == doc);
    }
    SECTION("merge source into doc at begin")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "1",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        doc.merge(doc.object_range().begin(),std::move(source));
        CHECK(expected == doc);
    }


    //std::cout << "(1)\n" << doc << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}

TEST_CASE("ojson merge_or_update test")
{
    ojson doc = ojson::parse(R"(
    {
        "a" : 1,
        "b" : 2
    }
    )");

    const ojson source = ojson::parse(R"(
    {
        "a" : 2,
        "c" : 3
    }
    )");

    SECTION("merge_or_update source into doc")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 2,
            "b" : 2,
            "c" : 3
        }
        )");
        doc.merge_or_update(source);
        CHECK(expected == doc);
    }

    SECTION("merge_or_update source into doc at pos 1")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 2,
            "b" : 2,
            "c" : 3
        }
        )");
        doc.merge_or_update(doc.object_range().begin()+1,source);
        CHECK(expected == doc);
    }

    //std::cout << doc << std::endl;
}

TEST_CASE("test_ojson_merge_or_update_move")
{
    ojson doc = ojson::parse(R"(
    {
        "a" : "1",
        "d" : [1,2,3]
    }
    )");

    ojson source = ojson::parse(R"(
    {
        "a" : "2",
        "c" : [4,5,6]
    }
    )");

    SECTION("merge or update doc from source")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "2",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        doc.merge_or_update(std::move(source));
        //CHECK(expected == doc);
    }
    SECTION("merge or update doc from source at pos")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "2",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        doc.merge_or_update(doc.object_range().begin(),std::move(source));
        CHECK(expected == doc);
    }
}

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)

#include <memory_resource> 
using namespace jsoncons;

using pmr_json = jsoncons::pmr::json;
using pmr_ojson = jsoncons::pmr::ojson;

TEST_CASE("ojson.merge test with stateful allocator")
{
    char buffer1[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
    std::pmr::polymorphic_allocator<char> alloc1(&pool1);

    char buffer2[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
    std::pmr::polymorphic_allocator<char> alloc2(&pool2);
}

#endif
