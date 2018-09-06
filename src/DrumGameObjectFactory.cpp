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

#include "midistar/DrumGameObjectFactory.h"

#include <cassert>

#include "midistar/CollidableComponent.h"
#include "midistar/Config.h"
#include "midistar/DeleteOffscreenComponent.h"
#include "midistar/Game.h"
#include "midistar/InstrumentCollisionHandlerComponent.h"
#include "midistar/InstrumentComponent.h"
#include "midistar/InstrumentInputHandlerComponent.h"
#include "midistar/NoteInfoComponent.h"
#include "midistar/PhysicsComponent.h"
#include "midistar/ResizeComponent.h"
#include "midistar/SongNoteCollisionHandlerComponent.h"
#include "midistar/SongNoteComponent.h"
#include "midistar/Utility.h"
#include "midistar/VerticalCollisionDetectorComponent.h"

namespace midistar {

const sf::Color DrumGameObjectFactory::BACKGROUND_COLOUR{0, 0, 0};

DrumGameObjectFactory::DrumGameObjectFactory(
    double note_speed
    , const std::vector<int>& song_notes)
        : GameObjectFactory{note_speed, BACKGROUND_COLOUR}
        , note_width_{Config::GetInstance().GetScreenWidth() /
            static_cast<double>(song_notes.size())}
        , song_notes_{song_notes} {
}

GameObject* DrumGameObjectFactory::CreateNotePlayEffect(GameObject*) {
    // NOTE: This feature is not implemented for the DrumGameObjectFactory.
    auto* rect = new sf::RectangleShape{};
    return new GameObject{rect, 0, 0, 0, 0};
}

std::vector<GameObject*> DrumGameObjectFactory::CreateInstrument() {
    std::vector<GameObject*> result;
    for (int key : song_notes_) {
        result.push_back(CreateInstrumentNote(key));
    }
    return result;
}

GameObject* DrumGameObjectFactory::CreateSongNote(
        int track
        , int chan
        , int note
        , int vel
        , double duration) {
    // Create underlying shape
    double x = GetXPosition(note);
    double height = duration * 1000 * GetNoteSpeed();
    sf::RectangleShape* rect = new sf::RectangleShape{{static_cast<float>(
            note_width_), static_cast<float>(height)}};

    // Create GameObject
    // Height is derived by note duration and speed (note should move its
    // entire height over its duration).
    auto song_note = new GameObject{rect, x, -height, note_width_, height};

    // Add components
    song_note->SetComponent(new SongNoteComponent{});
    song_note->SetComponent(new CollidableComponent{});
    song_note->SetComponent(new NoteInfoComponent{track, chan, note, vel});
    song_note->SetComponent(new PhysicsComponent{0, GetNoteSpeed()});
    song_note->SetComponent(new DeleteOffscreenComponent{});
    song_note->SetComponent(new VerticalCollisionDetectorComponent{});
    song_note->SetComponent(new SongNoteCollisionHandlerComponent{});
    return song_note;
}

GameObject* DrumGameObjectFactory::CreateInstrumentNote(int note) {
    // Create underlying shape
    double x = GetXPosition(note);
    double y = Config::GetInstance().GetScreenHeight() - INSTRUMENT_HEIGHT -
        (Config::GetInstance().GetScreenHeight() * INSTRUMENT_HOVER_PERCENTAGE);
    sf::RectangleShape* rect = new sf::RectangleShape{{static_cast<float>(
            note_width_), INSTRUMENT_HEIGHT}};;
    rect->setFillColor(sf::Color::Red);

    // Create GameObject
    auto ins_note = new GameObject{rect, x, y, note_width_, INSTRUMENT_HEIGHT};

    // Get key binding
    sf::Keyboard::Key key;
    bool ctrl, shift;
    GetInstrumentKeyBinding(note, &key, &ctrl, &shift);

    // Add components
    ins_note->SetComponent(new InstrumentComponent{});
    ins_note->SetComponent(new NoteInfoComponent{-1, 9, note
            , Config::GetInstance().GetMidiOutVelocity()});
    ins_note->SetComponent(new InstrumentInputHandlerComponent{key, ctrl
            , shift});
    ins_note->SetComponent(new VerticalCollisionDetectorComponent{});
    ins_note->SetComponent(new InstrumentCollisionHandlerComponent{});
    return ins_note;
}

void DrumGameObjectFactory::GetInstrumentKeyBinding(
        int midi_key
        , sf::Keyboard::Key* key
        , bool* ctrl
        , bool* shift) {
    const std::vector<sf::Keyboard::Key>& keys = Utility::GetQwertyKeys();
    assert(keys.size() >= song_notes_.size());

    // Get the key
    int idx = GetNoteUniqueIndex(midi_key);
    *key = keys[idx];
    *ctrl = false;
    *shift = false;
}

int DrumGameObjectFactory::GetNoteUniqueIndex(int note) {
    auto pos = std::find(song_notes_.begin(), song_notes_.end(), note);
    assert(pos != song_notes_.end());
    return pos - song_notes_.begin();
}

double DrumGameObjectFactory::GetXPosition(int note) {
    return GetNoteUniqueIndex(note) * note_width_;
}

bool DrumGameObjectFactory::Init() {
    // We have nothing to initialise...
    return true;
}

}  // End namespace midistar