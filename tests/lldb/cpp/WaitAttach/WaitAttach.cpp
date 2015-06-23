#include <iostream>

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
                          RS_INIT_OPT_LEVEL_0 |
                          RS_INIT_WAIT_FOR_ATTACH);

  auto e = Element::RGBA_8888(rs);
  Type::Builder tb(rs, e);
  tb.setX(size);
  tb.setY(size);
  auto t = tb.create();

  auto a = Allocation::createTyped(rs, t);
  auto b = Allocation::createTyped(rs, t);

  // Script is executed once, then the data is copied back when finished
  auto s = new ScriptC_simple(rs);
  s->forEach_simple_kernel(a, b);
  rs->finish();
  uint32_t * output = new uint32_t[size*size];
  b->copy2DRangeTo(0, 0, size, size, output);

  return 0;
}

