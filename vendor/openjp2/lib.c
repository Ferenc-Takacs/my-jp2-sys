
#include "opj_includes.h" 
#include <string.h>
#include <stdlib.h>

//#define CALL_FROM_CPP

typedef struct {
    unsigned char* data;
    size_t size;
    size_t allocated;
    size_t pos;
    unsigned char err;
} opj_mem_stream;

static OPJ_SIZE_T opj_read_from_mem(void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data) {
    opj_mem_stream* l_stream = (opj_mem_stream*)p_user_data;
    if (l_stream->pos >= l_stream->size) return (OPJ_SIZE_T)-1;
    size_t l_nb_read = (l_stream->pos + p_nb_bytes > l_stream->size) ? l_stream->size - l_stream->pos : p_nb_bytes;
    memcpy(p_buffer, l_stream->data + l_stream->pos, l_nb_read);
    l_stream->pos += l_nb_read;
    return l_nb_read;
}

static OPJ_SIZE_T opj_write_from_mem(void* p_buffer, OPJ_SIZE_T p_size, void* p_user_data) {
	opj_mem_stream* p_stream = (opj_mem_stream*)p_user_data;
	if (p_stream->err) return (OPJ_SIZE_T)-1;
	size_t required_end = p_stream->pos + p_size;
	if (required_end > p_stream->allocated) {
		size_t new_alloc = required_end + 0x10000; // 64KB ráhagyás
		void* new_data = opj_realloc(p_stream->data, new_alloc);
		if (!new_data) { p_stream->err = 1; return (OPJ_SIZE_T)-1; }
		p_stream->data = (unsigned char*)new_data;
		p_stream->allocated = new_alloc;
	}
	memcpy(p_stream->data + p_stream->pos, p_buffer, p_size);
	p_stream->pos += p_size;
	if (p_stream->pos > p_stream->size) {
		p_stream->size = p_stream->pos;
	}
	return p_size;
}

static OPJ_OFF_T opj_skip_from_memread(OPJ_OFF_T p_nb_bytes, void* p_user_data) {
	opj_mem_stream* l_stream = (opj_mem_stream*)p_user_data;
	size_t l_nb_read = (l_stream->pos + p_nb_bytes > l_stream->size) ? l_stream->size - l_stream->pos : p_nb_bytes;
	l_stream->pos += l_nb_read;
	return p_nb_bytes;
}

static OPJ_OFF_T opj_skip_from_memwrite(OPJ_OFF_T p_nb_bytes, void* p_user_data) {
	opj_mem_stream* p_stream = (opj_mem_stream*)p_user_data;
	if (p_stream->err) return (OPJ_OFF_T)-1;
	OPJ_OFF_T new_pos = (OPJ_OFF_T)p_stream->pos + p_nb_bytes;
	if ((size_t)new_pos > p_stream->allocated) {
		size_t new_alloc = (size_t)new_pos + 0x20000; // 128KB ráhagyás
		void* new_data = opj_realloc(p_stream->data, new_alloc);
		if (!new_data) {
			p_stream->err = 1;
			return (OPJ_OFF_T)-1;
		}
		memset((unsigned char*)new_data + p_stream->size, 0, new_alloc - p_stream->size);

		p_stream->data = (unsigned char*)new_data;
		p_stream->allocated = new_alloc;
	}
	p_stream->pos = (size_t)new_pos;
	if (p_stream->pos > p_stream->size) {
		p_stream->size = p_stream->pos;
	}
	return p_nb_bytes;
}

static OPJ_BOOL opj_seek_from_mem(OPJ_OFF_T p_nb_bytes, void* p_user_data) {
    opj_mem_stream* l_stream = (opj_mem_stream*)p_user_data;
    l_stream->pos = (size_t)p_nb_bytes;
    return OPJ_TRUE;
}

typedef struct {
    char* buffer;
    size_t size;
} error_context_t;

