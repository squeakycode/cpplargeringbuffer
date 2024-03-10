#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <cpplargeringbuffer/cpplargeringbuffer.hpp>
#include <string>

TEST_CASE("large_ring_buffer defaults", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<int> testee;

    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 0);
    CHECK(testee.get_segment_size() == 0);
    CHECK(testee.get_segment_count() == 0);
    CHECK(testee.get_used_segments() == 0);
    CHECK_NOTHROW(testee.clear());
}

TEST_CASE("large_ring_buffer defaults const", "[large_ring_buffer]")
{
    const cpplargeringbuffer::large_ring_buffer<int> testee;

    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 0);
    CHECK(testee.get_segment_size() == 0);
    CHECK(testee.get_segment_count() == 0);
    CHECK(testee.get_used_segments() == 0);
}

TEST_CASE("large_ring_buffer single item ring buffer back", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<int> testee(1, 1);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 1);
    CHECK(testee.get_segment_size() == 1);
    CHECK(testee.get_segment_count() == 1);
    CHECK(testee.get_used_segments() == 0);
    CHECK_NOTHROW(testee.clear());

    testee.push_back(1);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 1);
    CHECK(testee.back() == 1);

    testee.push_back(2);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 2);
    CHECK(testee.back() == 2);

    testee.pop_back();
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);

    testee.push_back(1);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 1);
    CHECK(testee.back() == 1);
}

TEST_CASE("large_ring_buffer single item ring buffer front", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<int> testee(1, 1);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 1);
    CHECK(testee.get_segment_size() == 1);
    CHECK(testee.get_segment_count() == 1);
    CHECK(testee.get_used_segments() == 0);
    CHECK_NOTHROW(testee.clear());

    testee.push_front(1);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 1);
    CHECK(testee.back() == 1);

    testee.push_front(2);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 2);
    CHECK(testee.back() == 2);

    testee.pop_front();
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);

    testee.push_front(1);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.front() == 1);
    CHECK(testee.back() == 1);
}

template <typename testee_type>
inline void checkValueRange(const testee_type& testee, size_t start_value, size_t count)
{
    REQUIRE(count == testee.size());
    for (size_t i = 0; i < count; ++i)
    {
        CHECK(testee[i] == start_value + i);
        CHECK(testee.at(i) == start_value + i);
    }
}

TEST_CASE("large_ring_buffer unused segment removal front", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_back(i);
    }
    //3
    testee.pop_front();
    testee.pop_front();
    testee.pop_front();
    checkValueRange(testee, 6, 12);
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    //6
    testee.pop_front();
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count()); //segment adjacent to end is not removed
    checkValueRange(testee, 9, 9);
    //9
    testee.pop_front();
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 1);
    checkValueRange(testee, 12, 6);
    //12
    testee.pop_front();
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 1);
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 2);
    checkValueRange(testee, 15, 3);
    //15
    testee.pop_front();
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 2);
    testee.pop_front();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 3);
}

TEST_CASE("large_ring_buffer unused segment removal back", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_back(i);
    }
    //3
    testee.pop_back();
    testee.pop_back();
    testee.pop_back();
    checkValueRange(testee, 3, 12);
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    //6
    testee.pop_back();
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count()); //segment adjacent to start is not removed
    checkValueRange(testee, 3, 9);
    //9
    testee.pop_back();
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count());
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 1);
    checkValueRange(testee, 3, 6);
    //12
    testee.pop_back();
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 1);
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 2);
    checkValueRange(testee, 3, 3);
    //15
    testee.pop_back();
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 2);
    testee.pop_back();
    CHECK(testee.get_used_segments() == testee.get_segment_count() - 3);
}

