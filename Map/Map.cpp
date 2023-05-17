#include "Map.h"
#include "../../../Resources/Resources.h"

Map::Map(std::istream& file) {
    LoadSize(file);
    LoadResources();
    LoadField(file);
}

void Map::LoadSize(std::istream& in_file) {
    in_file >> field_size_.x;
    in_file >> field_size_.y;

    tile_size_.x = window_size_.y / static_cast<float>(field_size_.y);
    tile_size_.y = tile_size_.x;

    visible_tiles_in_row_ = static_cast<uint32_t>(window_size_.x / tile_size_.x) + 1;

    background_tile_size_.x = window_size_.x;
    background_tile_size_.y = background_tile_size_.x *
        static_cast<float>(Config::Graphics::BACKGROUND_TEXTURE_SIZE.y) /
        static_cast<float>(Config::Graphics::BACKGROUND_TEXTURE_SIZE.x);
}

void Map::LoadField(std::istream& in_file) {
    for (uint32_t x = 0; x < field_size_.x; x++) {
        std::vector<BlockType> row(field_size_.y);
        for (uint32_t y = 0; y < field_size_.y; y++) {
            char cell;
            in_file >> cell;

            if (cell == '0') {
                row[field_size_.y - 1 - y] = BlockType::None;
            } else if (cell == '1') {
                row[field_size_.y - 1 - y] = BlockType::Block;
            } else if (cell == '2') {
                row[field_size_.y - 1 - y] = BlockType::Spike;
            }
        }
        field_.push_back(row);
    }

    for (int i = 0; i < visible_tiles_in_row_; i++) {
        field_.push_back(field_[i]);
    }
}

void Map::LoadResources() {
    block_sprite_.setTexture(Resources::BLOCK_TEXTURE());
    float block_scale = tile_size_.x / block_sprite_.getGlobalBounds().width;
    block_sprite_.setScale(block_scale, block_scale);

    spike_sprite_.setTexture(Resources::SPIKE_TEXTURE());
    float spike_scale = tile_size_.x / spike_sprite_.getGlobalBounds().width * Config::Graphics::SPIKE_SCALE;
    float spike_padding = -spike_sprite_.getLocalBounds().width * (1 - Config::Graphics::SPIKE_SCALE) / 2;
    spike_sprite_.setOrigin(spike_padding, spike_padding);
    spike_sprite_.setScale(spike_scale, spike_scale);

    background_sprite_.setTexture(Resources::BACKGROUND_TEXTURE());
    float background_scale = background_tile_size_.x / background_sprite_.getGlobalBounds().width;
    background_sprite_.setScale(background_scale, background_scale);
}

void Map::OnFrame(const Timer& timer) {
    // Изменяем сдвиг поля
    shift_ += timer.GetDelta() * shift_changing_speed_;

    if (shift_ > static_cast<float>(field_size_.x)) {
        shift_changing_speed_ *= 1.1;
        shift_ -= static_cast<float>(field_size_.x);
    }
}

void Map::OnDraw(sf::RenderWindow& window) {
    auto first_visible = static_cast<uint32_t>(shift_);

    window.draw(background_sprite_);

    // Отрисовываем каждый блок
    for (uint32_t x = first_visible; x <= first_visible + visible_tiles_in_row_; x++) {
        for (uint32_t y = 0; y < field_size_.y; y++) {
            if (field_[x][y] == BlockType::None)
                continue;

            sf::Sprite& tile_sprite = GetSpriteByType(field_[x][y]);

            sf::FloatRect float_rect = GetTileRect(x, y);
            tile_sprite.setPosition(float_rect.left, float_rect.top);

            window.draw(tile_sprite);
        }
    }
}

sf::FloatRect Map::GetTileRect(uint32_t x, uint32_t y) const {
    return {
        (static_cast<float>(x) - shift_) * tile_size_.x,
        static_cast<float >(y) * tile_size_.y,
        tile_size_.x, tile_size_.y
    };
}