void error_callback(const char *msg, void *client_data) {
	if (!msg) return;
#ifdef CALL_FROM_CPP
	printf("%s", msg);
#endif
	if (!client_data) return;
    error_context_t* ctx = (error_context_t*)client_data;
    if (ctx->buffer && ctx->size) {
        size_t blen = strlen(ctx->buffer);
        strncat(ctx->buffer, msg, ctx->size - blen - 1);
        ctx->buffer[ctx->size - 1] = '\0'; // Lezáró nulla
    }
}

#ifdef CALL_FROM_CPP
void info_callback(const char *msg, void *client_data) {
	if (!msg) return;
	printf("info: %s", msg);
}
#endif

typedef struct  {
    error_context_t err_ctx;
    opj_mem_stream mstream;
    opj_stream_t* stream;
    opj_codec_t* codec;
    opj_image_t* image;
} my_decode_t;

extern void my_decode_destroy(my_decode_t*dec)
{
    if( dec) {
        if(dec->codec)
            opj_destroy_codec(dec->codec);
        if(dec->stream)
            opj_stream_destroy(dec->stream);
        if(dec->image)
            opj_image_destroy(dec->image);
        opj_free(dec);
    }
}

extern int get_jp2_info(unsigned char* input_ptr, int input_size, int *w, int *h,
	float *xr, float *yr, unsigned short *dpi, char* err_msg, int err_msg_size, my_decode_t**decod)
{
	if (err_msg && err_msg_size > 0) err_msg[0] = '\0';
	if (input_size <= 4) {
		error_callback("no data", err_msg);
		return -1;
	}
	OPJ_CODEC_FORMAT format = (((int*)input_ptr)[0] == 0x0c000000) ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K;
	
	my_decode_t *dec = opj_malloc(sizeof(my_decode_t));
	memset(dec, 0, sizeof(my_decode_t));
	dec->err_ctx.buffer = err_msg;
	dec->err_ctx.size = (size_t)err_msg_size;
	dec->mstream.data = input_ptr;
	dec->mstream.size = (size_t)input_size;
	dec->mstream.allocated = (size_t)input_size;
	dec->mstream.pos = 0;
	dec->mstream.err = 0;

	dec->stream = opj_stream_default_create(OPJ_TRUE);
	opj_stream_set_read_function(dec->stream, opj_read_from_mem);
	opj_stream_set_skip_function(dec->stream, opj_skip_from_memread);
	opj_stream_set_seek_function(dec->stream, opj_seek_from_mem);
	opj_stream_set_user_data(dec->stream, &(dec->mstream), NULL);
	opj_stream_set_user_data_length(dec->stream, input_size);

	opj_dparameters_t params;

	opj_set_default_decoder_parameters(&params);
	dec->codec = opj_create_decompress(format);
	opj_set_error_handler(dec->codec, error_callback, &(dec->err_ctx));
	opj_set_warning_handler(dec->codec, error_callback, &(dec->err_ctx));
#ifdef CALL_FROM_CPP
	opj_set_info_handler(dec->codec, info_callback, &(dec->err_ctx));
#endif
	opj_setup_decoder(dec->codec, &params);

	if (!opj_read_header(dec->stream, dec->codec, &(dec->image))) {
		my_decode_destroy(dec);
		return -1;
	}

	*w = (int)(dec->image->x1 - dec->image->x0);
	*h = (int)(dec->image->y1 - dec->image->y0);
	*xr = dec->image->hres;
	*yr = dec->image->vres;
	*dpi = dec->image->dpi;
	*decod = dec;
	return 0;
}

extern int fill_jp2_rgba(unsigned char* output_pixels, my_decode_t*dec) {
    if( !opj_decode(dec->codec, dec->stream, dec->image) ) {
        my_decode_destroy(dec);
        return -1;
    }
    int w = dec->image->x1 - dec->image->x0;
    int h = dec->image->y1 - dec->image->y0;
    int comps = dec->image->numcomps;
    for (int i = 0; i < w * h; i++) {
        output_pixels[i * 4 + 0] = (unsigned char)dec->image->comps[0].data[i];
        output_pixels[i * 4 + 1] = (unsigned char)dec->image->comps[1].data[i];
        output_pixels[i * 4 + 2] = (unsigned char)dec->image->comps[2].data[i];
        output_pixels[i * 4 + 3] = (comps > 3) ? (unsigned char)dec->image->comps[3].data[i] : 255;
    }
    my_decode_destroy(dec);
    return 0;
}

