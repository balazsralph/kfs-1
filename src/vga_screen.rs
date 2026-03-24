pub struct VGAScreen {
    video_address: u32,
    buffer: *mut u8,
    width: u32,
    height: u32,
    color: u8,
    row: u32,
    col: u32,
}

impl VGAScreen {
    pub unsafe fn new(video_address: u32) -> Self {
        let mut s = Self {
            video_address,
            buffer: video_address as *mut u8,
            width: 80,
            height: 25,
            color: 0x0F, // white on black
            row: 0,
            col: 0,
        };
        s.init(&video_address);
        s
    }

    fn init(&mut self, _video_address: &u32) {
        self.video_address = *_video_address;
        self.buffer = self.video_address as *mut u8;
        self.width = 80;
        self.height = 25;
    }

    fn print_byte(&mut self, _byte: u8) {
        if self.row >= self.height {
            self.scroll();
            self.row = self.height - 1;
        }
        let index = ((self.row * self.width + self.col) * 2) as usize;
        unsafe {
            self.buffer.add(index).write_volatile(_byte);
            self.buffer.add(index + 1).write_volatile(self.color);
        }
        self.col += 1;
        if self.col >= self.width {
            self.col = 0;
            self.row += 1;
        }
    }

    fn scroll(&mut self) {
        let row_bytes = (self.width * 2) as usize;
        for r in 1..self.height {
            let src = (r * self.width * 2) as usize;
            let dst = ((r - 1) * self.width * 2) as usize;
            unsafe {
                core::ptr::copy(self.buffer.add(src), self.buffer.add(dst), row_bytes);
            }
        }
        let last = ((self.height - 1) * self.width * 2) as usize;
        for c in 0..(self.width as usize) {
            unsafe {
                self.buffer.add(last + c * 2).write_volatile(0x20);
                self.buffer.add(last + c * 2 + 1).write_volatile(self.color);
            }
        }
    }

    pub fn print_string(&mut self, _string: &str) {
        for &b in _string.as_bytes() {
            if b == b'\n' {
                self.col = 0;
                self.row += 1;
                if self.row >= self.height {
                    self.scroll();
                    self.row = self.height - 1;
                }
            } else {
                self.print_byte(b);
            }
        }
    }

    pub fn clear_screen(&mut self) {
        let mut index = 0usize;
        let total = (self.width * self.height * 2) as usize;
        while index < total {
            unsafe {
                self.buffer.add(index).write_volatile(0x20);
                self.buffer.add(index + 1).write_volatile(self.color);
            }
            index += 2;
        }
        self.row = 0;
        self.col = 0;
    }
}  


