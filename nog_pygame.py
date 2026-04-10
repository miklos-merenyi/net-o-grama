#!/usr/bin/env python3
"""
Net-o-Grama Pygame Client
A functional equivalent of the SDL client (cli_sdl.c)
"""

import pygame
import sys
import os
import socket
import select
import time
import random
import string
import argparse
from enum import Enum

# Initialize Pygame
pygame.init()

# Sound support is optional; if the mixer fails or quiet mode is enabled, no audio is played.
SOUND_ENABLED = True
try:
    pygame.mixer.init()
except pygame.error:
    print("Warning: audio disabled because mixer could not initialize")
    SOUND_ENABLED = False

# Set up constants matching SDL client
WINDOW_WIDTH = 1024
WINDOW_HEIGHT = 720
PLAY_X = 20
PLAY_Y = 20
PLAY_W = 700
PLAY_H = 220
SCORE_X = 720
SCORE_Y = 20
SCORE_W = 280
SCORE_H = 220
GUESS_X = 20
GUESS_Y = 260
GUESS_W = 980
GUESS_H = 440

# Color palette for 4 players (matching ncurses colors)
PLAYER_COLORS = [
    (255, 0, 0),       # Player 0: Red
    (0, 255, 0),       # Player 1: Green
    (255, 255, 0),     # Player 2: Yellow
    (0, 255, 255)      # Player 3: Cyan
]

BG_COLOR = (12, 18, 32)
PANEL_COLOR = (22, 36, 56)
BORDER_COLOR = (100, 170, 230)
TEXT_COLOR = (240, 240, 240)
ACCENT_COLOR = (200, 170, 90)

# Load sounds
sounds = {}
sound_files = {
    'click-answer': 'audio/click-answer.wav',
    'click-shuffle': 'audio/click-shuffle.wav',
    'found': 'audio/found.wav',
    'found2': 'audio/found2.wav',
    'badword': 'audio/badword.wav',
    'clear': 'audio/clearword.wav',
    'shuffle': 'audio/shuffle.wav',
    'clock-tick': 'audio/clock-tick.wav',
    'duplicate': 'audio/duplicate.wav',
    'foundbig': 'audio/foundbig.wav'
}

def load_sound(path):
    if not SOUND_ENABLED:
        return None
    try:
        return pygame.mixer.Sound(path)
    except (pygame.error, FileNotFoundError):
        print(f"Warning: Could not load sound {path}")
        return None

for name, path in sound_files.items():
    sounds[name] = load_sound(path)


def play_sound(name):
    if not SOUND_ENABLED:
        return
    sound = sounds.get(name)
    if sound:
        try:
            sound.play()
        except pygame.error:
            pass

# Game states
class GameState(Enum):
    BEFORE = 0
    GETID = 1
    GETNAMES = 2
    GETNODES = 3
    GETSHUFFLE = 4
    COUNTDOWN = 5
    GAMEISON = 6
    WAITING = 7
    ENDED = 8

# Network constants
PORT = 5555
BUFFER_SIZE = 256
DELIM = '\0'

# Network constants
PORT = 5555
BUFFER_SIZE = 256
DELIM = '\0'

# Key mappings
key_SHUFFLE = ' '
key_CHECK = '\n'
key_CHECK_NORET = pygame.K_RIGHT
key_CLEAR = pygame.K_BACKSPACE
key_DELCHAR = pygame.K_LEFT
key_SOLVE = '\t'
key_QUIT = 27  # ESC

