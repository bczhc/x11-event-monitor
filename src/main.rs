use std::ptr::null;
use std::thread::spawn;

use chrono::Utc;
use libc::c_int;
use winit::event::{DeviceEvent, ElementState, Event, MouseScrollDelta};
use winit::event_loop::ControlFlow;
use x11_clipboard::Clipboard;

const KEYS: &str = include_str!("./keys");

fn main() {
    spawn(start_clipboard_monitor);
    spawn(start_primary_selection_monitor);

    let mut keys = vec![None; 256];
    for line in KEYS.lines().filter(|x| !x.is_empty()) {
        let mut split = line.split_whitespace();
        let name = split.next().unwrap();
        let code = split.next().unwrap().parse::<usize>().unwrap();
        keys[code] = Some(String::from(name));
    }

    let xdo = XDo::new().unwrap();

    let event_loop = winit::event_loop::EventLoop::new();
    event_loop.run(move |e, _, cf| {
        *cf = ControlFlow::Wait;

        let time = Utc::now().timestamp_millis();
        if let Event::DeviceEvent { event, .. } = e {
            match event {
                DeviceEvent::MouseMotion { .. } => {
                    let location = xdo.mouse_location().unwrap();
                    println!("MouseMotion {} {} {}", time, location.0, location.1);
                }
                DeviceEvent::MouseWheel {
                    delta: MouseScrollDelta::LineDelta(x, y),
                } => {
                    println!("MouseWheel {} {} {}", time, x, y);
                }
                DeviceEvent::Button { button, state } => {
                    if state == ElementState::Pressed {
                        let location = xdo.mouse_location().unwrap();
                        let button_name = match button {
                            1 => "Left",
                            2 => "Middle",
                            3 => "Right",
                            _ => "Unknown",
                        };
                        println!(
                            "Button {} {} {} {}",
                            time, location.0, location.1, button_name
                        );
                    }
                }
                DeviceEvent::Key(k) => {
                    if k.state == ElementState::Pressed || k.state == ElementState::Released {
                        let code_name = keys[k.scancode as usize].as_ref();
                        let code_name = match code_name {
                            None => "UNKNOWN",
                            Some(s) => s,
                        };
                        let prefix = match k.state {
                            ElementState::Pressed => "KeyPress",
                            ElementState::Released => "KeyRelease",
                        };

                        println!("{} {} {} {}", prefix, time, k.scancode, code_name);
                    }
                }
                _ => {}
            }
        }
    });
}

fn start_clipboard_monitor() {
    let clipboard = Clipboard::new().unwrap();

    loop {
        let Ok(val) = clipboard
            .load_wait(
                clipboard.setter.atoms.clipboard,
                clipboard.setter.atoms.string,
                clipboard.setter.atoms.property,
            ) else {
            continue
        };

        let escaped = bczhc_lib::str::escape_utf8_bytes(&val);

        let time = Utc::now().timestamp_millis();
        println!("Clipboard {} {}", time, escaped);
    }
}

fn start_primary_selection_monitor() {
    let clipboard = Clipboard::new().unwrap();
    loop {
        let Ok(vec) = clipboard.load_wait(
            clipboard.getter.atoms.primary,
            clipboard.getter.atoms.utf8_string,
            clipboard.getter.atoms.property,
        ) else {
            continue
        };

        let escaped = bczhc_lib::str::escape_utf8_bytes(&vec);

        let time = Utc::now().timestamp_millis();
        println!("Selection {} {}", time, escaped);
    }
}

struct XDo {
    inner: *mut libxdo_sys::xdo_t,
}

impl XDo {
    fn new() -> Option<XDo> {
        let xdo = unsafe { libxdo_sys::xdo_new(null()) };
        if xdo.is_null() {
            return None;
        }

        Some(Self { inner: xdo })
    }

    fn mouse_location(&self) -> Option<(i32, i32)> {
        let mut x = 0;
        let mut y = 0;
        let mut screen_num = 0;

        let r = unsafe {
            libxdo_sys::xdo_get_mouse_location(
                self.inner,
                &mut x as *mut c_int,
                &mut y as *mut c_int,
                &mut screen_num as *mut c_int,
            )
        };
        if r != 0 {
            return None;
        }

        Some((x, y))
    }
}
