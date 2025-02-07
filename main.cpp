#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>    // For sqrt() and atan2()
#include <cstdlib>  // For rand()

float windowx = 800;
float windowy = 600;

float playex = windowx / 2;
float playey = windowy / 2;
float playerspeed = 5;
bool debug = false;
int dt = 0;
bool sprinting = false;
int fpsdt = 0;
int firedt = 0;
bool firedttrigger = false;
int playerHealth = 100;
bool playerIsDead = false;


class Bullet {
public:
    sf::RectangleShape shape;
    float speed;
    bool isActive;
    sf::Vector2f direction;
    bool isEnemyBullet; // false = player's bullet, true = enemy's bullet

    Bullet(float x, float y, float speed, const sf::Vector2f& direction, bool enemyBullet = false)
        : speed(speed), isActive(true), direction(direction), isEnemyBullet(enemyBullet)
    {
        shape.setSize(sf::Vector2f(10, 5));
        if (enemyBullet)
            shape.setFillColor(sf::Color::Blue);
        else
            shape.setFillColor(sf::Color(50, 0, 50, 255));
        shape.setPosition(x, y);
    }

    void update() {
        shape.move(direction * speed); // Move the bullet based on direction and speed
        if (shape.getPosition().y < 0 || shape.getPosition().x < 0 ||
            shape.getPosition().x > windowx || shape.getPosition().y > windowy) {
            isActive = false; // Deactivate bullet if it's out of bounds
        }
    }
};

class Enemy {
public:
    sf::RectangleShape shape;
    float speed;
    sf::Vector2f direction;
    int health;      // Enemy health; when <= 0, enemy is "killed"
    int shootTimer;  // Timer for enemy shooting
    int fadeTimer;   // Alpha value for fade effect when dead

    Enemy(float x, float y, float speed, const sf::Vector2f& dir)
        : speed(speed), direction(dir), health(3), shootTimer(0), fadeTimer(255)
    {
        shape.setSize(sf::Vector2f(30, 30));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
    }

    // Update the enemy. If alive, move and shoot. If dead, fade out.
    void update(std::vector<Bullet>& bullets) {
        if (health > 0) {
            // Move enemy along its direction.
            shape.move(direction * speed);
            shootTimer++;
            // Enemy shoots every 120 frames.
            if (shootTimer >= 120) {
                // Create a bullet directed towards the player's current position.
                sf::Vector2f bulletDir(playex - shape.getPosition().x, playey - shape.getPosition().y);
                float len = std::sqrt(bulletDir.x * bulletDir.x + bulletDir.y * bulletDir.y);
                if (len != 0)
                    bulletDir /= len;
                bullets.push_back(Bullet(shape.getPosition().x, shape.getPosition().y, 4, bulletDir, true));
                shootTimer = 0;
            }
        } else {
            // Enemy is "killed": stop moving and begin fade out.
            if (fadeTimer > 0) {
                fadeTimer -= 5;
                if (fadeTimer < 0) fadeTimer = 0;
                sf::Color col = sf::Color::Red;
                col.a = fadeTimer;
                shape.setFillColor(col);
            }
        }
    }

    // Check if enemy is completely off screen.
    bool isOffScreen() {
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f size = shape.getSize();
        return (pos.x + size.x < 0 || pos.x > windowx || pos.y + size.y < 0 || pos.y > windowy);
    }

    // Determine if the enemy should be removed.
    bool shouldRemove() {
        // If dead and faded out, remove.
        if (health <= 0 && fadeTimer <= 0)
            return true;
        // If still alive but completely off screen, remove.
        if (health > 0 && isOffScreen())
            return true;
        return false;
    }
};

