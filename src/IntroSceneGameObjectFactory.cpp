/*
 * midistar
 * Copyright (C) 2018-2019 Jeremy Collette.
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

#include "midistar/IntroSceneGameObjectFactory.h"

#include "midistar/Game.h"
#include "midistar/MenuComponent.h"
#include "midistar/MenuInputHandlerComponent.h"
#include "midistar/MenuItemComponent.h"
#include "midistar/SongNoteComponent.h"
#include "midistar/Version.h"

namespace midistar {

// TODO(@jeremy): this should be a scene factory
IntroSceneGameObjectFactory::IntroSceneGameObjectFactory() {
}

std::vector<GameObject*> IntroSceneGameObjectFactory::CreateGameObjects() {
	// TODO(@jeremy): cleanup heap objects
	// TODO(@jeremy): add font to project
	// TODO(@jeremy): resize based on screen size

	// Create menu
	auto font = new sf::Font();
	if (!font->loadFromFile("PixelMiners-KKal.otf")) {
		throw "Could not load font!";
	}

	auto menu_title = new sf::Text("midistar", *font, 100);
	menu_title->setFillColor(sf::Color::Green);
	auto menu = new GameObject{ menu_title, 0, 0, 0, 0 };
	menu->SetComponent(new MenuComponent{ });
	menu->SetComponent(new MenuInputHandlerComponent{ });

	// Add menu items
	auto menu_item_text = std::vector<std::string*>{
		new std::string{ "1. Piano" },
		new std::string{ "2. Drum" },
		new std::string{ "0. Exit" },
	};

    auto on_select_lambdas = std::vector<std::function<void(Game*, GameObject*, int)>>{
        [](Game* g, GameObject*, int) {
            g->SetScene("Piano");
        }, [](Game* g, GameObject*, int) {
            g->SetScene("Drum");
        }, [](Game* g, GameObject*, int) {
            g->SetScene("Exit");
        }
    };


    for (auto i = 0u; i < menu_item_text.size(); ++i) {
		auto drawable = new sf::Text(*menu_item_text[i], *font, 50);
		auto menu_item = new GameObject(drawable, 50, 150 + 50 * i, 20, 20);
		menu_item->SetComponent(new MenuItemComponent{ *menu_item_text[i]
            , on_select_lambdas[i] });
		menu->AddChild(menu_item);
	}

	auto copyright_string = new std::string{
		"Copyright (c) Jeremy Collette 2018-2020" };
	auto copyright_text = new sf::Text(*copyright_string, *font, 25);
	copyright_text->setFillColor(sf::Color::White);
	auto copyright = new GameObject{ copyright_text, 150, 650, 0, 0 };
	// TODO(@jeremy): revist empty GameObjects being deleted. Maybe we can
	// create a "KeepAliveComponent"?
	copyright->SetComponent(new SongNoteComponent{ });

    auto version_string = new std::string{ MIDISTAR_VERSION };
    auto version_text = new sf::Text(*version_string, *font, 25);
    auto version = new GameObject{ version_text, 900, 720, 0, 0 };
    // TODO(@jeremy): revist empty GameObjects being deleted. Maybe we can
    // create a "KeepAliveComponent"?
    version->SetComponent(new SongNoteComponent{ });

	return std::vector<GameObject*> { menu, copyright, version };
}

}  // End namespace midistar
