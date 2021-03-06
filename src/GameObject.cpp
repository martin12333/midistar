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

#include "midistar/GameObject.h"

namespace midistar {

GameObject::~GameObject() {
    for (auto c : components_) {
        delete c;
    }
    delete drawable_;
}

void GameObject::DeleteComponent(ComponentType type) {
    if (!components_[type]) {
        return;
    }
    to_delete_.push_back(components_[type]);
    components_[type] = nullptr;
}

void GameObject::Draw(sf::RenderWindow* window) {
    window->draw(*drawable_);
}

void GameObject::GetPosition(double* x, double* y) {
    auto pos = transformable_->getPosition();
    *x = pos.x;
    *y = pos.y;
}

void GameObject::GetSize(double* w, double* h) {
    // We use RectangleShape::setSize() in the SetSize() method, so we have to
    // use RectangleShape::getSize() here.
    auto* rect = GetDrawformable<sf::RectangleShape>();
    if (rect) {
        auto size = rect->getSize();
        *w = size.x;
        *h = size.y;
    } else {
        auto scale = transformable_->getScale();
        *w = original_width_ * scale.x;
        *h = original_height_ * scale.y;
    }
}

bool GameObject::GetRequestDelete() {
    return request_delete_;
}

bool GameObject::HasComponent(ComponentType type) {
    return components_[type];
}

void GameObject::SetComponent(Component* c) {
    DeleteComponent(c->GetType());
    components_[c->GetType()] = c;
}

void GameObject::SetPosition(double x, double y) {
    transformable_->setPosition(static_cast<float>(x), static_cast<float>(y));
}

void GameObject::SetRequestDelete(bool del) {
    request_delete_ = del;
}

void GameObject::SetSize(double w, double h) {
    // We use RectangleShape::setSize to stop the outline being stretched.
    auto* rect = GetDrawformable<sf::RectangleShape>();
    if (rect) {
        rect->setSize({static_cast<float>(w), static_cast<float>(h)});
    } else {
        transformable_->setScale(static_cast<float>(w / original_width_)
            , static_cast<float>(h / original_height_));
    }
}

void GameObject::Update(Game* g, int delta) {
    auto has_component = false;
    for (const auto& c : components_) {
        if (c) {
            c->Update(g, this, delta);
            has_component = true;
        }
    }

    // If we don't have any components, delete the GameObject
    if (!has_component) {
        SetRequestDelete(true);
    }

    for (const auto& c : to_delete_) {
        delete c;
    }
    to_delete_.clear();
}

}   // End namespace midistar