int main()
{
    sf::RenderWindow app(sf::VideoMode(windowx, windowy), "The Legend Of Azure");
    app.setFramerateLimit(60);

    sf::Text glitchText;
    sf::Font errorfont;
    if (!errorfont.loadFromFile("assets/Wingdings-Regular.ttf")) {
        return EXIT_FAILURE;
    }
    glitchText.setFont(errorfont);
    glitchText.setCharacterSize(24);
    glitchText.setFillColor(sf::Color::Red);

    sf::Text fpstext;
    sf::Font mainfont;
    if (!mainfont.loadFromFile("assets/comicsans_ms.ttf")) {
        return EXIT_FAILURE;
    }
    fpstext.setFont(mainfont);
    fpstext.setCharacterSize(24);
    fpstext.setFillColor(sf::Color::Black);

    sf::Text HealthText;
    HealthText.setFont(mainfont);
    HealthText.setCharacterSize(24);
    HealthText.setFillColor(sf::Color(115, 0, 0, 255));

    sf::Image icon;
    if (icon.loadFromFile("assets/Icon.png")) {
        app.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    sf::Texture texture;
    if (!texture.loadFromFile("assets/Player.png")) {
        return EXIT_FAILURE;
    }
    sf::Sprite sprite(texture);
    sprite.scale(0.25, 0.25); // Init
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
    sprite.setPosition(playex, playey);

    // Clock for FPS
    sf::Clock fpsclock;

    // Vectors for bullets and enemies.
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    sf::Clock enemySpawnClock;

    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();
        }

        // Player movement logic (your original logic preserved)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            if (sprinting) {
                playex -= playerspeed + 4;
            } else {
                playex -= playerspeed;
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (sprinting) {
                playex += playerspeed + 4;
            } else {
                playex += playerspeed;
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (sprinting) {
                playey -= playerspeed + 4;
            } else {
                playey -= playerspeed;
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            if (sprinting) {
                playey += playerspeed + 4;
            } else {
                playey += playerspeed;
            }
        }

        // Sprinting logic (your original logic preserved)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            sprinting = true;
        } else {
            sprinting = false;
        }

        // Debug key (keeping your debug key in)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tilde)) {
            if (!debug)
                std::cout << "Entering debug." << std::endl;
            debug = true;
        }

        // Keep player within bounds.
        if (playex < 12) playex = 12;
        if (playey < 27) playey = 27;
        if (playex > windowx - 10) playex = windowx - 10;
        if (playey > windowy - 20) playey = windowy - 20;

        // Shoot bullets (with direction towards mouse)
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !firedttrigger) {
            firedttrigger = true;
            firedt = 0;
            sf::Vector2i mousePos = sf::Mouse::getPosition(app); // Get mouse position relative to window
            sf::Vector2f direction = sf::Vector2f(mousePos.x - playex, mousePos.y - playey);
            // Normalize direction vector
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length != 0) {
                direction /= length;
            }
            bullets.push_back(Bullet(playex, playey, 5, direction)); // Player bullet (purple-ish)
        }
        firedt = (firedt + 1) % 60;
        if (firedt == 0)
            firedttrigger = false;

        // Update all bullets.
        for (auto& bullet : bullets) {
            bullet.update();
        }

        // --- Enemy Logic --- //
        // Spawn enemy every 0.7777... that number down there ↓ second(s).
        if (enemySpawnClock.getElapsedTime().asSeconds() > 0.77777) {
            // Spawn enemy at a random X position at the top (slightly above the screen) with a downward direction.
            float enemyX = static_cast<float>(rand() % (static_cast<int>(windowx) - 30));
            sf::Vector2f enemyDir(0, 1);
            enemies.push_back(Enemy(enemyX, -30, 2, enemyDir));
            enemySpawnClock.restart();
        }

        // Update enemies (and let them shoot).
        for (auto& enemy : enemies) {
            enemy.update(bullets);
        }

        // --- Collision Detection --- //
        // Player bullets vs Enemies.
        for (auto& bullet : bullets) {
            if (!bullet.isEnemyBullet && bullet.isActive) {
                sf::FloatRect bBounds = bullet.shape.getGlobalBounds();
                for (auto& enemy : enemies) {
                    if (enemy.health > 0) { // Only if enemy is alive.
                        sf::FloatRect eBounds = enemy.shape.getGlobalBounds();
                        if (bBounds.intersects(eBounds)) {
                            bullet.isActive = false;
                            enemy.health--;  // Damage enemy.
                            // When enemy is killed, its update() will trigger the fade-out effect.
                        }
                    }
                }
            }
        }

        // Enemy bullets vs Player.
        for (auto& bullet : bullets) {
            if (bullet.isEnemyBullet && bullet.isActive) {
                sf::FloatRect bBounds = bullet.shape.getGlobalBounds();
                sf::FloatRect playerBounds = sprite.getGlobalBounds();
                if (bBounds.intersects(playerBounds)) {
                    bullet.isActive = false;
                    playerHealth -= 10; // Damage from enemy bullet.
                }
            }
        }

        // Enemies vs Player.
        for (auto& enemy : enemies) {
            sf::FloatRect eBounds = enemy.shape.getGlobalBounds();
            sf::FloatRect playerBounds = sprite.getGlobalBounds();
            if (eBounds.intersects(playerBounds) && enemy.health > 0) {
                playerHealth -= 10;  // Damage the player.
                enemy.health = 0;    // Enemy is "killed" and begins fading out.
            }
        }

        // Remove inactive bullets.
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.isActive; }), bullets.end());

        // Remove enemies that should be removed.
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [](Enemy& e) { return e.shouldRemove(); }), enemies.end());

        // Check if player is dead.
        if (playerHealth <= 0) {
            std::cout << "Player died!" << std::endl;
            glitchText.setString("You Died...");
            playerIsDead = true;
        }
        else playerIsDead = false;

        // Clear screen.
        app.clear(sf::Color(100, 100, 100, 255));

        // FPS counter logic.
        fpsdt++;
        float elapsedtime = fpsclock.restart().asSeconds();
        float fps = 1.0f / elapsedtime;
        if (fpsdt == 10) {
            fpsdt = 0;
            fpstext.setString("FPS: " + std::to_string(static_cast<int>(fps)) + " | Health: " + std::to_string(playerHealth));
        }

        HealthText.setString("Health: " + std::to_string(playerHealth));

        // Debug info (keeping your logic for debug output).
        dt++;
        if (dt == 60 && debug) {
            dt = 0;
            std::cout << "Player X: " << sprite.getPosition().x << " | Y: " << sprite.getPosition().y << std::endl;
            std::cout << "firedt: " << firedt << std::endl;
            std::cout << "Player Health: " << playerHealth << std::endl;
        }
        if (dt == 60) {
            dt = 0;
        }

        if (debug) {
            app.draw(fpstext);
        }

        // Set sprite position before we draw it.
        sprite.setPosition(playex, playey);

        // Draw bullets.
        for (auto& bullet : bullets) {
            app.draw(bullet.shape);
        }

        // Draw enemies.
        for (auto& enemy : enemies) {
            app.draw(enemy.shape);
        }

        // Draw the player sprite.
        app.draw(sprite);

        app.draw(HealthText);
        // Death Text.
        if (playerIsDead) {
            app.clear(sf::Color::Black);
            glitchText.setPosition(windowx/2 - 120, windowy/2);
            app.draw(glitchText);
        }
        // Update the window.
        app.display();
        if (playerIsDead) {
            sf::sleep(sf::seconds(5));
            app.close();
        }
    }

    return EXIT_SUCCESS;
}
