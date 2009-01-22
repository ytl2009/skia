#ifndef SkBenchmark_DEFINED
#define SkBenchmark_DEFINED

#include "SkRefCnt.h"
#include "SkPoint.h"

class SkCanvas;
class SkPaint;

class SkBenchmark : public SkRefCnt {
public:
    SkBenchmark();

    const char* getName();
    SkIPoint getSize();
    void draw(SkCanvas*);
    
    void setForceAlpha(int alpha) {
        fForceAlpha = alpha;
    }
    
    void setForceAA(bool aa) {
        fForceAA = aa;
    }

protected:
    void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual SkIPoint onGetSize() = 0;
    virtual void onDraw(SkCanvas*) = 0;
    
private:
    int     fForceAlpha;
    bool    fForceAA;
};

static inline SkIPoint SkMakeIPoint(int x, int y) {
    SkIPoint p;
    p.set(x, y);
    return p;
}

#endif