extern int encode_rgba_to_jp2_mem(unsigned char* rgba_input, int w, int h, int chan, const unsigned char quality,
            float xres, float yres, unsigned short dpi,
            unsigned char* exif, int exif_len,
            unsigned char Jp2, unsigned char* output_buf, int max_out_size, int* actual_out_size, char* err_msg, int err_msg_size)
{
    error_context_t err_ctx = { err_msg, (size_t)err_msg_size };
    if (err_msg && err_msg_size > 0) err_msg[0] = '\0';
    
    opj_image_cmptparm_t cmptparm[4];
    memset(&cmptparm, 0, sizeof(cmptparm));
    for (int i = 0; i < chan; i++) {
        cmptparm[i].prec = 8;
        cmptparm[i].bpp = 8;
        cmptparm[i].dx = 1;
        cmptparm[i].dy = 1;
        cmptparm[i].w = w;
        cmptparm[i].h = h;
    }

    opj_image_t* image = opj_image_create(chan, &cmptparm[0], OPJ_CLRSPC_SRGB);
    for (int i = 0; i < w * h; i++) {
        image->comps[0].data[i] = rgba_input[i * chan + 0];
        image->comps[1].data[i] = rgba_input[i * chan + 1];
        image->comps[2].data[i] = rgba_input[i * chan + 2];
        if ( chan==4 ) image->comps[3].data[i] = rgba_input[i * chan + 3];
    }
    if ( chan==4 ) image->comps[3].alpha = 1;

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = w;
    image->y1 = h;
    image->hres = xres;
    image->vres = yres;
    image->dpi = dpi;
    
    opj_mem_stream mstream = {NULL, (size_t)0, (size_t)0, 0, 0};
    opj_stream_t* stream = opj_stream_default_create(OPJ_FALSE);
    opj_stream_set_write_function(stream, opj_write_from_mem);
    opj_stream_set_skip_function(stream, opj_skip_from_memwrite);
    opj_stream_set_seek_function(stream, opj_seek_from_mem);
    opj_stream_set_user_data(stream, &mstream, NULL);
    opj_stream_set_user_data_length(stream, 0);
    
    opj_codec_t* codec = opj_create_compress(Jp2 ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K);
    opj_set_error_handler( codec, error_callback, &err_ctx);
    opj_set_warning_handler( codec, error_callback, &err_ctx);
#ifdef CALL_FROM_CPP
	opj_set_info_handler( codec, info_callback, &err_ctx);
#endif
    opj_cparameters_t params;
    opj_set_default_encoder_parameters(&params);
	float psnr;
	if (quality >= 100) {
		psnr = 0; // Lossless
	}
	else if (quality > 90) {
		psnr = 50.0f + (quality - 90) * 1.0f; // 50-60 dB: Nagyon finom
	}
	else if (quality > 50) {
		psnr = 35.0f + (quality - 50) * 0.375f; // 35-50 dB: Jó tartomány
	}
	else {
		psnr = 25.0f + (quality * 0.2f); // 25-35 dB: Itt romlik gyorsan
	}
	params.tcp_distoratio[0] = psnr;
	//params.tcp_distoratio[0] = quality;
	params.cp_fixed_quality = 1;
    params.tcp_numlayers = 1;
    params.cp_comment = "Jpeg 2000 from OpenJpeg for iView";
	if (Jp2) {
		opj_jp2_t *jp2 = (opj_jp2_t*)(((opj_codec_private_t*)codec)->m_codec);
		jp2->exif = exif;
		jp2->exif_len = exif_len;
	}

    if (! opj_setup_encoder(codec, &params, image)) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        return 1;
    }
    if(! opj_start_compress(codec, image, stream) ) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        return 1;
    }
    if(! opj_encode(codec, stream) ) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        return 1;
    }
    if(! opj_end_compress(codec, stream) ) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        return 1;
    }
    *actual_out_size = (int)mstream.size;
    if(mstream.size > max_out_size) {
        opj_event_msg(&(((opj_codec_private_t*)codec)->m_event_mgr),
            EVT_ERROR, "file (%d) is bigger than buffer (%d)\n",mstream.size,max_out_size);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        return 1;
    }
    memcpy( output_buf, mstream.data, mstream.size);
    opj_destroy_codec(codec);
    opj_image_destroy(image);
    opj_stream_destroy(stream);
    return 0; // Implementálandó a memória-író callback-el
}


