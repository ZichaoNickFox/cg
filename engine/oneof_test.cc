#include <memory>

#include "glog/logging.h"
#include <google/protobuf/util/message_differencer.h>
#include "gtest/gtest.h"

#include "engine/oneof.h"

namespace base {
namespace container {
namespace {

struct A{ int aa = 123; };
inline bool operator== (const A& l, const A& r) {
  return l.aa == r.aa;
}

class B{ public: int bb = 234; };
inline bool operator== (const B& l, const B& r) {
  return l.bb == r.bb;
}

TEST(OneofTest, StaticAssert) {
  Oneof<int, bool, float> a(1);
  // Oneof<bool, float> b = 1;    // assertion
  Oneof<A, int> c{A()};
  Oneof<B> d{B()};
  Oneof<std::shared_ptr<A>> e(std::make_shared<A>());
}

TEST(OneofTest, TypeIs) {
  Oneof<int, bool, float> a(1);
  EXPECT_TRUE(a.TypeIs<int>());

  // Oneof<bool, float> b = 1;    // assertion

  Oneof<A, int> c{A()};
  EXPECT_TRUE(c.TypeIs<A>());

  Oneof<B> d{B()};
  EXPECT_TRUE(d.TypeIs<B>());

  Oneof<std::shared_ptr<A>> e{std::make_shared<A>()};
  EXPECT_TRUE(e.TypeIs<std::shared_ptr<A>>());
}

TEST(OneofTest, Value) {
  Oneof<int, bool, float> a(1);
  EXPECT_EQ(a.Value<int>(), 1);

  // Oneof<bool, float> b = 1;    // assertion

  Oneof<A, int> c{A()};
  EXPECT_EQ(c.Value<A>(), A());

  Oneof<B> d{B()};
  EXPECT_EQ(d.Value<B>(), B());

  auto ptr = std::make_shared<A>();
  Oneof<std::shared_ptr<A>> e{ptr};
  EXPECT_EQ(e.Value<std::shared_ptr<A>>(), ptr);
}

} // namespace
} // namespace container
} // namespace base
