#include <iostream>

#include <RenderScript.h>

#include "ScriptC_simple.h"

using namespace android;
using namespace RSC;

int main()
{
  // we want to allocate a 2d allocation on the device, 256*256
  int result;
  int size = 64;
  sp<RS> rs = new RS();
  rs->init("/system/bin", RS_INIT_WAIT_FOR_ATTACH);

  auto e = Element::RGBA_8888(rs);
  Type::Builder tb(rs, e);
  tb.setX(size);
  tb.setY(size);
  auto t = tb.create();

  auto a = Allocation::createTyped(rs, t);
  auto b = Allocation::createTyped(rs, t);

  auto s = new ScriptC_simple(rs);
  s->forEach_simple_kernel(a, b);
  rs->finish();
  uint32_t * output = new uint32_t[size*size];
  b->copy2DRangeTo(0, 0, size, size, output);

  return 0;
}

