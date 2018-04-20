#include <gtest/gtest.h>
#include "../src/api/object.h"

class Dummy : public api::ExposedObjectT<Dummy, api::ObjectType::kDummy> {};

class MyObject
    : public api::ExposedObjectT<MyObject, api::ObjectType::kContext> {
 public:
  MyObject(int) {}
  ~MyObject() { ++dtor_calls_; }

  static int GetDtorCalls() { return dtor_calls_; }

 private:
  static int dtor_calls_;
};

int MyObject::dtor_calls_ = 0;

TEST(ObjectTest, Create) {
  auto obj = MyObject::Create(123);
  EXPECT_TRUE(obj.unique());
}

TEST(ObjectTest, Type) {
  auto obj = MyObject::Create(123);
  EXPECT_EQ(obj->Type(), api::ObjectType::kContext);
  EXPECT_EQ(MyObject::kType, api::ObjectType::kContext);
}

TEST(ObjectTest, Handle) {
  auto obj = MyObject::Create(123);
  EXPECT_NE(obj->Handle(), nullptr);
}

TEST(ObjectTest, Cast) {
  auto my_obj = MyObject::Create(123);
  auto obj = std::dynamic_pointer_cast<api::ExposedObject>(my_obj);
  auto my_obj_2 = obj->Cast<MyObject>();
  EXPECT_EQ(my_obj, my_obj_2);

  EXPECT_THROW(my_obj->Cast<Dummy>(), api::Error);
  EXPECT_THROW(obj->Cast<Dummy>(), api::Error);
}

TEST(ObjectTest, RegisterAndDestroyObject) {
  auto dtor_calls = MyObject::GetDtorCalls();

  auto obj = MyObject::Create(123);
  auto handle = api::ObjectRegister::Register(obj);
  EXPECT_FALSE(obj.unique());
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);
  EXPECT_EQ(handle, obj->Handle());

  EXPECT_THROW(api::ObjectRegister::Destroy(nullptr), api::Error);
  EXPECT_THROW(api::ObjectRegister::Destroy(handle), api::Error);

  obj.reset();
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);
  EXPECT_NO_THROW(api::ObjectRegister::Destroy(handle));
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls + 1);

  EXPECT_THROW(api::ObjectRegister::Destroy(handle), api::Error);
}

TEST(ObjectTest, GetRegisteredObject) {
  EXPECT_THROW(api::ObjectRegister::Get(nullptr), api::Error);

  auto obj = MyObject::Create(123);
  auto handle = api::ObjectRegister::Register(obj);

  api::ObjectPtr obj2 = api::ObjectRegister::Get(handle);
  EXPECT_EQ(obj2, obj);

  std::shared_ptr<MyObject> obj3 = api::ObjectRegister::Get<MyObject>(handle);
  EXPECT_EQ(obj3, obj);

  EXPECT_THROW(api::ObjectRegister::Get<Dummy>(handle), api::Error);
}

TEST(ObjectTest, DestroyAllObjects) {
  EXPECT_NO_THROW(api::ObjectRegister::DestroyAll());

  auto obj1 = MyObject::Create(123);
  auto obj2 = MyObject::Create(456);

  auto dtor_calls = MyObject::GetDtorCalls();
  api::ObjectRegister::Register(obj2);
  api::ObjectRegister::Register(obj1);
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);

  obj1.reset();
  obj2.reset();
  api::ObjectRegister::DestroyAll();
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls + 2);
}
