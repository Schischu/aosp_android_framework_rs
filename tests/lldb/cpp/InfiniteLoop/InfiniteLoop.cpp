#include <thread>
#include <chrono>

#include <RenderScript.h>

#include "ScriptC_simple.h"

using namespace android;
using namespace RSC;

int main()
{
  int result;
  static const int size = 64;
  sp<RS> rs = new RS();
  // Init with /system/bin because this is a standalone exe
  rs->init("/system/bin", RS_INIT_LOW_LATENCY |
                          RS_INIT_OPT_LEVEL_0);

  auto e = Element::RGBA_8888(rs);
  Type::Builder tb(rs, e);
  tb.setX(size);
  tb.setY(size);
  auto t = tb.create();

  auto a = Allocation::createTyped(rs, t);
  auto b = Allocation::createTyped(rs, t);

  auto s = new ScriptC_simple(rs);

  // Test is designed to loop forever, waits for two seconds
  // between each invocation of the kernel
  bool forever = true;
  while(forever)
  {
    s->forEach_simple_kernel(a, b);
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  uint32_t * output = new uint32_t[size*size];
  b->copy2DRangeTo(0, 0, size, size, output);

  return 0;
}

