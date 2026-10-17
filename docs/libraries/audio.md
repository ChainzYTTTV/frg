# `<audio>`: Sounds and Music

```cpp
#include <audio>
```

Plays sound effects and music. Audio files go in the game folder and are found the same way as [assets](assets.md#where-files-go).

## Sound vs Music

| | `Sound` | `Music` |
|---|---|---|
| For | short effects: jump, hit, coin | long tracks: background music |
| Loaded | completely into memory | streamed from the file a bit at a time while it plays |
| Formats | `.wav`, `.ogg`, `.mp3`, `.flac`, `.qoa` | `.ogg`, `.mp3`, `.wav`, `.flac`, `.xm`, `.mod` |
| Loops by default | no | yes |

## Loading

| Function | Gives back |
|---|---|
| `audio.loadSound(string path)` | `Sound` |
| `audio.loadMusic(string path)` | `Music` |

Load once (as global variables, or at the start of the game), not every time a sound plays:

```cpp
Sound jump = audio.loadSound("sounds/jump.wav");
Music theme = audio.loadMusic("music/theme.ogg");
```

## Sound

| Method | What it does |
|---|---|
| `play()` | plays the sound from the start. Playing it again while it's playing restarts it. |
| `stop()` | stops it |
| `isPlaying()` | `true` while it's playing |
| `setVolume(float volume)` | 0.0 (silent) to 1.0 (full volume) |
| `setPitch(float pitch)` | 1.0 is normal; 2.0 is an octave higher and twice as fast, 0.5 an octave lower |

Tip: a slightly random pitch makes repeated sounds less tiring to hear: `jump.setPitch(random.range(0.9, 1.1)); jump.play();`

## Music

| Method | What it does |
|---|---|
| `play()` | starts from the beginning |
| `stop()` | stops it (the next `play()` starts from the beginning) |
| `pause()` | pauses it |
| `resume()` | carries on from where it was paused |
| `isPlaying()` | `true` while it's playing |
| `setVolume(float volume)` | 0.0 to 1.0 |
| `setLooping(bool looping)` | whether it starts over when it ends (on to start with) |
| `length()` | the track's length in seconds (`float`) |
| `position()` | how far through it is, in seconds (`float`) |

Music has to be fed to the speakers a little at a time. **With a window open, Forge does this every frame by itself.** In a program without a window, call `audio.update()` often (many times a second), or the music stutters and stops.

## Volume

| Function | What it does |
|---|---|
| `audio.setVolume(float volume)` | the volume of everything, 0.0 to 1.0 |
| `audio.update()` | keeps music playing in programs without a window |

## No sound device

If the computer has no working sound device, Forge prints `[audio] no sound device found - sounds will be silent` once. Everything else keeps working; sounds just don't make any noise.

## Example

```cpp
#include <window>
#include <input>
#include <audio>

Sound coin = audio.loadSound("coin.wav");
Music song = audio.loadMusic("song.ogg");

int main(){
    window.open(800, 450, "Audio");
    song.setVolume(0.6);
    song.play();
    window.run();
    return 0;
}

void update(float dt){
    if (input.keyPressed(Key.SPACE)) {
        coin.play();
    }
    if (input.keyPressed(Key.M)) {
        if (song.isPlaying()) {
            song.pause();
        } else {
            song.resume();
        }
    }
}

void draw(){
}
```

## Limits

- **One `Sound` plays one copy at a time**: playing it while it's still playing starts it again from the beginning. For sounds that overlap (fast gunfire, many coins at once), load the same file into two or three `Sound`s and take turns.
- There's no left/right panning, no sounds placed in a 3D world, and no effects (echo, filters).
- Music can't jump to a point in the track: `position()` only says where it is.
