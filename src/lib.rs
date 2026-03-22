
use image::{RgbaImage, ImageBuffer};
unsafe extern "C" {
    fn get_jp2_info(input: *const u8, input_len: i32, w: *mut i32, h: *mut i32,
        xr: *mut f32, yr: *mut f32, dpi: *mut u16, errbuf: *mut i8,err_len: i32, decoder: *mut usize) -> i32;
    fn fill_jp2_rgba(output: *mut u8, decoder: usize) -> i32;
    //#[warn(unused)]
    //fn my_decode_destroy(decoder: usize);
    fn encode_rgba_to_jp2_mem( rgb_input: *const u8, w: i32, h: i32, chan: i32, quality: u8,
        xres: f32, yres: f32, dpi: u16, exif: *const u8, exif_len: i32, jp2: u8,
        output_buf: *mut u8, max_out_size: i32, actual_out_size: *mut i32, errbuf: *mut i8,err_len: i32) -> i32;
}

fn decode_c_string(bytes: &[u8]) -> Result<String, std::string::FromUtf8Error> {
    let end = bytes.iter().position(|&b| b == 0).unwrap_or(bytes.len());
    String::from_utf8(bytes[..end].to_vec())
}

pub fn load_jp2_from_memory(data: &[u8]) -> Result<(RgbaImage, f32, f32, u16, String), String> {
    let mut w = 0i32;
    let mut h = 0i32;
    let mut dpi = 0u16;
    let mut xr = 0.0f32;
    let mut yr = 0.0f32;
    
    // 512 bájtos puffer a hibaüzenetnek
    let mut err_buf = [0u8; 512];

    unsafe {
        // Info lekérése
        let mut decoder : usize = 0;
        let res_info = get_jp2_info( data.as_ptr(), data.len() as i32, 
            &mut w, &mut h, &mut xr, &mut yr, &mut dpi,
            err_buf.as_mut_ptr() as *mut i8, err_buf.len() as i32, &mut decoder );

        if res_info != 0 {
            return Err(decode_c_string(&err_buf).unwrap_or_else(|_| "Ismeretlen hiba az info lekérésekor".to_string()));
        }

        // Pixel adatok helye
        let buffer_size = (w * h * 4) as usize;
        let mut pixels = vec![0u8; buffer_size];
        
        // Pixel adatok betöltése
        let res_fill = fill_jp2_rgba( pixels.as_mut_ptr(), decoder );

        if res_fill != 0 {
            return Err(decode_c_string(&err_buf).unwrap_or_else(|_| "Error dedecoding".to_string()));
        }

        // ImageBuffer létrehozása
        let img = ImageBuffer::from_raw(w as u32, h as u32, pixels)
            .ok_or_else(|| "Nem sikerült létrehozni a kép-puffert".to_string())?;

        Ok((img, xr, yr, dpi, decode_c_string(&err_buf).unwrap_or_else(|_| "".to_string())))
    }
}

pub fn save_rgba_to_jp2(img: &image::DynamicImage, jp2: u8, quality: u8, xres:f32, yres:f32, dpi: bool, exif: Vec<u8> ) -> Result<(Vec<u8>,String), String> {
    let w = img.width();
    let h = img.height();
    let (ptr, chan) = match &img {
        image::DynamicImage::ImageRgb8(rgb) => (rgb.as_ptr(), 3),
        image::DynamicImage::ImageRgba8(rgba) => (rgba.as_ptr(), 4),
        _ => unreachable!(),
    };
    
    // Kezdjünk egy nagy pufferrel (vagy becsüljük meg a méretet)
    let siz = w as usize * h as usize * 4 + exif.len();
    let mut out_buffer = vec![0u8; siz]; 
    let mut actual_size = 0i32;
    let mut err_buf = [0u8; 512];

    unsafe {
        let dp : u16 = if dpi {1} else {0};
        let res = encode_rgba_to_jp2_mem(
            ptr, w as i32, h as i32, chan as i32,
            quality, xres, yres, dp, exif.as_ptr(), exif.len() as i32,
            jp2, out_buffer.as_mut_ptr(), out_buffer.len() as i32, &mut actual_size,
            err_buf.as_mut_ptr() as *mut i8, err_buf.len() as i32 );

        if res == 0 {
            out_buffer.truncate(actual_size as usize);
            let wr = format!("{:?}",actual_size);
            return Ok(( out_buffer, decode_c_string(&err_buf).unwrap_or_else(|_| wr.to_string()) ))
        } else {
            return Err(decode_c_string(&err_buf).unwrap_or_else(|_| "Error encoding".to_string()));
        }
    }
}

