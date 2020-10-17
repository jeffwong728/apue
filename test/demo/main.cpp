#include <SkData.h>
#include <SkImage.h>
#include <SkStream.h>
#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPath.h>
#include <SkMaskFilter.h>
#include <SkBlurMaskFilter.h>
#include <SkBlurDrawLooper.h>
#include <SkLightingImageFilter.h>
#include <SkPoint3.h>

void draw_0(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SkColorSetARGB(255, 0xF9, 0xA6, 0x02));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5));
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}

void draw_1(SkCanvas* canvas) {
    SkPaint paint;
    paint.setDrawLooper(SkBlurDrawLooper::Make(0xFFFF0000, 2, 0, 0));
    paint.setDrawLooper(SkBlurDrawLooper::Make(0xFFFF0000, 2, 0, 0));
    paint.setDrawLooper(SkBlurDrawLooper::Make(0xFFFF0000, 2, 0, 0));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);
    paint.setAntiAlias(true);
    paint.setColor(0xFFFF0000);
    //canvas->clear(SK_ColorBLACK);
    canvas->drawCircle(70, 70, 50, paint);
}

void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(96);
    offscreen.clear(0);
    offscreen.drawString("e", 20, 70, paint);
    paint.setImageFilter(
        SkLightingImageFilter::MakePointLitDiffuse(SkPoint3::Make(80, 100, 10),
            SK_ColorWHITE, 1, 2, nullptr, nullptr));
    canvas->drawBitmap(bitmap, 0, 0, &paint);
}

int main()
{
    int width  = 512;
    int height = 512;
    const char* path = "out_raster.png";
    
        sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
    SkCanvas* rasterCanvas = rasterSurface->getCanvas();
    draw_0(rasterCanvas);
    sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
    if (!img) { return -1; }
    sk_sp<SkData> png(img->encodeToData());
    if (!png) { return -1; }
    SkFILEWStream out(path);
    (void)out.write(png->data(), png->size());
    
    return 0;
}