# Bitmap font data - 8x8 characters
BITMAP_FONT = {
    'A': [0x18,0x24,0x42,0x42,0x7E,0x42,0x42,0x42],
    'B': [0x7C,0x42,0x42,0x7C,0x42,0x42,0x42,0x7C],
    'C': [0x3C,0x42,0x40,0x40,0x40,0x40,0x42,0x3C],
    'D': [0x78,0x44,0x42,0x42,0x42,0x42,0x44,0x78],
    'E': [0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x7E],
    'F': [0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x40],
    'G': [0x3C,0x42,0x40,0x4E,0x42,0x42,0x42,0x3C],
    'H': [0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42],
    'I': [0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x3C],
    'J': [0x1E,0x08,0x08,0x08,0x08,0x48,0x48,0x30],
    'K': [0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x42],
    'L': [0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E],
    'M': [0x42,0x66,0x5A,0x5A,0x42,0x42,0x42,0x42],
    'N': [0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x42],
    'O': [0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C],
    'P': [0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x40],
    'Q': [0x3C,0x42,0x42,0x42,0x42,0x4A,0x44,0x3A],
    'R': [0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x42],
    'S': [0x3C,0x42,0x40,0x3C,0x02,0x02,0x42,0x3C],
    'T': [0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18],
    'U': [0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C],
    'V': [0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18],
    'W': [0x42,0x42,0x42,0x42,0x5A,0x5A,0x66,0x42],
    'X': [0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42],
    'Y': [0x42,0x42,0x42,0x24,0x18,0x18,0x18,0x18],
    'Z': [0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x7E],
    '0': [0x3C,0x42,0x62,0x52,0x4A,0x46,0x42,0x3C],
    '1': [0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x3C],
    '2': [0x3C,0x42,0x02,0x04,0x08,0x10,0x20,0x7E],
    '3': [0x3C,0x42,0x02,0x1C,0x02,0x02,0x42,0x3C],
    '4': [0x04,0x0C,0x14,0x24,0x44,0x7E,0x04,0x04],
    '5': [0x7E,0x40,0x40,0x7C,0x02,0x02,0x42,0x3C],
    '6': [0x3C,0x40,0x40,0x7C,0x42,0x42,0x42,0x3C],
    '7': [0x7E,0x02,0x04,0x08,0x10,0x10,0x10,0x10],
    '8': [0x3C,0x42,0x42,0x3C,0x42,0x42,0x42,0x3C],
    '9': [0x3C,0x42,0x42,0x42,0x3E,0x02,0x02,0x3C],
    ':': [0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00],
    '-': [0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00],
    '.': [0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00],
    ' ': [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
}

# Global game state
class GameClient:
    def __init__(self, username="player", server="localhost", port=PORT):
        self.window = None
        self.clock = None
        self.running = True
        
        # Network
        self.srv_socket = None
        self.username = username
        self.server = server
        self.port = port
        self.buffer = bytearray()
        
        # Game state
        self.state = GameState.BEFORE
        self.my_id = None
        self.nop = 0  # number of players
        self.gamers = []
        
        # Play field
        self.current_word = ""
        self.current_try = ""
        self.current_time = 0
        self.root_word = ""
        self.shuffle = "        "
        self.answer = ""
        self.answer_len = 0
        self.rem = "        "
        self.blank = [0] * 10
        self.del_answer = 1
        
        # Guess board
        self.head = None  # linked list of nodes
        self.prev_node = None
        self.node_id = 0
        self.ended_node = None
        
    def connect_to_server(self):
        """Connect to the game server."""
        try:
            self.srv_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.srv_socket.connect((self.server, self.port))
            self.srv_socket.setblocking(False)
            self.send_message(f"name: {self.username}")
            print(f"Connected to {self.server}:{self.port}")
            return True
        except Exception as e:
            print(f"Error connecting to server: {e}")
            return False
    
    def send_message(self, msg):
        """Send a message to the server."""
        try:
            self.srv_socket.send((msg + DELIM).encode())
        except Exception as e:
            print(f"Error sending message: {e}")
    
    def receive_messages(self):
        """Receive and process messages from server."""
        try:
            data = self.srv_socket.recv(BUFFER_SIZE)
            if data:
                self.buffer.extend(data)
                self.process_buffer()
        except BlockingIOError:
            pass
        except Exception as e:
            print(f"Error receiving message: {e}")
            self.running = False
    
    def process_buffer(self):
        """Process messages from buffer."""
        while DELIM.encode() in bytes(self.buffer):
            idx = bytes(self.buffer).index(DELIM.encode())
            msg = bytes(self.buffer[:idx]).decode('utf-8', errors='ignore')
            self.buffer = self.buffer[idx + 1:]
            self.handle_message(msg)
    
    def handle_message(self, msg):
        """Handle a message from the server."""
        if not msg:
            return
        
        print(f"[{self.state.name}] Received: {msg}")
        
        if self.state == GameState.BEFORE:
            if msg[0] == 'R':
                print("Rejected by server")
                self.running = False
            elif msg[0] == 'S':
                print("Starting new game")
                self.state = GameState.GETID
                self.reset_game()
        
        elif self.state == GameState.GETID:
            self.my_id = int(msg)
            self.state = GameState.GETNAMES
            print(f"Got ID: {self.my_id}")
        
        elif self.state == GameState.GETNAMES:
            if msg[0] == '.':
                print(f"Got all names, {self.nop} players total")
                self.state = GameState.GETNODES
            else:
                if self.nop < 4:
                    name = msg.strip()
                    self.gamers.append({'name': name[:8], 'score': 0, 'state': 1})
                    self.nop += 1
                    print(f"Got player: {name[:8]}")
        
        elif self.state == GameState.GETNODES:
            if msg[0] == '.':
                self.state = GameState.GETSHUFFLE
            else:
                length = int(msg)
                placeholder = 'X' * length
                new_node = {
                    'anagram': placeholder,
                    'found': 0,
                    'guessed': 0,
                    'length': length,
                    'id': self.node_id,
                    'next': None
                }
                if not self.head:
                    self.head = new_node
                else:
                    node = self.head
                    while node['next']:
                        node = node['next']
                    node['next'] = new_node
                self.prev_node = new_node
                self.node_id += 1
        
        elif self.state == GameState.GETSHUFFLE:
            self.root_word = msg
            self.shuffle = msg
            self.rem = msg
            self.answer = ""
            self.answer_len = 0
            self.state = GameState.COUNTDOWN
            self.init_play_field(self.rem)
        
        elif self.state == GameState.COUNTDOWN:
            if msg[0] == '.':
                print("Countdown complete, entering GAMEISON")
                self.state = GameState.GAMEISON
                self.current_time = 0
                self.init_play_field(self.rem)
                return
            else:
                self.update_play_field(msg, "      ")
                play_sound('clock-tick')
                return
        
        elif self.state in (GameState.GAMEISON, GameState.WAITING):
            if msg[0] == 'G':
                parts = msg.split(':')
                if len(parts) >= 5:
                    gid = int(parts[1])
                    word = parts[2]
                    nodeid = int(parts[3])
                    score = int(parts[4])
                    if gid < len(self.gamers):
                        self.gamers[gid]['score'] = score
                    node = self.head
                    idx = 0
                    while node and idx != nodeid:
                        node = node['next']
                        idx += 1
                    if node:
                        node['guessed'] = gid + 1
                        node['anagram'] = word
                    if gid == self.my_id:
                        self.clear_answer()
                        self.del_answer = 1
                        play_sound('found')
                    else:
                        play_sound('found2')
            elif msg[0] == 'F':
                print("Guess failed, clearing answer")
                self.clear_answer()
                play_sound('badword')
            elif msg[0] == 'T':
                try:
                    time_val = int(msg[2:])
                    self.current_time = time_val
                    if time_val <= 10:
                        play_sound('clock-tick')
                except:
                    pass
            elif msg[0] == 'E':
                self.state = GameState.ENDED
                self.ended_node = self.head
            elif msg[0] == '.':
                self.state = GameState.GAMEISON
                self.init_play_field(self.rem)
        
        elif self.state == GameState.ENDED:
            if msg[0] == '.':
                self.state = GameState.BEFORE
            else:
                # Fill in found words
                if self.ended_node:
                    self.ended_node['anagram'] = msg
                    if not self.ended_node['guessed']:
                        self.ended_node['guessed'] = 5  # Server found
                    self.ended_node = self.ended_node['next']
    
    def reset_game(self):
        """Reset game for new round."""
        self.head = None
        self.prev_node = None
        self.node_id = 0
        self.ended_node = None
        self.nop = 0
        self.gamers = []
        self.answer = ""
        self.answer_len = 0
    
    def init_play_field(self, word):
        """Initialize the play field."""
        self.current_word = word
        self.current_try = "       "
    
    def update_play_field(self, word, try_str):
        """Update the play field."""
        self.current_word = word
        self.current_try = try_str
    
    def clear_answer(self):
        """Clear the current answer."""
        if not self.del_answer:
            return
        self.answer = ""
        self.answer_len = 0
        self.rem = self.shuffle
        self.update_play_field(self.rem, self.answer)
    
    def shuffle_string(self, s):
        """Shuffle a string."""
        chars = list(s)
        random.shuffle(chars)
        return ''.join(chars)
    
    def where_in_str(self, s, char):
        """Find position of character in string."""
        try:
            return s.index(char)
        except ValueError:
            return -1
    
    def key_pressed(self, key):
        """Handle key press."""
        print(f"key_pressed called with key={repr(key)}, state={self.state.name}")
        
        if key == key_QUIT:
            self.send_message("quit")
            self.running = False
            return
        
        if self.state == GameState.BEFORE:
            if key == key_CHECK:
                print("Sending 'ready' to server")
                self.send_message("ready")
        
        elif self.state in (GameState.GAMEISON, GameState.WAITING):
            if key == key_SHUFFLE:
                print("Shuffling letters")
                self.rem = self.shuffle_string(self.rem)
                self.update_play_field(self.rem, self.answer)
                play_sound('shuffle')
            
            elif key in (key_CHECK_NORET, key_CHECK):
                self.del_answer = (key == key_CHECK)
                if self.answer:
                    print(f"Sending guess: {self.answer}")
                    self.send_message(f"g:{self.answer}")
                    self.state = GameState.WAITING
                    play_sound('click-answer')
                else:
                    print("No answer to submit")
            
            elif key == key_CLEAR:
                print("Clearing answer")
                self.del_answer = 1
                self.clear_answer()
                play_sound('clear')
            
            elif key == key_DELCHAR:
                print("Deleting last character")
                if self.answer_len > 0:
                    idx = self.blank[self.answer_len]
                    rem_list = list(self.rem)
                    rem_list[idx] = self.answer[self.answer_len - 1]
                    self.rem = ''.join(rem_list)
                    self.answer_len -= 1
                    self.answer = self.answer[:self.answer_len]
                    self.update_play_field(self.rem, self.answer)
            
            elif key == key_SOLVE:
                print("Giving up (solve)")
                self.send_message("solve")
            
            elif isinstance(key, str) and 'a' <= key <= 'z':
                print(f"Adding letter: {key}")
                pos = self.where_in_str(self.rem, key)
                if pos != -1:
                    self.answer += key
                    self.answer_len += 1
                    rem_list = list(self.rem)
                    rem_list[pos] = ' '
                    self.rem = ''.join(rem_list)
                    self.blank[self.answer_len] = pos
                    self.update_play_field(self.rem, self.answer)
                else:
                    print(f"Letter {key} not found in remaining letters")
            
            elif isinstance(key, str) and '1' <= key <= '7':
                print(f"Adding numbered letter: {key}")
                pos = int(key) - 1
                if pos < len(self.rem) and self.rem[pos] != ' ':
                    self.answer += self.rem[pos]
                    self.answer_len += 1
                    rem_list = list(self.rem)
                    rem_list[pos] = ' '
                    self.rem = ''.join(rem_list)
                    self.blank[self.answer_len] = pos
                    self.update_play_field(self.rem, self.answer)
        else:
            print(f"No action for key {repr(key)} in state {self.state.name}")


# Global client instance
client = None


def get_glyph(c):
    """Get bitmap data for a character."""
    key = c.upper()
    if key in BITMAP_FONT:
        return BITMAP_FONT[key]
    return BITMAP_FONT[' ']


def draw_text(surface, x, y, text, color, scale=2):
    """Draw text using bitmap font."""
    for i, char in enumerate(text):
        glyph = get_glyph(char)
        for row in range(8):
            for col in range(8):
                if glyph[row] & (1 << (7 - col)):
                    rect = pygame.Rect(
                        x + col * scale,
                        y + row * scale,
                        scale,
                        scale
                    )
                    pygame.draw.rect(surface, color, rect)
        x += (8 + 1) * scale


def draw_panel(surface, rect, bg_color, border_color):
    """Draw a panel with background and border."""
    pygame.draw.rect(surface, bg_color, rect)
    pygame.draw.rect(surface, border_color, rect, 2)


def render_screen(surface):
    """Render the game screen."""
    if not client:
        return
    
    surface.fill(BG_COLOR)

    play_rect = pygame.Rect(PLAY_X, PLAY_Y, PLAY_W, PLAY_H)
    score_rect = pygame.Rect(SCORE_X, SCORE_Y, SCORE_W, SCORE_H)
    guess_rect = pygame.Rect(GUESS_X, GUESS_Y, GUESS_W, GUESS_H)

    draw_panel(surface, play_rect, PANEL_COLOR, BORDER_COLOR)
    draw_panel(surface, score_rect, PANEL_COLOR, BORDER_COLOR)
    draw_panel(surface, guess_rect, PANEL_COLOR, BORDER_COLOR)

    draw_text(surface, PLAY_X + 16, PLAY_Y + 18, client.current_word, TEXT_COLOR, 5)
    draw_text(surface, PLAY_X + 16, PLAY_Y + 120, client.current_try, ACCENT_COLOR, 5)

    time_buffer = f"TIME:{client.current_time:3d}"
    draw_text(surface, SCORE_X + 10, SCORE_Y + 15, time_buffer, ACCENT_COLOR, 2)

    for i in range(client.nop):
        y = SCORE_Y + 15 + (i + 1) * 38
        player_color = PLAYER_COLORS[i % 4]
        name = client.gamers[i]['name'][:8] if i < len(client.gamers) else ""
        score = client.gamers[i]['score'] if i < len(client.gamers) else 0

        draw_text(surface, SCORE_X + 10, y, name, player_color, 2)
        draw_text(surface, SCORE_X + 162, y, ":", player_color, 2)
        draw_text(surface, SCORE_X + 180, y, f"{score:3d}", player_color, 2)

    if client.head:
        node = client.head
        col = 0
        row = 0
        guess_scale = 2
        guess_row_height = 26
        guess_col_width = 160
        max_rows = (GUESS_H - 20) // guess_row_height

        while node:
            x = GUESS_X + col * guess_col_width + 10
            y = GUESS_Y + row * guess_row_height + 10

            if node['guessed'] > 0:
                guess_color = PLAYER_COLORS[(node['guessed'] - 1) % 4]
                draw_text(surface, x, y, node['anagram'], guess_color, guess_scale)
            else:
                draw_text(surface, x, y, '-' * node['length'], ACCENT_COLOR, guess_scale)

            row += 1
            if row >= max_rows:
                row = 0
                col += 1
            node = node.get('next')

    pygame.display.flip()


def init_screen():
    """Initialize the pygame screen."""
    pygame.event.clear()  # Clear any pending events
    window = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
    pygame.display.set_caption("Net-o-Grama Pygame Client")
    clock = pygame.time.Clock()
    
    # Ensure the window is focused and ready for input
    pygame.event.set_allowed([pygame.KEYDOWN, pygame.KEYUP, pygame.QUIT])
    
    render_screen(window)
    print("Pygame screen initialized")
    return window, clock


def end_screen():
    """Clean up pygame."""
    pygame.quit()
    sys.exit()


def handle_input(event):
    """Handle keyboard input."""
    if not client:
        return
    
    if event.type == pygame.QUIT:
        print("QUIT event received")
        client.send_message("quit")
        client.running = False
        return

    if event.type != pygame.KEYDOWN:
        return

    print(f"DEBUG: KeyDown event - key={event.key}")
    key = event.key
    mapped = None

    if key == pygame.K_ESCAPE:
        mapped = key_QUIT
        print("DEBUG: ESCAPE mapped")
    elif key == pygame.K_SPACE:
        mapped = key_SHUFFLE
        print("DEBUG: SPACE mapped")
    elif key in (pygame.K_RETURN, pygame.K_KP_ENTER):
        mapped = key_CHECK
        print("DEBUG: RETURN mapped")
    elif key == pygame.K_RIGHT:
        mapped = key_CHECK_NORET
        print("DEBUG: RIGHT mapped")
    elif key == pygame.K_LEFT:
        mapped = key_DELCHAR
        print("DEBUG: LEFT mapped")
    elif key == pygame.K_BACKSPACE:
        mapped = key_CLEAR
        print("DEBUG: BACKSPACE mapped")
    elif key == pygame.K_TAB:
        mapped = key_SOLVE
        print("DEBUG: TAB mapped")
    elif pygame.K_a <= key <= pygame.K_z:
        mapped = chr(key)
        print(f"DEBUG: Letter {chr(key)} mapped")
    elif pygame.K_1 <= key <= pygame.K_7:
        mapped = chr(ord('1') + (key - pygame.K_1))
        print(f"DEBUG: Number {mapped} mapped")

    if mapped is not None:
        print(f"DEBUG: Calling key_pressed with {mapped}")
        client.key_pressed(mapped)
    else:
        print(f"DEBUG: No mapping for key {key}")


def heart_beat():
    """Send heartbeat to server."""
    if client and client.running:
        client.send_message("t")


def game_loop(window, clock):
    """Main game loop."""
    print("Starting game loop...")
    event_count = 0
    while client and client.running:
        # Poll and handle all events (prioritize event handling)
        event_list = pygame.event.get()
        if event_list:
            print(f"DEBUG: Got {len(event_list)} events in this frame")
            event_count += len(event_list)
        
        for event in event_list:
            handle_input(event)

        client.receive_messages()
        heart_beat()
        render_screen(window)
        
        # Cap at 60 FPS for better responsiveness
        clock.tick(60)
    
    print(f"Game loop ended. Total events processed: {event_count}")


def main():
    """Main entry point."""
    global client
    
    parser = argparse.ArgumentParser(description='Net-o-Grama Pygame Client')
    parser.add_argument('-s', '--server', default='localhost', help='Server address')
    parser.add_argument('-p', '--port', type=int, default=PORT, help=f'Server port (default: {PORT})')
    parser.add_argument('-n', '--name', default='player', help='Player nickname')
    parser.add_argument('-q', '--quiet', action='store_true', help='Disable sound effects')
    
    args = parser.parse_args()
    
    global SOUND_ENABLED
    if args.quiet:
        SOUND_ENABLED = False
    
    client = GameClient(username=args.name, server=args.server, port=args.port)
    
    window, clock = init_screen()
    
    if not client.connect_to_server():
        end_screen()
        return

    # Show welcome message before the game begins
    welcome_line = f"WELCOME"
    name_line = args.name.upper()[:8]
    client.update_play_field(welcome_line, name_line)
    render_screen(window)
    
    try:
        game_loop(window, clock)
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        if client and client.srv_socket:
            try:
                client.send_message("quit")
                client.srv_socket.close()
            except:
                pass
        end_screen()


if __name__ == "__main__":
    main()
