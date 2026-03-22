

In the lib.rs contained two function for jpeg 2000 images encoding and decoding.
It reads from memory and writes to memory. If you want to work with files, you have to handle their writing and reading yourself. Only the writing of the exif information is programmed, you have to do the reading yourself after reading the image. This is also the way to go with other rust image readers, because they do not handle either the exif info or the image resolution. However, this can be crucial for printing.


pub fn load_jp2_from_memory(data: &[u8]) -> Result<(RgbaImage, f32, f32, u16, String), String> {}

pub fn save_rgba_to_jp2(img: &image::DynamicImage, jp2: u8, quality: u8, xres:f32, yres:f32, dpi: bool, exif: Vec<u8> ) -> Result<(Vec<u8>,String), String> {}

