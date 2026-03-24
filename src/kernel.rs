use crate::vga_screen::VGAScreen;

#[no_mangle]
pub extern "C" fn kernel_main(_magic: u32, _info: u32) -> ! {
    let video_address = 0xB8000u32;
    let mut vga = unsafe { VGAScreen::new(video_address) };
    vga.clear_screen();
    vga.print_string("Hello, kfs!\n");

    loop {
        core::hint::spin_loop();
    }
}