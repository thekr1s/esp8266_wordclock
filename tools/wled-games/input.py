import ctypes
import os
import argparse
from pathlib import Path
import threading
import keyboard
import sdl2
import sdl2.ext
import curses
import select
import sys

DIR_UP = (0, -1)
DIR_RIGHT = (1, 0)
DIR_DOWN = (0, 1)
DIR_LEFT = (-1, 0)

def get_parsed_args():
    # Make parser object
    p = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    
    p.add_argument("--host", default="localhost",
                   help="Hostname or IP-address of WLED device")
    p.add_argument("--port", type=int, default=21324,
                   help="UDP port number for DRGB protocol")
    p.add_argument("--columns", type=int, default=13,
                   help="The number of columns from the display")
    p.add_argument("--rows", type=int, default=13,
                   help="The number of rows from the display")
    p.add_argument("--leds_per_pixel", type=int, default=1,
                   help="The number leds per pixel from the display")
    p.add_argument("--pixel_size", type=int, default=50,
                   help="The pixel size in the simulator")

    return(p.parse_args())

def get_resource_dir():
    filepath = Path(__file__).parents[0].resolve()
    return os.path.join(filepath, "resources")

class Joystick():
    def __init__(self):
        self.direction = None
        self.thread = threading.Thread(target=self._listen_joystick)
        self.thread.daemon = True
        self.thread.start()

    def _listen_joystick(self):
        sdl2.SDL_Init(sdl2.SDL_INIT_JOYSTICK)
        event = sdl2.SDL_Event()
        i=0
        while True:
            sdl2.SDL_WaitEvent(ctypes.byref(event))
            if event.type == sdl2.SDL_JOYDEVICEADDED:
                self.device = sdl2.SDL_JoystickOpen(event.jdevice.which)
            elif event.type == sdl2.SDL_JOYHATMOTION:
                if event.jhat.value == 1: # Up
                    self.direction = DIR_UP
                elif event.jhat.value == 4: # Down
                    self.direction = DIR_DOWN
                elif event.jhat.value == 2: # Right
                    self.direction = DIR_RIGHT
                elif event.jhat.value == 8: # Left
                    self.direction = DIR_LEFT

class KeyboardLinux():
    def __init__(self):
        self.direction = None
        self.thread = threading.Thread(target=self._listen_keyboard)
        self.thread.daemon = True
        self.thread.start()

    def _listen_keyboard(self):
        sdl2.ext.init()
        window = sdl2.ext.Window("SDL2 Keyboard Events", size=(800, 600))
        window.show()
        running = True
        while running:
            events = sdl2.ext.get_events()
            for event in events:
                if event.type == sdl2.SDL_QUIT:
                    running = False
                    break
                elif event.type == sdl2.SDL_KEYDOWN:
                    if event.key.keysym.sym == sdl2.SDLK_UP:
                        self.direction = DIR_UP
                    elif event.key.keysym.sym == sdl2.SDLK_DOWN:
                        self.direction = DIR_DOWN
                    elif event.key.keysym.sym == sdl2.SDLK_RIGHT:
                        self.direction = DIR_RIGHT
                    elif event.key.keysym.sym == sdl2.SDLK_LEFT:
                        self.direction = DIR_LEFT

class KeyboardLinuxV2():
    def __init__(self):
        self.p1_dir = None
        self.p2_dir = None
        self.thread = threading.Thread(target=curses.wrapper, args=(self._listen_keyboard,))
        self.thread.daemon = True
        self.thread.start()

    def _listen_keyboard(self, stdscr):
        stdscr.addstr(0, 0, "Listening for arrow keys... (Press 'ctrl-c' to quit)")
        while True:
            key = stdscr.getch()
            if key == curses.KEY_UP:
                self.p1_dir = DIR_UP
            elif key == curses.KEY_DOWN:
                self.p1_dir = DIR_DOWN
            elif key == curses.KEY_RIGHT:
                self.p1_dir = DIR_RIGHT
            elif key == curses.KEY_LEFT:
                self.p1_dir = DIR_LEFT
            elif chr(key) == 'w':
                self.p2_dir = DIR_UP
            elif chr(key) == 's':
                self.p2_dir = DIR_DOWN
            elif chr(key) == 'd':
                self.p2_dir = DIR_RIGHT
            elif chr(key) == 'a':
                self.p2_dir = DIR_LEFT
    def get_dir_p1(self):
        return self.p1_dir
    def get_dir_p2(self):
        return self.p2_dir

class KeyboardLinuxV3():
    def __init__(self):
        self.direction = None
        self.thread = threading.Thread(target=self._listen_keyboard)
        self.thread.daemon = True
        self.thread.start()

    def _listen_keyboard(self):
        
        while True:
            if select.select([sys.stdin,], [], [], 0)[0]:
                key = sys.stdin.read(1)
            else:
                continue
            if key.upper() == 'W':
                self.direction = DIR_UP
            elif key.upper() == 'S':
                self.direction = DIR_DOWN
            elif key.upper() == 'D':
                self.direction = DIR_RIGHT
            elif key.upper() == 'A':
                self.direction = DIR_LEFT
            elif key == '\x1b': # arrow keys are escaped
                key = sys.stdin.read(1)
                if key == '[':
                    key = sys.stdin.read(1)
                    if key == 'A':
                        self.direction = DIR_UP
                    elif key == 'B':
                        self.direction = DIR_DOWN
                    elif key == 'C':
                        self.direction = DIR_RIGHT
                    elif key == 'D':
                        self.direction = DIR_LEFT


class KeyboardWin():
    def __init__(self):
        # direction is (X, Y)
        self.direction = None
        self.input = None
        self.thread = threading.Thread(target=self._listen_keyboard)
        self.thread.daemon = True
        self.thread.start()

        print("Waiting for input (W, S, A, D)")
        print("Waiting for input (Up, Down, Left, Right)")

    def _listen_keyboard(self):
        while True:
            event = keyboard.read_event()
            if event.name == "up" or event.name.upper() == 'W':
                self.direction = DIR_UP
            if event.name == "down" or event.name.upper() == 'S':
                self.direction = DIR_DOWN
            if event.name == "right" or event.name.upper() == 'D':
                self.direction = DIR_RIGHT
            if event.name == "left" or event.name.upper() == 'A':
                self.direction = DIR_LEFT
            self.input = event.name
            print(f'Last character: {event.name}')
