#include <memory>

#include <jni.h>
#include <RenderScript.h>

#include "ScriptC_simple.h"

using namespace android;
using namespace RSC;

extern "C" void JNICALL
Java_com_android_rs_renderscriptjni_MainActivity_nativeRS(
    JNIEnv * env,
    jclass,
    jstring pathObj)
{
    int result;
    static const int size = 64;

    const char * path = env->GetStringUTFChars(pathObj, nullptr);
    sp<RS> rs = new RS();
    rs->init(path, RS_INIT_LOW_LATENCY |
                   RS_INIT_OPT_LEVEL_0 |
                   RS_INIT_WAIT_FOR_ATTACH);
    env->ReleaseStringUTFChars(pathObj, path);

    auto e = Element::RGBA_8888(rs);
    Type::Builder tb(rs, e);
    tb.setX(size);
    tb.setY(size);
    auto t = tb.create();

    auto a = Allocation::createTyped(rs, t);
    auto b = Allocation::createTyped(rs, t);

    sp<ScriptC_simple> s = new ScriptC_simple(rs);
    s->forEach_simple_kernel(a, b);
    rs->finish();
    std::unique_ptr<uint32_t[]> output(new uint32_t[size * size]);
    b->copy2DRangeTo(0, 0, size, size, output.get());
}

