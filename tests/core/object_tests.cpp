#include <doctest.h>
#include <kali/core/object.h>

using namespace kali;

class TestObject : public Object {
public:
    TestObject() = default;
};

TEST_SUITE_BEGIN("object");

TEST_CASE("ref")
{
    ref<TestObject> r;
    CHECK(r.operator bool() == false);
    CHECK(r.get() == nullptr);
    CHECK(r == nullptr);

    r = new TestObject();
    CHECK(r.operator bool() == true);
    CHECK(r.get() != nullptr);
    CHECK(r != nullptr);
    CHECK(r->ref_count() == 1);
}

TEST_SUITE_END();