//////////////////////////////////////////////////////////////////////////////////////
/*
extern "C" int fill_jp2_rgba(unsigned char* input_ptr, int input_size, 
                                   unsigned char* output_pixels) {
    jas_stream_t *in = jas_stream_memopen((char*)input_ptr, input_size);
    if (!in) return -1;

    jas_image_t *image = jp2_decode(in, NULL);
    if (!image) {
        jas_stream_close(in);
        return -2;
    }
    int width = jas_image_width(image);
    int height = jas_image_height(image);

    // RGBA másolás - Nincs padding gond!
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = jas_image_readcmptsample(image, 0, x, y);
            int g = jas_image_readcmptsample(image, 1, x, y);
            int b = jas_image_readcmptsample(image, 2, x, y);
            int a = (jas_image_numcmpts(image) > 3) ? jas_image_readcmptsample(image, 3, x, y) : 255;
            
            int idx = (y * width + x) * 4;
            output_pixels[idx]     = (unsigned char)r;
            output_pixels[idx + 1] = (unsigned char)g;
            output_pixels[idx + 2] = (unsigned char)b;
            output_pixels[idx + 3] = (unsigned char)a;
        }
    }

    jas_image_destroy(image);
    jas_stream_close(in);
    return 0;
}

extern "C" int get_jp2_info(unsigned char* input_ptr, int input_size, 
                           int* width, int* height, 
                           float* xres, float* yres) {
    jas_stream_t *in = jas_stream_memopen((char*)input_ptr, input_size);
    if (!in) return -1;
    jas_image_t *image = jp2_decode(in, NULL);
    if (!image) {
        jas_stream_close(in);
        return -2;
    }    
    *width = jas_image_width(image);
    *height = jas_image_height(image);
    *xres = float(image->hres * 0.0254);
    *yres = float(image->vres * 0.0254);
    jas_image_destroy(image);
    jas_stream_close(in);
    return 0;
}

extern "C" int encode_rgb_to_jp2_mem(unsigned char* rgb_input, int w, int h, const char* optstr, unsigned char*exif, int exif_len,
                                      unsigned char* output_buf, int max_out_size, int* actual_out_size) {

    jas_image_cmptparm_t cmptparms[3];
    for (int i = 0; i < 3; i++) {
        jas_image_cmptparm_t cmptparm;
        cmptparms[i].tlx = 0;
        cmptparms[i].tly = 0;
        cmptparms[i].hstep = 1;
        cmptparms[i].vstep = 1;
        cmptparms[i].width = w;
        cmptparms[i].height = h;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = false;
    }
    jas_image_t *image = jas_image_create(3, cmptparms, JAS_CLRSPC_UNKNOWN);
    if (!image) return -1;
    jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
    jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
    jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
    jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w + x) * 3;
            jas_image_writecmptsample(image, 0, x, y, rgb_input[idx]);     // R
            jas_image_writecmptsample(image, 1, x, y, rgb_input[idx + 1]); // G
            jas_image_writecmptsample(image, 2, x, y, rgb_input[idx + 2]); // B
        }
    }

    jas_stream_t *out = jas_stream_memopen(NULL, 0); 
    if (!out) { jas_image_destroy(image); return -2; }

    if (jp2_encode(image, out, (char*)optstr) != 0) {
        jas_image_destroy(image);
        jas_stream_close(out);
        return -3;
    }
    long len = jas_stream_tell(out);
    if (len > max_out_size) {
        *actual_out_size = (int)len;
        jas_image_destroy(image);
        jas_stream_close(out);
        return -4;
    }
    
    jas_stream_rewind(out);
    jas_stream_read(out, output_buf, len);
    *actual_out_size = (int)len;
    jas_image_destroy(image);
    jas_stream_close(out);
    return 0;
}
*/