/*
 * midistar
 * Copyright (C) 2018 Jeremy Collette.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>

#include "midistar/PianoGameObjectFactory.h"

#include "midistar/CollidableComponent.h"
#include "midistar/CollisionDetectorComponent.h"
#include "midistar/Config.h"
#include "midistar/DeleteOffscreenComponent.h"
#include "midistar/Game.h"
#include "midistar/GraphicsComponent.h"
#include "midistar/InstrumentCollisionHandlerComponent.h"
#include "midistar/InstrumentComponent.h"
#include "midistar/InstrumentInputHandlerComponent.h"
#include "midistar/InvertColourComponent.h"
#include "midistar/NoteInfoComponent.h"
#include "midistar/PhysicsComponent.h"
#include "midistar/ResizeComponent.h"
#include "midistar/SongNoteCollisionHandlerComponent.h"
#include "midistar/SongNoteComponent.h"
#include "midistar/Utility.h"

namespace midistar {

const sf::Color PianoGameObjectFactory::MIDI_TRACK_COLOURS[NUM_TRACK_COLOURS] {
    sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow
    , sf::Color::Magenta, sf::Color::Cyan
};

const bool PianoGameObjectFactory::OCTAVE_BLACK_KEYS[NOTES_PER_OCTAVE] {
    0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1
};

const char PianoGameObjectFactory::OCTAVE_KEY_TO_WHITE_KEY[NOTES_PER_OCTAVE] {
    0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6
};

PianoGameObjectFactory::PianoGameObjectFactory(double note_speed)
        : GameObjectFactory{note_speed}
        , white_width_{Config::GetInstance().GetScreenWidth() /
            static_cast<double>(NUM_WHITE_KEYS)} {
}

std::vector<GameObject*> PianoGameObjectFactory::CreateInstrument() {
    std::vector<GameObject*> result;
    for (int key = PIANO_FIRST_MIDI_KEY; key < PIANO_FIRST_MIDI_KEY +
            NUM_PIANO_KEYS; ++key) {
        GameObject* note = CreateInstrumentNote(key);
        // We put black keys after white keys so they are displayed on top
        result.insert(IsBlackKey(key) ? result.end() : result.begin(), note);
    }
    return result;
}

GameObject* PianoGameObjectFactory::CreateSongNote(
        int track
        , int chan
        , int note
        , int vel
        , double duration) {
    // Create actual note
    double x = CalculateXPosition(note);
    double height = duration * 1000 * GetNoteSpeed();
    GameObject* song_note = new GameObject{x, -height};

    // White notes VS black notes (sharp notes) are slightly different
    double width;
    sf::Color colour = GetTrackColour(track);
    if (IsBlackKey(note)) {
        // Darken sharp notes and change width
        colour = DarkenColour(colour);
        width = white_width_ * BLACK_WIDTH_MULTIPLIER;
    } else {
        width = white_width_;
    }

    // Create underlying rectangle
    sf::RectangleShape* rect = new sf::RectangleShape{{static_cast<float>(
            width), static_cast<float>(height)}};
    rect->setPosition({static_cast<float>(x), -static_cast<float>(height)});
    rect->setFillColor(colour);
    rect->setOutlineThickness(NOTE_OUTLINE_THICKNESS);
    rect->setOutlineColor(sf::Color::Black);

    // Add components
    song_note->SetComponent(new SongNoteComponent{});
    song_note->SetComponent(new CollidableComponent{});
    song_note->SetComponent(new NoteInfoComponent{track, chan, note, vel});
    song_note->SetComponent(new GraphicsComponent{rect});
    song_note->SetComponent(new PhysicsComponent{0, GetNoteSpeed()});
    song_note->SetComponent(new DeleteOffscreenComponent{});
    song_note->SetComponent(new CollisionDetectorComponent{});
    song_note->SetComponent(new SongNoteCollisionHandlerComponent{});
    return song_note;
}

sf::Color PianoGameObjectFactory::DarkenColour(sf::Color c) {
    c.r *= COLOUR_DARKEN_MULTIPLIER;
    c.g *= COLOUR_DARKEN_MULTIPLIER;
    c.b *= COLOUR_DARKEN_MULTIPLIER;
    return c;
}

sf::Color PianoGameObjectFactory::GetTrackColour(int midi_track) {
    return MIDI_TRACK_COLOURS[midi_track % NUM_TRACK_COLOURS];
}

void PianoGameObjectFactory::GetInstrumentKeyBinding(
        int midi_key
        , sf::Keyboard::Key* key
        , bool* ctrl
        , bool* shift) {
    const std::vector<sf::Keyboard::Key>& keys = Utility::GetQwertyKeys();
    int num_keys = keys.size();

    // We bind each white piano key to its own computer keyboard key. Black
    // keys are played by pressing the key binding for the white key plus the
    // shift modifier.
    assert(num_keys >= NUM_WHITE_KEYS);

    int white_index = GetWhiteKeyIndex(midi_key);
    *key = keys[white_index];
    if (IsBlackKey(midi_key)) {
        *shift = true;
    }
    *ctrl = false;
}

int PianoGameObjectFactory::GetWhiteKeyIndex(int midi_key) {
    // Here we find the white key index for any given MIDI key. MIDI keys that
    // are assigned to a black key will return the previous white key.
    int key_index = midi_key - PIANO_FIRST_MIDI_KEY;
    int octave_num = key_index / NOTES_PER_OCTAVE;
    int prev_white_keys = octave_num * WHITE_KEYS_PER_OCTAVE;
    int octave_index = key_index % NOTES_PER_OCTAVE;
    int result =  prev_white_keys + OCTAVE_KEY_TO_WHITE_KEY[octave_index];
    assert(result >= 0 && result < NUM_WHITE_KEYS);
    return result;
}

bool PianoGameObjectFactory::IsBlackKey(int midi_key) {
    // The OCTAVE_BLACK_KEYS array indicates black keys in a typical octave.
    // To find if a specific key is a black key, we subtract the first piano
    // MIDI key (the MIDI key of the first key on a piano), and find the
    // remainder if we divide by NOTES_PER_OCTAVE.
    //
    // This gives us a value between 0 and NOTES_PER_OCTAVE, that we can use
    // as an index in the OCTAVE_BALCK_KEYS array. We return the value at this
    // index.
    return OCTAVE_BLACK_KEYS[(midi_key - PIANO_FIRST_MIDI_KEY)
        % NOTES_PER_OCTAVE];
}

double PianoGameObjectFactory::CalculateXPosition(int midi_key) {
    double white_key_index = GetWhiteKeyIndex(midi_key);
    double x = white_key_index * white_width_;
    if (IsBlackKey(midi_key)) {
        double black_width = white_width_ * BLACK_WIDTH_MULTIPLIER;
        // Move the black key/note so that it is in between two white keys
        x += white_width_ / 2.0  + black_width / 2.0;
    }
    return x;
}

GameObject* PianoGameObjectFactory::CreateInstrumentNote(int note) {
    bool is_black = IsBlackKey(note);

    // Black and white keys have slightly different atributes
    double height, width, outline_thickness;
    sf::Color colour;
    if (is_black) {
        height = BLACK_KEY_HEIGHT;
        colour = sf::Color::Black;
        width = white_width_ * BLACK_WIDTH_MULTIPLIER;
        outline_thickness = WHITE_KEY_OUTLINE_THICKNESS;
    } else {
        height = WHITE_KEY_HEIGHT;
        width = white_width_;
        colour = sf::Color::White;
        outline_thickness = BLACK_KEY_OUTLINE_THICKNESS;
    }

    // Create the actual note
    double x = CalculateXPosition(note);
    double y = Config::GetInstance().GetScreenHeight() - WHITE_KEY_HEIGHT -
        (Config::GetInstance().GetScreenHeight() * KEY_HOVER_PERCENTAGE);
    GameObject* ins_note = new GameObject{x, y};

    // Create the underlying shape
    sf::RectangleShape* rect = new sf::RectangleShape{{static_cast<float>(
            width), static_cast<float>(height)}};
    rect->setPosition({static_cast<float>(x), static_cast<float>(y)});
    rect->setFillColor(colour);
    rect->setOutlineColor(sf::Color::Black);
    rect->setOutlineThickness(outline_thickness);

    // Get the instrument key binding
    sf::Keyboard::Key key;
    bool ctrl, shift;
    GetInstrumentKeyBinding(note, &key, &ctrl, &shift);

    // Add components
    ins_note->SetComponent(new InstrumentComponent{});
    ins_note->SetComponent(new NoteInfoComponent{-1, 0, note
            , Config::GetInstance().GetMidiOutVelocity()});
    ins_note->SetComponent(new GraphicsComponent{rect});
    ins_note->SetComponent(new InstrumentInputHandlerComponent{key, ctrl,
            shift});
    ins_note->SetComponent(new CollisionDetectorComponent{});
    ins_note->SetComponent(new InstrumentCollisionHandlerComponent{});
    return ins_note;
}

}  // End namespace midistar