TEST_CASE("large_ring_buffer operation", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<int> testee(3,2);
    //int testValue = 1;
    int* pItem1 = nullptr;
    int* pItem2 = nullptr;
    int* pItem3 = nullptr;
    int* pItem4 = nullptr;
    int* pItem5 = nullptr;
    int* pItem6 = nullptr;

    //1
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 6);
    CHECK(testee.get_segment_size() == 2);
    CHECK(testee.get_segment_count() == 3);
    CHECK(testee.get_used_segments() == 0);
    pItem1 = &testee.extend_back();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 1);
    CHECK(testee.get_used_segments() == 1);
    CHECK(pItem1 != nullptr);
    CHECK(*pItem1 == 0);
    CHECK(pItem1 == &testee.front());
    CHECK(pItem1 == &testee.back());
    testee.back() = 1;
    CHECK(*pItem1 == 1);

    //2
    testee.push_back(2);
    pItem2 = &testee.back();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 2);
    CHECK(testee.get_used_segments() == 1);
    CHECK(pItem1 == &testee.front());
    CHECK(pItem1 != &testee.back());
    CHECK(testee.back() == 2);
    CHECK(*pItem2 == 2);
    CHECK(testee.at(0) == 1);
    CHECK(testee.at(1) == 2);
    CHECK(testee[0] == 1);
    CHECK(testee[1] == 2);
    CHECK(&testee[0] == pItem1);
    CHECK(&testee[1] == pItem2);

    //3
    testee.push_back(3);
    pItem3 = &testee.back();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 3);
    CHECK(testee.get_used_segments() == 2);
    CHECK(pItem1 == &testee.front());
    CHECK(pItem1 != &testee.back());
    CHECK(testee.back() == 3);
    CHECK(*pItem3 == 3);
    CHECK(testee[0] == 1);
    CHECK(testee[1] == 2);
    CHECK(testee[2] == 3);
    CHECK(&testee[0] == pItem1);
    CHECK(&testee[1] == pItem2);
    CHECK(&testee[2] == pItem3);

    //6
    testee.push_front(6);
    pItem6 = &testee.front();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 4);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem6 == &testee.front());
    CHECK(pItem3 == &testee.back());
    CHECK(testee.back() == 3);
    CHECK(*pItem6 == 6);
    CHECK(testee[0] == 6);
    CHECK(testee[1] == 1);
    CHECK(testee[2] == 2);
    CHECK(testee[3] == 3);
    CHECK(&testee[0] == pItem6);
    CHECK(&testee[1] == pItem1);
    CHECK(&testee[2] == pItem2);
    CHECK(&testee[3] == pItem3);

    //5
    testee.extend_front() = 5;
    pItem5 = &testee.front();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 5);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem5 == &testee.front());
    CHECK(pItem3 == &testee.back());
    CHECK(testee.back() == 3);
    CHECK(*pItem5 == 5);
    CHECK(testee[0] == 5);
    CHECK(testee[1] == 6);
    CHECK(testee[2] == 1);
    CHECK(testee[3] == 2);
    CHECK(testee[4] == 3);
    CHECK(&testee[0] == pItem5);
    CHECK(&testee[1] == pItem6);
    CHECK(&testee[2] == pItem1);
    CHECK(&testee[3] == pItem2);
    CHECK(&testee[4] == pItem3);

    //4
    testee.extend_front() = 4;
    pItem4 = &testee.front();
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 6);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem4 == &testee.front());
    CHECK(pItem3 == &testee.back());
    CHECK(testee.back() == 3);
    CHECK(*pItem4 == 4);
    CHECK(testee[0] == 4);
    CHECK(testee[1] == 5);
    CHECK(testee[2] == 6);
    CHECK(testee[3] == 1);
    CHECK(testee[4] == 2);
    CHECK(testee[5] == 3);
    CHECK(&testee[0] == pItem4);
    CHECK(&testee[1] == pItem5);
    CHECK(&testee[2] == pItem6);
    CHECK(&testee[3] == pItem1);
    CHECK(&testee[4] == pItem2);
    CHECK(&testee[5] == pItem3);

    //7
    testee.extend_front() = 7;
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 6);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem3 == &testee.front());
    CHECK(pItem2 == &testee.back());
    CHECK(testee[0] == 7);
    CHECK(testee[1] == 4);
    CHECK(testee[2] == 5);
    CHECK(testee[3] == 6);
    CHECK(testee[4] == 1);
    CHECK(testee[5] == 2);
    CHECK(&testee[0] == pItem3);
    CHECK(&testee[1] == pItem4);
    CHECK(&testee[2] == pItem5);
    CHECK(&testee[3] == pItem6);
    CHECK(&testee[4] == pItem1);
    CHECK(&testee[5] == pItem2);

    //pop_front
    testee.pop_front();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 5);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem4 == &testee.front());
    CHECK(pItem2 == &testee.back());
    CHECK(testee[0] == 4);
    CHECK(testee[1] == 5);
    CHECK(testee[2] == 6);
    CHECK(testee[3] == 1);
    CHECK(testee[4] == 2);
    CHECK(&testee[0] == pItem4);
    CHECK(&testee[1] == pItem5);
    CHECK(&testee[2] == pItem6);
    CHECK(&testee[3] == pItem1);
    CHECK(&testee[4] == pItem2);

    //8
    testee.push_back(7);
    testee.push_back(8);
    CHECK(!testee.empty());
    CHECK(testee.full());
    CHECK(testee.size() == 6);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem5 == &testee.front());
    CHECK(pItem4 == &testee.back());
    CHECK(testee[0] == 5);
    CHECK(testee[1] == 6);
    CHECK(testee[2] == 1);
    CHECK(testee[3] == 2);
    CHECK(testee[4] == 7);
    CHECK(testee[5] == 8);
    CHECK(&testee[0] == pItem5);
    CHECK(&testee[1] == pItem6);
    CHECK(&testee[2] == pItem1);
    CHECK(&testee[3] == pItem2);
    CHECK(&testee[4] == pItem3);
    CHECK(&testee[5] == pItem4);

    //pop_back
    testee.pop_back();
    CHECK(!testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 5);
    CHECK(testee.get_used_segments() == 3);
    CHECK(pItem5 == &testee.front());
    CHECK(pItem3 == &testee.back());
    CHECK(testee[0] == 5);
    CHECK(testee[1] == 6);
    CHECK(testee[2] == 1);
    CHECK(testee[3] == 2);
    CHECK(testee[4] == 7);
    CHECK(&testee[0] == pItem5);
    CHECK(&testee[1] == pItem6);
    CHECK(&testee[2] == pItem1);
    CHECK(&testee[3] == pItem2);
    CHECK(&testee[4] == pItem3);

    testee.clear();
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 6);
    CHECK(testee.get_segment_size() == 2);
    CHECK(testee.get_segment_count() == 3);
    CHECK(testee.get_used_segments() == 0);
}

