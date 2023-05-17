#pragma once

#include "../../../Scene/Scene.h"
#include "../../../Config/Config.h"

#include <deque>

// Типы блоков
enum class BlockType {
    None, Spike, Block
};

// Типы коллизий
enum class CollisionType {
    None, Safe, Unsafe
};

// Типы направлений
enum class Direction {
    Up, Down, Left, Right
};

class Map : public Scene {
public:
    ~Map() override = default;

    explicit Map(std::istream& file);
    void OnFrame(const Timer& timer) override;
    void OnDraw(sf::RenderWindow& window) override;
    void OnEvent(sf::Event& event, const Timer& timer) override;

    // Проверка столкновений
    CollisionType CheckForCollision(sf::FloatRect hitbox, Direction direction) const;

    // Функции для получения позиции ближайшего блока сверху и снизу
    float NearestUp(const sf::FloatRect& hitbox) const;
    float NearestDown(const sf::FloatRect& hitbox) const;

    float GetTileSize() const;
private:
    void LoadSize(std::istream& in_file);
    void LoadField(std::istream& in_file);
    void LoadResources();

    // Поле и данные для его отрисовки
    std::vector<std::vector<BlockType>> field_;
    sf::Vector2u field_size_;
    sf::Vector2f tile_size_;

    sf::Vector2f background_tile_size_;

    uint32_t visible_tiles_in_row_{};

    sf::Sprite block_sprite_;
    sf::Sprite spike_sprite_;
    sf::Sprite background_sprite_;
    sf::Sprite& GetSpriteByType(BlockType block_type);
    sf::FloatRect GetTileRect(uint32_t x, uint32_t y) const;

    // Сжимает хитбокс, чтобы из прямоугольника он стал палкой, это предотвратит баги связанные с точностью float
    static void SqueezeHitbox(sf::FloatRect& hitbox, Direction direction) ;

    float shift_ = 0;
    float shift_changing_speed_ = Config::Physics::MAP_SHIFT_CHANGING_SPEED;
};
