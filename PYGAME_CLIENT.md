# Net-o-Grama Pygame Client

## Overview

`nog_pygame.py` is a Python-based client for the Net-o-Grama networked word game, providing a functional equivalent to the SDL client (`src/cli_sdl.c`).

## Features

### UI Components
- **Display Areas** matching SDL client layout:
  - Play field (top left): Shows shuffled letters and current answer
  - Score board (top right): Shows player names and scores (max 4 players)
  - Guess board (bottom): Displays all guesses with player colors

### Rendering
- Bitmap font rendering (8x8 character glyphs) for authentic retro styling
- Color-coded player display (Red, Green, Yellow, Cyan)
- Styled panels with borders and themed colors

### Game Logic
- Full game state machine (BEFORE, GETID, GETNAMES, GETNODES, GETSHUFFLE, GAMEISON, WAITING, ENDED)
- Network protocol implementation matching C server
- Complete keyboard controls matching SDL client
- Answer management (add letters, delete, clear, shuffle)
- Scorer tracking and guess board updates

### Network Features
- TCP socket communication with game server
- Message buffering and parsing (null-delimited protocol)
- Non-blocking receive operations
- Automatic heartbeat/keepalive messages
- Support for custom server address and port

## Requirements

- Python 3.6+
- Pygame library

### Installation

```bash
pip install pygame
```

## Usage

### Basic Usage

```bash
./nog_pygame.py
```

### Command Line Options

```bash
./nog_pygame.py -s localhost -p 5555 -n playername
```

- `-s, --server`: Server address (default: localhost)
- `-p, --port`: Server port (default: 5555)
- `-n, --name`: Player nickname (default: player)

## Keyboard Controls

| Key | Action |
|-----|--------|
| `Space` | Shuffle remaining letters |
| `Enter` | Submit guess and clear answer |
| `Right Arrow` | Submit guess without clearing |
| `Backspace` | Clear entire answer |
| `Left Arrow` | Delete last character |
| `Tab` | Give up (solve) |
| `Esc` | Quit game |
| `a-z` | Add letter to answer |
| `1-7` | Add numbered letter position |

## Game States

1. **BEFORE**: Waiting to connect/start game
2. **GETID**: Receiving player ID from server
3. **GETNAMES**: Receiving list of player names
4. **GETNODES**: Receiving word list (anagrams)
5. **GETSHUFFLE**: Receiving shuffled word
6. **GAMEISON**: Active game play
7. **WAITING**: Waiting for server response to guess
8. **ENDED**: Game over, showing results

## Architecture

### GameClient Class

Main class handling:
- Network connectivity
- Game state management
- Keyboard input processing
- Answer/scoring logic

### Key Methods

- `connect_to_server()`: Establish TCP connection
- `send_message(msg)`: Send message to server
- `receive_messages()`: Poll for incoming messages
- `process_buffer()`: Parse buffered messages
- `handle_message(msg)`: Process server message
- `key_pressed(key)`: Handle keyboard input
- `key_pressed` actions:
  - Letter input: Add letter to answer
  - Shuffle: Randomize remaining letters
  - Submit: Send answer to server
  - etc.

### Rendering Functions

- `render_screen()`: Main rendering pipeline
- `draw_text()`: Bitmap font text rendering
- `draw_panel()`: Panel backgrounds and borders
- `get_glyph()`: Bitmap font character lookup

## Network Protocol

The client implements the following message protocol:

### Client → Server
- `name: <name>`: Set player name
- `ready`: Ready for game start
- `g:<word>`: Submit guess
- `t`: Heartbeat/keepalive
- `solve`: Give up
- `quit`: Disconnect

### Server → Client
- `S`: Start game
- `R`: Reject (connection refused)
- `<id>`: Player ID
- `<name>`: Player name
- `.`: End of list marker
- `<anagram>`: Word anagram
- `<shuffled>`: Shuffled word
- `G:<id>:<word>:<nodeid>:<score>`: Guess result
- `T:<time>`: Time remaining
- `END`: Game ended
- `E:<error>`: Error message

## Differences from SDL Client

The pygame implementation is a **functional equivalent** with these considerations:

### Advantages
- Pure Python implementation (no C dependencies)
- Cross-platform compatibility
- Easier to modify/extend
- Standalone executable (no need to recompile C code)

### Design Differences
- Uses classes for state management instead of C structs
- Dictionary-based node list instead of linked list
- Pygame for rendering instead of SDL2
- Non-blocking I/O instead of select() with file descriptors

### Compatibility
- Maintains exact same network protocol
- Identical UI layout and color scheme
- Same keyboard controls and game logic
- Compatible with existing game servers

## Testing

To test with the included server:

```bash
# Terminal 1 - Start server
./nog_srv

# Terminal 2 - Start pygame client
./nog_pygame.py -n player1

# Terminal 3 - Start another client  
./nog_pygame.py -s localhost -n player2
```

## Troubleshooting

### Connection Refused
- Ensure server is running on the specified port
- Check firewall settings
- Verify hostname/IP address

### No Input Response
- Window must be focused
- Check for errors in terminal output
- Verify pygame installation

### Rendering Issues
- Ensure pygame display is supported on your system
- Try updating pygame: `pip install --upgrade pygame`

## Implementation Details

### Bitmap Font
Uses fixed 8x8 bitmap glyphs for retro styling, matching SDL client rendering exactly.

### Buffer Management
Network messages use null-terminated strings. The client maintains a byte buffer and processes complete messages when delimiters are found.

### Game Loop
- Polls pygame events at 100 FPS
- Receives network messages non-blocking
- Sends heartbeat every loop iteration
- Renders screen at 100 FPS

## Future Enhancements

Potential improvements:
- Sound effect support (matching SDL_mixer features)
- Configuration file support
- Better error recovery
- Game statistics/replay logging
- Multiple connection profiles

## License

Same as main Net-o-Grama project

## See Also

- Original SDL client: [src/cli_sdl.c](src/cli_sdl.c)
- Game server: [src/nog_srv.c](src/nog_srv.c)
- Ncurses client: [src/nog_ncurses.c](src/nog_ncurses.c)
- Network protocol: [src/network.h](src/network.h)
