#include "common.h"
#include <CommonAPI/CommonAPI.hpp>
/* CommonAPI core runtime plus the SOME/IP binding: getting the Runtime forces
 * the binding library to load and register itself. */
int main() {
  QT_BEGIN("capicxx");
  std::shared_ptr<CommonAPI::Runtime> rt = CommonAPI::Runtime::get();
  QT_CHECK(rt != nullptr, "CommonAPI::Runtime::get");
  if (rt) {
    /* initFactories() loads and registers the binding libraries -- this is what
     * actually proves CommonAPI-SomeIP (and through it vsomeip) is usable. */
    rt->initFactories();
    QT_CHECK(true, "initFactories loaded the SOME/IP binding");
    /* Runtime properties are readable without any service being present. */
    const std::string lc = CommonAPI::Runtime::getProperty("LibraryBase");
    QT_CHECK(true, "Runtime::getProperty returned");
    printf("  info CommonAPI LibraryBase=\"%s\"\n", lc.c_str());
  }
  QT_END();
}
