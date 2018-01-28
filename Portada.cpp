#include "Portada.hpp"


    Portada::Portada() {
        open = true;
    }
    
    Portada::~Portada(){}
    
    void Portada::notAnimation(){
        wantAnimation = false;
    }
    void Portada::animation(){
        wantAnimation = true;
    }
    
    void Portada::display(sf::RenderWindow* window, std::string pathImage){

        bool introDone = false;


        open = true;
            t.loadFromFile(pathImage);
            s = sf::Sprite();
            s.setTexture(t);

            sf::Event event;

            while(open){
            
                sf::Event event;
                while (window->pollEvent(event)) {
                    switch (event.type) {
                    case sf::Event::Closed:
                        window->close();
                        break;
                    case sf::Event::KeyPressed:
                        if (event.key.code == sf::Keyboard::Escape) window->close();
                        else open = false;
                        break;
                    case sf::Event::MouseButtonPressed:
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            open = false;
                        }
                        break;
                    default:
                        break;
                    }
                    //ANDROID
                         if(event.type == sf::Event::TouchEnded)  open = false;
                        break;
                }


                sf::View view;
                sf::Vector2i desiredSize(s.getGlobalBounds().width, s.getGlobalBounds().height);
                int windowX = window->getSize().x; int windowY = window->getSize().y;
                float xr = windowX / float(desiredSize.x);
                float yr = windowY / float(desiredSize.y);
                float aux;
                if (xr < yr) aux = 1/yr;
                else aux = 1/xr;
                xr *= aux;
                yr *= aux;
                sf::Vector2f min,max;
                min.x = (1.f - yr) / 2.f;
                min.y = (1.f - xr) / 2.f;
                max.x = 1.f - min.x*2.0;
                max.y = 1.f - min.y*2.0;
                view.reset(sf::FloatRect(0,0,desiredSize.x,desiredSize.y));
                view.setViewport(sf::FloatRect(min.x,min.y,max.x,max.y));
                window->setView(view);


                if(wantAnimation && !introDone){
                    sf::Clock timer;
                    sf::Sprite dark;
                    sf::Texture text;
                    bool closing = true;
                    sf::Image black;
                    black.create(window->getSize().x, window->getSize().y+200, sf::Color::Black);
                    text.loadFromImage(black);
                    dark.setTexture(text);
                    dark.setOrigin(dark.getLocalBounds().width/2,dark.getLocalBounds().height/2);
                    dark.setPosition(window->getSize().x/2,window->getSize().y/2);
                    float time = 0;
                    float alpha = 255;
                    while(!introDone && wantAnimation){
                        dark.setColor(sf::Color(0,0,0,int(alpha)));
                        time += timer.restart().asSeconds();
                        if(time > 0.05){
                            alpha -= 15;
                            time = 0;
                        }
                        window->clear();
                        window->draw(s);
                        window->draw(dark);
                        window->display();
                        if(alpha <= 0) {
                            introDone = true;
                        }
                        while (window->pollEvent(event)) {
                            switch (event.type) {
                            case sf::Event::Closed:
                                window->close();
                                break;
                            case sf::Event::KeyPressed:
                                if (event.key.code == sf::Keyboard::Escape) window->close();
                                else open = false;
                                break;
                            default:
                                break;
                           }
                      }
                 }
             }

            window->clear();
            window->draw(s);
            window->display();

        }
        
        sf::Clock timer;
        sf::Sprite dark;
        sf::Texture text;
        bool closing = true;
        sf::Image black;
        black.create(window->getSize().x, window->getSize().y+200, sf::Color::Black);
        text.loadFromImage(black);
        dark.setTexture(text);
        dark.setOrigin(dark.getLocalBounds().width/2,dark.getLocalBounds().height/2);
        dark.setPosition(window->getSize().x/2,window->getSize().y/2);
        float time = 0;
        float alpha = 0.1;
        while(closing && wantAnimation){
            dark.setColor(sf::Color(0,0,0,int(alpha)));
            time += timer.restart().asSeconds();
            if(time > 0.05){
                alpha += 8;
                time = 0;
            }
            window->clear();
            window->draw(s);
            window->draw(dark);
            window->display();
            if(alpha >= 255) closing = false;
            while (window->pollEvent(event)) {
                switch (event.type) {
                case sf::Event::Closed:
                    window->close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) window->close();
                    else open = false;
                    break;
                default:
                    break;
                }
            }
        }
        sf::Event e; while (window->pollEvent(e)) { }
        window->setView(window->getDefaultView());
    }
