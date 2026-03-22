// build.rs
//use std::env;
//use std::path::PathBuf;


fn main() {
    println!("cargo:rerun-if-changed=vendor");

    cc::Build::new()
        .include("vendor/openjp2")
        .file("vendor/openjp2/lib.c")
        .file("vendor/openjp2/thread.c")
        .file("vendor/openjp2/bio.c")
        .file("vendor/openjp2/cio.c")
        .file("vendor/openjp2/dwt.c")
        .file("vendor/openjp2/event.c")
        .file("vendor/openjp2/ht_dec.c")
        .file("vendor/openjp2/image.c")
        .file("vendor/openjp2/invert.c")
        .file("vendor/openjp2/j2k.c")
        .file("vendor/openjp2/jp2.c")
        .file("vendor/openjp2/mct.c")
        .file("vendor/openjp2/mqc.c")
        .file("vendor/openjp2/openjpeg.c")
        .file("vendor/openjp2/opj_clock.c")
        .file("vendor/openjp2/pi.c")
        .file("vendor/openjp2/t1.c")
        .file("vendor/openjp2/t2.c")
        .file("vendor/openjp2/tcd.c")
        .file("vendor/openjp2/tgt.c")
        .file("vendor/openjp2/function_list.c")
        .file("vendor/openjp2/opj_malloc.c")
        .file("vendor/openjp2/sparse_array.c")
        .define("_CRT_FUNCTIONS_REQUIRED=1", None)    // Ha vannak preprocessor flagjeid
        .compile("my-jp2-sys");          // Ebből lesz a libmyjp2.a vagy myjp2.lib

    /*  // my old jp2 
    cc::Build::new()
        .include("vendor/jp2")
        .include("vendor") // A header fájljaid helye
        .file("vendor/jp2/base/jas_cm.cpp") 
        .file("vendor/jp2/base/jas_debug.cpp") 
        .file("vendor/jp2/base/jas_getopt.cpp") 
        .file("vendor/jp2/base/jas_icc.cpp") 
        .file("vendor/jp2/base/jas_iccdata.cpp") 
        .file("vendor/jp2/base/jas_image.cpp") 
        .file("vendor/jp2/base/jas_init.cpp") 
        .file("vendor/jp2/base/jas_malloc.cpp") 
        .file("vendor/jp2/base/jas_seq.cpp") 
        .file("vendor/jp2/base/jas_stream.cpp") 
        .file("vendor/jp2/base/jas_string.cpp") 
        .file("vendor/jp2/base/jas_tvp.cpp") 
        .file("vendor/jp2/base/jas_version.cpp") 
        .file("vendor/jp2/bmp/bmp_cod.cpp") 
        .file("vendor/jp2/bmp/bmp_dec.cpp") 
        .file("vendor/jp2/bmp/bmp_enc.cpp") 
        .file("vendor/jp2/jp2/jp2_cod.cpp") 
        .file("vendor/jp2/jp2/jp2_dec.cpp") 
        .file("vendor/jp2/jp2/jp2_enc.cpp") 
        .file("vendor/jp2/jpc/jpc_bs.cpp") 
        .file("vendor/jp2/jpc/jpc_cs.cpp") 
        .file("vendor/jp2/jpc/jpc_dec.cpp") 
        .file("vendor/jp2/jpc/jpc_enc.cpp") 
        .file("vendor/jp2/jpc/jpc_math.cpp") 
        .file("vendor/jp2/jpc/jpc_mct.cpp") 
        .file("vendor/jp2/jpc/jpc_mqcod.cpp") 
        .file("vendor/jp2/jpc/jpc_mqdec.cpp") 
        .file("vendor/jp2/jpc/jpc_mqenc.cpp") 
        .file("vendor/jp2/jpc/jpc_qmfb.cpp") 
        .file("vendor/jp2/jpc/jpc_t1cod.cpp") 
        .file("vendor/jp2/jpc/jpc_t1dec.cpp") 
        .file("vendor/jp2/jpc/jpc_t1enc.cpp") 
        .file("vendor/jp2/jpc/jpc_t2cod.cpp") 
        .file("vendor/jp2/jpc/jpc_t2dec.cpp") 
        .file("vendor/jp2/jpc/jpc_t2enc.cpp") 
        .file("vendor/jp2/jpc/jpc_tagtree.cpp") 
        .file("vendor/jp2/jpc/jpc_tsfb.cpp") 
        .file("vendor/jp2/jpc/jpc_util.cpp") 
        .define("_CRT_FUNCTIONS_REQUIRED=1", None)    // Ha vannak preprocessor flagjeid
        .compile("my-jp2-sys");          // Ebből lesz a libmyjp2.a vagy myjp2.lib
        */
        
    /*let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Nem sikerült generálni a kötéseket");

    // 3. Írás az OUT_DIR mappába
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Nem sikerült kiírni a bindings.rs fájlt");*/
        
}