void Map::OnEvent(sf::Event& event, const Timer& timer) {

}

sf::Sprite& Map::GetSpriteByType(BlockType block_type) {
    if (block_type == BlockType::Spike) {
        return spike_sprite_;
    } else if (block_type == BlockType::Block) {
        return block_sprite_;
    }
    return block_sprite_;
}

CollisionType Map::CheckForCollision(sf::FloatRect hitbox, Direction direction) const {
    SqueezeHitbox(hitbox, direction);

    // Если выходит за границы, безопасная коллизия
    if (hitbox.top < 0)
        return CollisionType::Safe;
    if (hitbox.top + hitbox.height >= window_size_.y)
        return CollisionType::Safe;

    bool was_collision = false;

    // Перебором всех блоков ищем коллизию
    auto first_visible = static_cast<uint32_t>(shift_);
    for (uint32_t x = first_visible; x <= first_visible + visible_tiles_in_row_; x++) {
        for (uint32_t y = 0; y < field_size_.y; y++) {
            if (field_[x][y] == BlockType::None)
                continue;
            if (hitbox.intersects(GetTileRect(x, y))) {
                was_collision = true;
                if (field_[x][y] == BlockType::Spike || direction == Direction::Right)
                    return CollisionType::Unsafe;
            }
        }
    }

    if (was_collision)
        return CollisionType::Safe;
    return CollisionType::None;
}
float Map::GetTileSize() const {
    return tile_size_.x;
}
float Map::NearestUp(const sf::FloatRect& hitbox) const {
    float nearest_up = -tile_size_.y;

    auto first_visible = static_cast<uint32_t>(shift_);
    for (uint32_t x = first_visible; x <= first_visible + visible_tiles_in_row_; x++) {
        for (uint32_t y = 0; y < field_size_.y; y++) {
            if (field_[x][y] == BlockType::None)
                continue;

            sf::FloatRect tile_rect = GetTileRect(x, y);
            if (tile_rect.top < hitbox.top && hitbox.intersects(tile_rect)) {
                nearest_up = GetTileRect(x, y).top;
            }
        }
    }

    return nearest_up + tile_size_.y;
}

float Map::NearestDown(const sf::FloatRect& hitbox) const {
    float nearest_down = window_size_.y;

    auto first_visible = static_cast<uint32_t>(shift_);
    for (uint32_t x = first_visible; x <= first_visible + visible_tiles_in_row_; x++) {
        for (uint32_t y = 0; y < field_size_.y; y++) {
            if (field_[x][y] == BlockType::None)
                continue;

            sf::FloatRect tile_rect = GetTileRect(x, y);
            if (tile_rect.top > hitbox.top && hitbox.intersects(tile_rect)) {
                nearest_down = GetTileRect(x, y).top;
            }
        }
    }

    return nearest_down - tile_size_.y;
}
void Map::SqueezeHitbox(sf::FloatRect& hitbox, Direction direction) {
    float length = Config::Physics::Hitbox::END_LENGTH_PERCENTS;
    float thickness = Config::Physics::Hitbox::END_THICKNESS_PERCENTS;

    // Сжатие по длине
    if (direction == Direction::Up || direction == Direction::Down) {
        hitbox.left += hitbox.width * (1 - length) / 2;
        hitbox.width *= length;
    } else {
        hitbox.top += hitbox.height * (1 - length) / 2;
        hitbox.height *= length;
    }

    // Сжатие по ширине
    if (direction == Direction::Up) {
        hitbox.height *= thickness;
    } else if (direction == Direction::Down) {
        hitbox.top += hitbox.height * (1 - thickness);
        hitbox.height *= thickness;
    } else if (direction == Direction::Right) {
        hitbox.left += hitbox.width * (1 - thickness);
        hitbox.width *= thickness;
    } else if (direction == Direction::Left) {
        hitbox.width *= thickness;
    }
}


