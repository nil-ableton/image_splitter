#include <wincodec.h>
#include <synchapi.h>
#include "stb_image_write.h"

static IWICImagingFactory *wic_factory;
static SRWLOCK wic_factory_lock = SRWLOCK_INIT;

void split_images(char const *filename, char const* output_basename)
{
    if (!wic_factory) {
        AcquireSRWLockExclusive(&wic_factory_lock   );
        if (!wic_factory) {
            CoInitializeEx(0, COINITBASE_MULTITHREADED);
            if (CoCreateInstance(&CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, &wic_factory) < 0) {
                ReleaseSRWLockExclusive(&wic_factory_lock);
                goto done;
            }
        }
        ReleaseSRWLockExclusive(&wic_factory_lock);
    }
    int wide_filename_length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, 0, 0);
    WCHAR *wide_filename = (WCHAR *)_alloca(wide_filename_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wide_filename, wide_filename_length);
    IWICBitmapDecoder *image_decoder = 0;
    if (wic_factory->lpVtbl->CreateDecoderFromFilename(wic_factory, wide_filename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &image_decoder) < 0) {
        goto done;
    }

    int frame_index = 0;
    int num_frames;
    {
        UINT y;
        if (image_decoder->lpVtbl->GetFrameCount(image_decoder, &y) < 0) {
            goto done;
        }
        guard(y < INT_MAX);
        num_frames = y;
    }
    for (frame_index = 0; frame_index<num_frames; frame_index++) {
        IWICBitmapFrameDecode *image_frame = 0;
        IWICBitmapSource *rgba_image_frame = 0;
        if (image_decoder->lpVtbl->GetFrame(image_decoder, frame_index, &image_frame) < 0) {
            goto done_with_frame;
        }
        if (WICConvertBitmapSource(&GUID_WICPixelFormat32bppRGBA, image_frame, &rgba_image_frame) < 0) {
            goto done_with_frame;
        }
        uint32_t width;
        uint32_t height;
        image_frame->lpVtbl->GetSize(image_frame, &width, &height);
        uint32_t channels = 4;
        uint32_t buffer_size = channels * width * height;
        uint8_t *pixels = calloc(buffer_size, 1);
        uint32_t buffer_stride = channels * width;
        if (rgba_image_frame->lpVtbl->CopyPixels(rgba_image_frame, 0, buffer_stride, buffer_size, pixels) < 0) {
            free(pixels);
            goto done_with_frame;
        }

        char *output_filename = aprintf("%s-%d.png", output_basename, frame_index);
        stbi_write_png(output_filename, width, height, channels, pixels, buffer_stride);
        free(pixels);
done_with_frame:
        if (image_frame) {
            image_frame->lpVtbl->Release(image_frame);
        }
        if (rgba_image_frame) {
            rgba_image_frame->lpVtbl->Release(rgba_image_frame);
        }
    }
done:
    if (image_decoder) {
        image_decoder->lpVtbl->Release(image_decoder);
    }
}

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Ole32.lib")
