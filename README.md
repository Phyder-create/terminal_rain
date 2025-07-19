# Terminal Rain & Lightning ⛈️

A calming and highly customizable terminal screensaver that simulates a rain and lightning storm, complete with audio effects. Perfect for creating a relaxing mood while you code or work.

## Features

- **Atmospheric Audio:** Looping rain sounds with periodic, distant thunderclaps.
- **Dynamic Lightning:** Procedurally generated lightning bolts that grow, fork, and fade.
- **Fully Customizable:** Control rain and lightning colors, thunder volume, and lightning frequency via command-line arguments.
- **Responsive Design:** Adapts to terminal window size changes.
- **Graceful Exit:** Cleans up the terminal and restores the cursor upon `Ctrl+C`.
- **Easy Installation:** Standard `make install` support for Linux and macOS.

## Dependencies

Before compiling, install the SFML (Simple and Fast Multimedia Library) for audio playback:

**Debian/Ubuntu**
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```

**Fedora/CentOS/RHEL**
```bash
sudo dnf install SFML-devel
```

**Arch Linux**
```bash
sudo pacman -S sfml
```

**macOS (with Homebrew)**
```bash
brew install sfml
```

## Installation

This project uses a standard Makefile for Linux and macOS.

1. **Clone the Repository**
   ```bash
   git clone https://github.com/Phyder-create/terminal_rain
   cd terminal-rain
   ```

2. **Compile the Program**
   ```bash
   make
   ```
   This creates the `terminal_rain` executable in the project directory.

3. **Install System-Wide**
   ```bash
   sudo make install
   ```
   - The executable will be installed to `/usr/local/bin`
   - Sound assets are installed to `/usr/local/share/terminal_rain`

## Usage

Once installed, you can run the program from any terminal window.

### Basic Usage

Start the simulation with default settings (calm storm, blue rain, white lightning):
```bash
terminal_rain
```

### Customization Options

You can customize the storm with these command-line arguments:

| Argument              | Description                               | Default     | Example                     |
|-----------------------|-------------------------------------------|-------------|-----------------------------|
| `--rain-color`        | Sets the color of the rain                | blue        | `--rain-color cyan`         |
| `--lightning-color`   | Sets the color of the lightning           | yellow      | `--lightning-color yellow`  |
| `--thunder-volume`    | Thunder volume (0-100)                    | 30          | `--thunder-volume 50`       |
| `--lightning-chance`  | % chance of lightning per frame           | 0.05         | `--lightning-chance 2.0`    |
| `--help`              | Shows the help message                    | N/A         | `--help`                    |

**Available Colors:** `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`

#### Example: A More Intense Storm
```bash
terminal_rain --rain-color white --lightning-color magenta --thunder-volume 60 --lightning-chance 1.5
```

## Customizing Audio

You may use your own `.wav` sound effects:

1. Prepare your replacement `rain.wav` and/or `thunder.wav` audio files.
2. In the project’s `sounds/` directory, replace the existing files (filenames must remain the same).
3. Reinstall assets (from the project root):
   ```bash
   sudo make install
   ```
   The program will use your custom sounds the next time it runs.

## Uninstallation

To remove the program and its data files:

```bash
sudo make uninstall
```

Run this from the original project directory.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
