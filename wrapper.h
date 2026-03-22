
typedef void* HGLOBAL;
struct BITMAPINFOHEADER{
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
};
struct jas_image_t { void* a; };
struct jas_stream_t{ void* a; };

#ifdef __cplusplus
extern "C" {
#endif

    jas_image_t *BmpDecode(void *buf, int bufsize);
    jas_image_t *BmpDecodeDR(BITMAPINFOHEADER *header, unsigned char *data, double xres, double yres);
    int BmpEncode(jas_image_t *image, void *buf, int bufsize);
    int BmpEncodeHR(jas_image_t *image, HGLOBAL *header, HGLOBAL *data, double *xres, double *yres);
    jas_image_t *jp2_decode(jas_stream_t *in, char *optstr);
    jas_image_t *jpc_decode(jas_stream_t *in, char *optstr);
    int jpc_encode(jas_image_t *image, jas_stream_t *out, char *optstr);
    int jp2_encode(jas_image_t *image, jas_stream_t *out, char *optstr);
    void jas_image_destroy(jas_image_t *image);
    jas_stream_t *jas_stream_memopen(void *buf, int bufsize);
    //jas_stream_t *jas_stream_fopen(const char *filename, const char *mode);
    //jas_stream_t *jas_stream_fwopen(const wchar_t *filename, const wchar_t *mode);
    int jas_stream_close(jas_stream_t *stream);

#ifdef __cplusplus
}
#endif