TEST_CASE("large_ring_buffer item clear", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<std::string, cpplargeringbuffer::clearable_clear_handler<std::string> > testee(1, 4);

    testee.push_back("1");
    testee.push_back("2");
    testee.push_back("3");
    CHECK(testee.size() == 3);
    std::string* pItem1 = &testee.front();
    std::string* pItem3 = &testee.back();
    CHECK(!pItem1->empty());
    testee.pop_front();
    CHECK(testee.size() == 2);
    CHECK(pItem1->empty());
    CHECK(!pItem3->empty());
    testee.pop_back();
    CHECK(testee.size() == 1);
    CHECK(pItem3->empty());
    testee.push_back("4");
    CHECK(testee.size() == 2);
    testee.push_back("5");
    CHECK(testee.size() == 3);
    testee.push_back("6");
    CHECK(testee.full());
    CHECK(testee.size() == 4);
    CHECK(!testee.front().empty());
    CHECK(testee.extend_back().empty());
    testee.push_back("7");
    CHECK(testee.full());
    CHECK(!testee.back().empty());
    CHECK(testee.extend_front().empty());
}

TEST_CASE("large_ring_buffer default clear", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<std::string, cpplargeringbuffer::assign_default_clear_handler<std::string> > testee(1, 4);

    testee.push_back("1");
    testee.push_back("2");
    testee.push_back("3");
    CHECK(testee.size() == 3);
    std::string* pItem1 = &testee.front();
    std::string* pItem3 = &testee.back();
    CHECK(!pItem1->empty());
    testee.pop_front();
    CHECK(testee.size() == 2);
    CHECK(pItem1->empty());
    CHECK(!pItem3->empty());
    testee.pop_back();
    CHECK(testee.size() == 1);
    CHECK(pItem3->empty());
}

TEST_CASE("large_ring_buffer configure", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = testee.get_max_size() + testee.get_segment_size();
    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_back(i);
    }
    testee.discard_and_change_configuration(4, 5);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 20);
    CHECK(testee.get_segment_size() == 5);
    CHECK(testee.get_segment_count() == 4);
    CHECK(testee.get_used_segments() == 0);

    testee.discard_and_change_configuration(0, 0);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 0);
    CHECK(testee.get_segment_size() == 0);
    CHECK(testee.get_segment_count() == 0);
    CHECK(testee.get_used_segments() == 0);

    testee.discard_and_change_configuration(1, 0);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 0);
    CHECK(testee.get_segment_size() == 0);
    CHECK(testee.get_segment_count() == 0);
    CHECK(testee.get_used_segments() == 0);

    testee.discard_and_change_configuration(0, 1);
    CHECK(testee.empty());
    CHECK(!testee.full());
    CHECK(testee.size() == 0);
    CHECK(testee.get_max_size() == 0);
    CHECK(testee.get_segment_size() == 0);
    CHECK(testee.get_segment_count() == 0);
    CHECK(testee.get_used_segments() == 0);
}

template <typename testee_type>
inline void checkValueRangeReverse(const testee_type& testee, size_t start_value, size_t count)
{
    REQUIRE(count == testee.size());
    for (size_t i = 0; i < count; ++i)
    {
        CHECK(testee[i] == start_value - i);
        CHECK(testee.at(i) == start_value - i);
    }
}

TEST_CASE("large_ring_buffer fill front", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = 2 * testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_back(i);
        testee.pop_back();
        testee.push_front(i);
        testee.extend_front() = i;
        testee.pop_front();
    }
    checkValueRangeReverse(testee, max_value - 1, testee.get_max_size() - 1);
}

TEST_CASE("large_ring_buffer fill front 2", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = 2 * testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_front(i);
    }
    checkValueRangeReverse(testee, max_value - 1, testee.get_max_size());
}

TEST_CASE("large_ring_buffer fill back", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = 2 * testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_front(i);
        testee.pop_front();
        testee.push_back(i);
        testee.extend_back() = i;
        testee.pop_back();
    }
    checkValueRange(testee, testee.get_max_size() + testee.get_segment_size() + 1, testee.get_max_size() - 1);
}

TEST_CASE("large_ring_buffer fill back 2", "[large_ring_buffer]")
{
    cpplargeringbuffer::large_ring_buffer<size_t> testee(5, 3);
    const size_t max_value = 2 * testee.get_max_size() + testee.get_segment_size();

    for (size_t i = 0; i < max_value; ++i)
    {
        testee.push_back(i);
    }
    checkValueRange(testee, testee.get_max_size() + testee.get_segment_size(), testee.get_max_size());
}
