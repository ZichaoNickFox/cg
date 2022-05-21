#include "gtest/gtest.h"

#include "engine/any.h"

namespace engine {
class BaseClass {};
class DeriveClass : public BaseClass {
 public:
  DeriveClass() {}
  explicit DeriveClass(int in) : a(in) {}
  DeriveClass(const DeriveClass& o) : a(o.a) {}
  int a = 1;
};

inline bool operator==(const DeriveClass& l, const DeriveClass& r) {
  return l.a == r.a;
}

TEST(AnyTest, TypeIs) {
  Any a{1};
  EXPECT_EQ(a.TypeIs<int>(), true);

  DeriveClass derive_class;
  Any b{derive_class};
  EXPECT_EQ(b.TypeIs<DeriveClass>(), true);

  Any c{DeriveClass()};
  EXPECT_EQ(c.TypeIs<DeriveClass>(), true);

  Any d{new DeriveClass};
  EXPECT_EQ(d.TypeIs<DeriveClass*>(), true);

  // Attention : Any don't polymorphic object
  BaseClass* polymorphic = new DeriveClass;
  Any e{polymorphic};
  EXPECT_EQ(e.TypeIs<BaseClass*>(), true);

  Any f{std::make_shared<int>(1)};
  EXPECT_EQ(f.TypeIs<std::shared_ptr<int>>(), true);

  Any g{std::make_shared<DeriveClass>()};
  EXPECT_EQ(g.TypeIs<std::shared_ptr<DeriveClass>>(), true);

  int* p = new int;
  Any h{&p};
  EXPECT_EQ(h.TypeIs<int**>(), true);
}

TEST(AnyTest, Value) {
  Any a{1};
  EXPECT_EQ(a.Value<int>(), 1);

  DeriveClass derive_class(2);
  Any b(derive_class);
  EXPECT_EQ(b.Value<DeriveClass>().a, 2) << b.Value<DeriveClass>().a << "aaa";

  Any c{DeriveClass()};
  EXPECT_EQ(c.Value<DeriveClass>().a, 1);

  Any d(new DeriveClass);
  EXPECT_EQ(d.Value<DeriveClass*>()->a, 1);

  // Attention : polymorphic can work
  BaseClass* polymorphic = new DeriveClass;
  Any e(polymorphic);
  EXPECT_EQ(e.Value<DeriveClass*>()->a, 1);

  Any f(std::make_shared<int>(123));
  EXPECT_EQ(*f.Value<std::shared_ptr<int>>(), 123);

  Any g(std::make_shared<DeriveClass>());
  EXPECT_EQ(g.Value<std::shared_ptr<DeriveClass>>()->a, 1);

  int* p = new int(1024);
  Any h(&p);
  EXPECT_EQ(**h.Value<int**>(), 1024);
}

TEST(AnyTest, MutableValue) {
  Any a(1);
  a.MutableValue<int>() = 2;
  EXPECT_EQ(a.Value<int>(), 2);

  DeriveClass derive_class(2);
  Any b(derive_class);
  DeriveClass derive_class2(4);
  b.MutableValue<DeriveClass>() = derive_class2;
  EXPECT_EQ(b.Value<DeriveClass>().a, 4);

  Any c{DeriveClass()};
  c.MutableValue<DeriveClass>() = derive_class2;
  EXPECT_EQ(c.Value<DeriveClass>().a, 4);

  Any d(new DeriveClass);
  d.MutableValue<DeriveClass*>() = new DeriveClass(2);
  EXPECT_EQ(d.Value<DeriveClass*>()->a, 2);

  // Attention : polymorphic can work
  BaseClass* polymorphic = new DeriveClass;
  BaseClass* polymorphic2 = new DeriveClass(2);
  Any e(polymorphic);
  e.MutableValue<BaseClass*>() = polymorphic2;
  EXPECT_EQ(e.Value<DeriveClass*>()->a, 2);

  Any f(std::make_shared<int>(123));
  auto fp = std::make_shared<int>(234);
  f.MutableValue<std::shared_ptr<int>>() = fp;
  EXPECT_EQ(*f.Value<std::shared_ptr<int>>(), 234);

  Any g(std::make_shared<DeriveClass>());
  auto gp = std::make_shared<DeriveClass>(2);
  g.MutableValue<std::shared_ptr<DeriveClass>>() = gp;
  EXPECT_EQ(g.Value<std::shared_ptr<DeriveClass>>()->a, 2);

  int* p = new int(1024);
  int* p2 = new int(1025);
  Any h(&p);
  h.MutableValue<int**>() = &p2;
  EXPECT_EQ(**h.Value<int**>(), 1025);
}
} // namespace engine
