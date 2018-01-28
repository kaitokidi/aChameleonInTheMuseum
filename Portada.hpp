#ifndef PORTADA_H
#define PORTADA_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class Portada {

private:
    sf::Sprite s;
    sf::Texture t;
    bool open;
    bool wantAnimation;
    
public:
    Portada();
    ~Portada();
    void display(sf::RenderWindow* window);
    void display(sf::RenderWindow* window, std::string pathImage);
    void notAnimation();
    void animation();
    
};

#endif // PORTADA_H