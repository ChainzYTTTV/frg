// Forge runtime: <audio> - sound effects and music.
//
// Sounds are loaded completely into memory (best for short effects). Music is streamed from the file
// while it plays (best for long songs); the window keeps it playing every frame.
#pragma once

#include "forge_window.hpp"

namespace frg::audio {

struct Sound_;
struct Music_;
inline std::vector<Sound_*>& liveSounds() {
    static std::vector<Sound_*> v;
    return v;
}
inline std::vector<Music_*>& liveMusic() {
    static std::vector<Music_*> v;
    return v;
}

struct State {
    bool started = false;
    bool ready = false;  // false if the computer has no sound device
};
inline State& st() {
    static State s;
    return s;
}

inline void updateMusic();
inline void closeAudio();

inline bool ensureDevice() {
    State& s = st();
    if (!s.started) {
        s.started = true;
        SetTraceLogLevel(LOG_WARNING);
        InitAudioDevice();
        s.ready = IsAudioDeviceReady();
        if (!s.ready) std::fprintf(stderr, "[audio] no sound device found - sounds will be silent\n");
        // FRG_AUTOTEST_HIDDEN=1: a test running out of sight is out of earshot too
        if (s.ready && std::getenv("FRG_AUTOTEST_HIDDEN")) SetMasterVolume(0);
        addHook(frameHooks(), &updateMusic);
        addHook(shutdownHooks(), &closeAudio);
    }
    return s.ready;
}

struct Sound_ : std::enable_shared_from_this<Sound_> {
    ::Sound sound{};
    bool loaded = false;
    Sound_() = default;
    Sound_(const Sound_&) = delete;
    virtual ~Sound_() {
        auto& live = liveSounds();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        if (loaded && st().ready) UnloadSound(sound);
    }
    static std::shared_ptr<Sound_> frg$new() { return std::make_shared<Sound_>(); }
    virtual void play_() { if (loaded) PlaySound(sound); }
    virtual void stop_() { if (loaded) StopSound(sound); }
    virtual bool isPlaying_() { return loaded && IsSoundPlaying(sound); }
    virtual void setVolume_(float v) { if (loaded) SetSoundVolume(sound, v); }
    virtual void setPitch_(float p) { if (loaded) SetSoundPitch(sound, p); }
};

struct Music_ : std::enable_shared_from_this<Music_> {
    ::Music music{};
    bool loaded = false;
    bool playing = false;
    Music_() = default;
    Music_(const Music_&) = delete;
    virtual ~Music_() {
        auto& live = liveMusic();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        if (loaded && st().ready) UnloadMusicStream(music);
    }
    static std::shared_ptr<Music_> frg$new() { return std::make_shared<Music_>(); }
    virtual void play_() {
        if (!loaded) return;
        PlayMusicStream(music);
        playing = true;
    }
    virtual void stop_() {
        if (!loaded) return;
        StopMusicStream(music);
        playing = false;
    }
    virtual void pause_() {
        if (!loaded) return;
        PauseMusicStream(music);
        playing = false;
    }
    virtual void resume_() {
        if (!loaded) return;
        ResumeMusicStream(music);
        playing = true;
    }
    virtual bool isPlaying_() { return loaded && IsMusicStreamPlaying(music); }
    virtual void setVolume_(float v) { if (loaded) SetMusicVolume(music, v); }
    virtual void setLooping_(bool loop) { music.looping = loop; }
    virtual float length_() { return loaded ? GetMusicTimeLength(music) : 0; }
    virtual float position_() { return loaded ? GetMusicTimePlayed(music) : 0; }
};

// Music has to be fed to the speakers a little at a time; this runs once per frame.
inline void updateMusic() {
    for (Music_* m : liveMusic())
        if (m->playing) UpdateMusicStream(m->music);
}

inline void closeAudio() {
    State& s = st();
    if (!s.ready) return;
    for (Sound_* snd : liveSounds()) {
        UnloadSound(snd->sound);
        snd->loaded = false;
    }
    for (Music_* m : liveMusic()) {
        UnloadMusicStream(m->music);
        m->loaded = false;
    }
    liveSounds().clear();
    liveMusic().clear();
    CloseAudioDevice();
    s.ready = false;
}

inline std::shared_ptr<Sound_> loadSound_(const std::string& path) {
    std::string file = requireGameFile(path, "audio.loadSound()");
    auto s = std::make_shared<Sound_>();
    if (!ensureDevice()) return s;
    s->sound = LoadSound(file.c_str());
    if (s->sound.frameCount == 0) panic("audio.loadSound()", "couldn't read '" + path + "' as a sound (use .wav, .ogg, .mp3, .flac or .qoi)");
    s->loaded = true;
    liveSounds().push_back(s.get());
    return s;
}

inline std::shared_ptr<Music_> loadMusic_(const std::string& path) {
    std::string file = requireGameFile(path, "audio.loadMusic()");
    auto m = std::make_shared<Music_>();
    if (!ensureDevice()) return m;
    m->music = LoadMusicStream(file.c_str());
    if (m->music.frameCount == 0) panic("audio.loadMusic()", "couldn't read '" + path + "' as music (use .ogg, .mp3, .wav, .flac, .xm or .mod)");
    m->music.looping = true;
    m->loaded = true;
    liveMusic().push_back(m.get());
    return m;
}

inline void setVolume_(float v) {
    if (ensureDevice() && !std::getenv("FRG_AUTOTEST_HIDDEN")) SetMasterVolume(v);
}

// For games without a window: keeps music playing (the window does this by itself).
inline void update_() { updateMusic(); }

inline std::string frg$str(const std::shared_ptr<Sound_>& s) { return s ? "Sound" : "null"; }
inline std::string frg$str(const std::shared_ptr<Music_>& m) { return m ? "Music" : "null"; }

}  // namespace frg::audio
