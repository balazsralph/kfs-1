#[no_mangle]
pub extern "C" fn kernel_main(_magic: u32, _info: u32) -> ! {
    // TODO: interface écran + afficher "42"
    loop {
        core::hint::spin_loop();
    }
}