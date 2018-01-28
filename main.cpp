#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <stdio.h>
#include <Portada.hpp>

bool isWhite(sf::Image& image, float px, float py){
	return image.getPixel(px, py) == sf::Color::White;
}

float getAngle(sf::Vector2f &orig, sf::Vector2i &des) {
    return std::atan2(des.y - orig.y, des.x - orig.x)*180/(M_PI);
}

float getModule(const sf::Vector2f &orig, const sf::Vector2f &des) {
    return std::sqrt(std::pow(std::abs(des.x-orig.x), 2) + std::pow(std::abs(des.y-orig.y), 2));
}

int randomSignOne(){
    return 1 - 2*std::rand()%2;
}


float getAngle(const sf::Vector2f &orig,const sf::Vector2f &des) {
    return std::atan2(des.y - orig.y, des.x - orig.x)*180/(M_PI);
}

struct Chameleon{
    sf::Sprite chameleonLines;
    sf::Sprite chameleonColour;
    sf::Texture lines;
    sf::Texture color;

//    sf::Texture mask;
    sf::Image chameleonMask;
};

struct Anchor {

    Anchor(){}
    Anchor(const Anchor& a){
        posx = a.posx;
        posy = a.posy;
        angle= a.angle;
    }
    float posx;
    float posy;
    float angle;
};

struct QuadreDescriptor{

    QuadreDescriptor(){};
    QuadreDescriptor(std::string n, const Anchor& i, const Anchor& d)
        :name(n)
        ,init(i)
        ,dest(d)
    {};
    std::string name;
    Anchor init;
    Anchor dest;
};

struct Quadre : public sf::Sprite {
    Anchor init;
    Anchor dest;
    sf::Texture painting;
    int indexInDescriptors;
};

enum EState {
  playing, guard, movingScreen, ending
};

int main(){

    /* initialize random seed: */
    std::srand (time(NULL));

	//SFML OBJECTS
	sf::View view;
	sf::Event event;
	sf::Clock deltaClock;

    float deltatime = 0.0;

    sf::Font font;
    font.loadFromFile("res/font.otf");

    sf::Text text;
    text.setFont(font);

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), L"xamilion", sf::Style::Close);
    view.reset(sf::FloatRect(0,0,
                             window.getSize().x, window.getSize().y));

    window.setFramerateLimit(60);



    Portada portada;
    portada.notAnimation();
    portada.display(&window, "res/portada0.png");
    portada.animation();
    portada.display(&window, "res/portada1.png");
    portada.display(&window, "res/controls.png");


    sf::Music m;
    m.openFromFile("res/attack.ogg");
    m.openFromFile("res/gramatik_just_jammin.ogg");
    m.setLoop(true);
    m.setVolume(75);
    m.play();

    sf::SoundBuffer buffer;
    buffer.loadFromFile("res/attack.ogg");
    sf::Sound stepsound;
    stepsound.setBuffer(buffer);


    std::vector<QuadreDescriptor> quadreDescriptors;
    std::string line;
    std::ifstream myfile ("res/quadres.txt");
    if (myfile.is_open()) {

        std::getline (myfile,line);
        while (line[0] == '#') std::getline (myfile,line);

        while (line[0] != '$') {

            if(line[0] == '-'){

                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                std::string name = line;

                Anchor anchorIni, anchorEnd;
                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorIni.posx = std::stoi(line);
                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorIni.posy = std::stoi(line);
                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorIni.angle = std::stoi(line);


                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorEnd.posx = std::stoi(line);
                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorEnd.posy = std::stoi(line);
                std::getline (myfile,line);
                while (line[0] == '#') std::getline (myfile,line);
                anchorEnd.angle = std::stoi(line);

                quadreDescriptors.emplace_back(QuadreDescriptor(name, anchorIni, anchorEnd));

            }
            std::getline (myfile,line);
            while (line[0] == '#') std::getline (myfile,line);
        }
        myfile.close();
    } else std::cout << "not oppened quadres file " << std::endl;


    int quadreIndex = 9;

    Quadre* quadreToPick = new Quadre;
    quadreToPick->painting.loadFromFile("res/quadres/"+quadreDescriptors[quadreIndex].name);
    quadreToPick->setTexture(quadreToPick->painting);
    quadreToPick->init = quadreDescriptors[quadreIndex].init;
    quadreToPick->dest = quadreDescriptors[quadreIndex].dest;
    quadreToPick->indexInDescriptors = quadreIndex;
    ++quadreIndex;

    Quadre* quadreToDraw = new Quadre;
    quadreToDraw->painting.loadFromFile("res/quadres/"+quadreDescriptors[quadreIndex].name);
    quadreToDraw->setTexture(quadreToDraw->painting);
    quadreToDraw->init = quadreDescriptors[quadreIndex].init;
    quadreToDraw->dest = quadreDescriptors[quadreIndex].dest;
    quadreToDraw->indexInDescriptors = quadreIndex;
    ++quadreIndex;

    Quadre* quadreForTransition = new Quadre;
    quadreForTransition->painting.loadFromFile("res/quadres/"+quadreDescriptors[quadreIndex].name);
    quadreForTransition->setTexture(quadreForTransition->painting);
    quadreForTransition->init = quadreDescriptors[quadreIndex].init;
    quadreForTransition->dest = quadreDescriptors[quadreIndex].dest;
    quadreForTransition->indexInDescriptors = quadreIndex;
    ++quadreIndex;


    Chameleon chameleon;
    if(! chameleon.lines.loadFromFile("res/chameleonLines.png"))std::cout << "nochameleonlines" << std::endl;
    chameleon.color.loadFromFile("res/chameleonColor.png");
    chameleon.chameleonMask.loadFromFile("res/chameleonMask.png");
    //chameleon.chameleonMask.setTexture(chameleon.mask);
    chameleon.chameleonLines.setTexture(chameleon.lines, true);

    const float chamAnimTimer = 0.05;
    float timeSinceLastAnim = 0;
    int currentChameleonFrame = 0;
    int currentChameleonRow = 0;
    const int CHAMELEON_ROWS = 1;
    const int CHAMELEON_FRAMES = 8;
    const int CHAMELEON_WIDTH = chameleon.chameleonLines.getGlobalBounds().width;
    const int CHAMELEON_HEIGHT = chameleon.chameleonLines.getGlobalBounds().height;
    const int CHAMELEON_FRAME_WIDTH = CHAMELEON_WIDTH/CHAMELEON_FRAMES;
    const int CHAMELEON_ROEW_HEIGHT = CHAMELEON_HEIGHT/CHAMELEON_ROWS;

    chameleon.chameleonLines.setTextureRect(sf::IntRect(currentChameleonFrame*CHAMELEON_FRAME_WIDTH,currentChameleonRow*CHAMELEON_ROEW_HEIGHT,CHAMELEON_WIDTH/CHAMELEON_FRAMES, CHAMELEON_HEIGHT));
    chameleon.chameleonColour.setTexture(chameleon.color, true);
    chameleon.chameleonLines.setOrigin(chameleon.chameleonLines.getGlobalBounds().width/2, chameleon.chameleonLines.getGlobalBounds().height/2);
    chameleon.chameleonColour.setOrigin(chameleon.chameleonColour.getGlobalBounds().width/2, chameleon.chameleonColour.getGlobalBounds().height/2);

    chameleon.chameleonColour.setPosition(120,400);
    chameleon.chameleonLines.setPosition(120,400);
    chameleon.chameleonColour.setRotation(90);
    chameleon.chameleonLines.setRotation(90);

    std::vector<sf::Texture> cuttedColors;
    for(int i = 0; i < CHAMELEON_FRAMES; ++i){
        sf::Image temporalImage = chameleon.color.copyToImage();
        sf::Image maskImage;
        maskImage.create(CHAMELEON_WIDTH/CHAMELEON_FRAMES,CHAMELEON_HEIGHT);
        maskImage.copy(chameleon.chameleonMask,0,0,
                       sf::IntRect(i*CHAMELEON_FRAME_WIDTH,
                                   currentChameleonRow*CHAMELEON_ROEW_HEIGHT,
                                   CHAMELEON_WIDTH/CHAMELEON_FRAMES,
                                   CHAMELEON_HEIGHT));

        temporalImage = chameleon.color.copyToImage();
        for(int i = 0; i < temporalImage.getSize().x; ++i){
            for(int j = 0; j < temporalImage.getSize().y; ++j){
                if(i < maskImage.getSize().x && j < maskImage.getSize().y){
                    if(maskImage.getPixel(i,j) == sf::Color::Green){
                        temporalImage.setPixel(i,j, sf::Color::Transparent);
                    }
                }
                if(i > temporalImage.getSize().y-95)temporalImage.setPixel(i,j, sf::Color::Transparent);
            }
        }
        cuttedColors.push_back(sf::Texture());
        cuttedColors.back().loadFromImage(temporalImage);
    }

    chameleon.color = cuttedColors[currentChameleonFrame];
    chameleon.chameleonColour.setTexture(chameleon.color, true);
    chameleon.chameleonColour.setRotation(chameleon.chameleonLines.getRotation());



    std::vector<sf::Sprite> bgs(2);
    std::vector<sf::Texture> bgsTex (1);
    bgsTex[0].loadFromFile("res/bg0.png");

    int BG_X = 1337;
    for(int i = 0; i < bgs.size(); ++i){
        int randomelem = std::rand()%bgsTex.size();
        bgs[i].setTexture(bgsTex[randomelem]);
        bgs[i].setOrigin(bgs[i].getGlobalBounds().width/2, bgs[i].getGlobalBounds().height/2);

        BG_X = bgs[i].getGlobalBounds().width;
        bgs[i].setPosition(bgs[i].getGlobalBounds().width/2+i*BG_X,bgs[i].getGlobalBounds().height/2);
    }
    const int BG_Y = bgs[0].getGlobalBounds().height;
    const int HALF_BG_X = bgs[0].getGlobalBounds().width/2;




    std::vector<sf::Sprite> overbgs(2);
    std::vector<sf::Texture> overbgsTex (1);
    overbgsTex[0].loadFromFile("res/overbg0.png");

    for(int i = 0; i < overbgs.size(); ++i){
        int randomelem = std::rand()%overbgsTex.size();
        overbgs[i].setTexture(overbgsTex[randomelem]);
        overbgs[i].setOrigin(overbgs[i].getGlobalBounds().width/2, overbgs[i].getGlobalBounds().height/2);

        const int OVBG_X = overbgs[0].getGlobalBounds().width;
        overbgs[i].setPosition(overbgs[i].getGlobalBounds().width/2+i*OVBG_X,overbgs[i].getGlobalBounds().height/2);
    }


    quadreToPick->setOrigin(quadreToPick->getGlobalBounds().width/2,quadreToPick->getGlobalBounds().height/2);
    quadreToDraw->setOrigin(quadreToDraw->getGlobalBounds().width/2,quadreToDraw->getGlobalBounds().height/2);
    quadreForTransition->setOrigin(quadreForTransition->getGlobalBounds().width/2,quadreForTransition->getGlobalBounds().height/2);

    quadreToPick->setPosition(BG_X/4, BG_Y/3);
    quadreToDraw->setPosition(BG_X/4*3, BG_Y/3);
    quadreForTransition->setPosition(BG_X/4*5, BG_Y/3);

    EState gameState = EState::playing;
    float screenMovement;
    screenMovement = 0;


    bool guardGone = false;
    bool guardAnimated = true;
    bool animationDone = false;
    sf::Sprite guardS;
    sf::Texture guardT;

    guardT.loadFromFile("res/guard.png");
    guardS.setTexture(guardT);

    float timeSinceLastAnimGuard = 0;
    float guardAnimTimer = 0.2;
    int guardActions = 0;
    int currentGuardFrame = 0;
    const int GUARD_FRAMES = 4;
    const int GUARD_FRAME_WIDTH = guardS.getGlobalBounds().width/GUARD_FRAMES;
    const int GUARD_HEIGHT = guardS.getGlobalBounds().height;
    guardS.setTextureRect(sf::IntRect(currentGuardFrame*GUARD_FRAME_WIDTH, 0,
                                                  GUARD_FRAME_WIDTH, GUARD_HEIGHT));
    guardS.setOrigin(guardS.getGlobalBounds().width/2,0);

    guardS.setPosition(-800, BG_Y-guardS.getGlobalBounds().height-10);

    //GAME LOOP
	while(window.isOpen()){

        //Deltatime
        deltatime = deltaClock.restart().asSeconds();

		//Loop for handling events
		while(window.pollEvent(event)){
			switch (event.type){
				//Close event
				case sf::Event::Closed:
					window.close();
					break;
				//KeyPressed event
				case  sf::Event::KeyPressed:
					//Close key
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::N) {
                    screenMovement = 0;
                    //m.pause();
                    gameState = EState::movingScreen;
                    guardGone = false;
                    guardAnimated = true;
                    animationDone = false;
                    timeSinceLastAnimGuard = 0;
                }
                //Default
				default:
					//Do nothing
					break;
			}
		}	


        if(gameState == EState::ending){
            static float timer = 0.0;
            timer += deltatime;
            quadreToDraw->painting.loadFromFile("res/quadres/ending.png");
            quadreToDraw->setTexture(quadreToDraw->painting, true);
            static int currentfinalFrame = 0;
            const int finalFrames = 4;
            const int finalframeheight = quadreToDraw->getGlobalBounds().height;
            const int finalframeSize = quadreToDraw->getGlobalBounds().width/finalFrames;
            quadreToDraw->setTextureRect( sf::IntRect(currentfinalFrame*finalframeSize,0,finalframeSize,finalframeheight));
            if(timer >= 1){
                currentfinalFrame++;
                if(currentfinalFrame >= finalFrames){
                    quadreToDraw->setTextureRect( sf::IntRect(3*finalframeSize,0,finalframeSize,finalframeheight));
                    if(timer >= 3){
                        portada.display(&window, "res/end.png");
                        window.close();
                    }
                }else {
                    currentfinalFrame = std::min(finalFrames-1,currentfinalFrame);
                    quadreToDraw->setTextureRect( sf::IntRect(currentfinalFrame*finalframeSize,0,finalframeSize,finalframeheight));
                    timer = 0;
                }
            }
        }
        else if(gameState == EState::movingScreen){

            if(guardGone){
                float movement = -600*deltatime;
                if(screenMovement + movement > HALF_BG_X){
                    movement = HALF_BG_X - (screenMovement+movement);
                }

                screenMovement += movement;

                for(uint i = 0; i < bgs.size(); ++i){
                    bgs[i].move(movement, 0);
                    if(bgs[i].getPosition().x <= -1*HALF_BG_X) bgs[i].setPosition(BG_X+HALF_BG_X, bgs[i].getGlobalBounds().height/2);
                }

                for(uint i = 0; i < overbgs.size(); ++i){
                    overbgs[i].move(movement, 0);
                    if(overbgs[i].getPosition().x <= -1*HALF_BG_X) overbgs[i].setPosition(BG_X+HALF_BG_X, overbgs[i].getGlobalBounds().height/2);
                }

                chameleon.chameleonLines.move(movement, 0);
                chameleon.chameleonColour.move(movement, 0);

                quadreToPick->move(movement, 0);
                quadreToDraw->move(movement, 0);
                quadreForTransition->move(movement, 0);


                if(screenMovement <= -1*HALF_BG_X){
                    delete quadreToPick;

                    quadreToPick = quadreToDraw;
                    quadreToDraw = quadreForTransition;

                    if(quadreIndex >= quadreDescriptors.size()) quadreIndex = quadreDescriptors.size()-1;
                    quadreForTransition = new Quadre();
                    quadreForTransition->indexInDescriptors = quadreIndex;
                    quadreForTransition->painting.loadFromFile("res/quadres/"+quadreDescriptors[quadreIndex].name);
                    quadreForTransition->setTexture(quadreForTransition->painting);
                    quadreForTransition->init = quadreDescriptors[quadreIndex].init;
                    quadreForTransition->dest = quadreDescriptors[quadreIndex].dest;
                    quadreForTransition->setOrigin(quadreForTransition->getGlobalBounds().width/2,quadreForTransition->getGlobalBounds().height/2);

                    quadreToPick->setPosition(BG_X/4, BG_Y/3);
                    quadreToDraw->setPosition(BG_X/4*3, BG_Y/3);
                    quadreForTransition->setPosition(BG_X/4*5, BG_Y/3);
                    ++quadreIndex;

                    gameState = EState::playing;

                    m.setVolume(75);
//                    m.play();
                }
            }
            else {
                const float guardSpeed = 400;

                if(!guardAnimated){

                    timeSinceLastAnimGuard += deltatime;

                    if(guardActions == 0) {
                        if(timeSinceLastAnimGuard >= 1){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.5){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.2){
                            guardAnimated = true;
                        }
                    }
                    else if(guardActions == 1) {
                        if(timeSinceLastAnimGuard >= 1){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.5){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.2){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.5){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.9){
                            guardAnimated = true;
                        }
                    }

                    else if(guardActions == 2) {
                        if(timeSinceLastAnimGuard >= 1){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.2){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.4){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.6){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 1.8){
                            guardS.setScale(-1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.0){
                            guardS.setScale(1.f,1.f);
                        }
                        if(timeSinceLastAnimGuard >= 2.4){
                            guardAnimated = true;
                        }
                    }
                    else if(guardActions == 3) {
                        if(timeSinceLastAnimGuard >= 1){
                            guardS.setScale(-1,1);
                        }
                        if(timeSinceLastAnimGuard >= 3){
                            guardAnimated = true;
                        }
                    }
                    else if(guardActions == 4) {
                        if(timeSinceLastAnimGuard >= 1.4){
                            guardAnimated = true;
                        }
                    }
                    else if(guardActions == 5) {
                        if(timeSinceLastAnimGuard >= 0.5){
                            guardS.setRotation(5*std::abs(std::sin(timeSinceLastAnimGuard*10))/2.5);
                        }
                        if(timeSinceLastAnimGuard >= 1.4){
                            guardAnimated = true;
                        }
                    }

                }
                else {
                    guardS.move(guardSpeed*deltatime,0);
                    timeSinceLastAnimGuard += deltatime;

                    if(timeSinceLastAnimGuard >= guardAnimTimer){
                        m.setVolume(m.getVolume()*0.5);

                        timeSinceLastAnimGuard = 0;
                        currentGuardFrame = (currentGuardFrame + 1)%GUARD_FRAMES;
                        guardS.setTextureRect(sf::IntRect(currentGuardFrame*GUARD_FRAME_WIDTH, 0,
                                                                      GUARD_FRAME_WIDTH, GUARD_HEIGHT));
                    }
                }

                if(guardS.getPosition().x >= BG_X +270){
                    guardS.setPosition(sf::Vector2f(-800,guardS.getPosition().y));
                    guardGone = true;
                }

                if(!animationDone && guardS.getPosition().x >= BG_X*0.75){
                    guardAnimated = false;
                    animationDone = true;
                    timeSinceLastAnim = 0;
                    guardActions = std::rand()%6;
                }

            }
        }
        else if(gameState == EState::playing){

            sf::Vector2f movement(0,0);
            static const float speedCham = 220;
            static const float rotationAngleCham = 60;
            static const float pitagoras = sqrt(22500+22500);


            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                movement.x =  1* speedCham*speedCham/pitagoras*deltatime;
                movement.y = -1* speedCham*speedCham/pitagoras*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
                movement.x = -1* speedCham*speedCham/pitagoras*deltatime;
                movement.y = -1* speedCham*speedCham/pitagoras*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                movement.x = +1* speedCham*speedCham/pitagoras*deltatime;
                movement.y = +1* speedCham*speedCham/pitagoras*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
                movement.x = -1* speedCham*speedCham/pitagoras*deltatime;
                movement.y = +1* speedCham*speedCham/pitagoras*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
                movement.y = -1* speedCham*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
                movement.y = +1* speedCham*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                movement.x = +1* speedCham*deltatime;
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
                movement.x = -1* speedCham*deltatime;
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
                chameleon.chameleonColour.rotate(rotationAngleCham*deltatime);
                chameleon.chameleonLines.rotate(rotationAngleCham*deltatime);
                movement.x = 0.001;
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)){
                chameleon.chameleonColour.rotate(-1*rotationAngleCham*deltatime);
                chameleon.chameleonLines.rotate(-1*rotationAngleCham*deltatime);
                movement.x = 0.001;
            }

            if(movement.x != 0 || movement.y != 0){

                timeSinceLastAnim += deltatime;

                if(timeSinceLastAnim >= chamAnimTimer){
                    timeSinceLastAnim = 0;
                    currentChameleonFrame = (currentChameleonFrame + 1)%CHAMELEON_FRAMES;
                    chameleon.chameleonLines.setTextureRect(sf::IntRect(currentChameleonFrame*CHAMELEON_FRAME_WIDTH,currentChameleonRow*CHAMELEON_ROEW_HEIGHT,CHAMELEON_WIDTH/CHAMELEON_FRAMES, CHAMELEON_HEIGHT));

                    chameleon.color = cuttedColors[currentChameleonFrame];
                    chameleon.chameleonColour.setTexture(chameleon.color, true);
                    chameleon.chameleonColour.setRotation(chameleon.chameleonLines.getRotation());
                }

            }

            chameleon.chameleonColour.move(movement);
            chameleon.chameleonLines.move(movement);

            if(chameleon.chameleonColour.getPosition().y > 3*BG_Y/4){
                chameleon.chameleonLines.setPosition(chameleon.chameleonLines.getPosition().x, 3*BG_Y/4);
                chameleon.chameleonColour.setPosition(chameleon.chameleonColour.getPosition().x, 3*BG_Y/4);
            }

            static int onetimeC = 0;
            static Anchor anchorWhenCoppyed;
            static bool Cpressed = false;
            static float lastCpressedTimer = 0;
            if(Cpressed){
                lastCpressedTimer += deltatime;
                if(lastCpressedTimer >= 0.4){
                    onetimeC = 0;
                }
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::C) && onetimeC == 0 && chameleon.chameleonLines.getPosition().x < HALF_BG_X){
                    onetimeC = 1;
                    Cpressed = true;

                    static sf::RenderTexture rtex;
                    rtex.clear();
                    chameleon.chameleonColour.setRotation(0);
                    rtex.create(uint(chameleon.chameleonColour.getGlobalBounds().width), uint(chameleon.chameleonColour.getGlobalBounds().height));

                    sf::Vector2f distDos(chameleon.chameleonColour.getPosition()-quadreToPick->getPosition());

                    auto oldPosition = quadreToPick->getPosition();
                    auto quadreOrigin = quadreToPick->getOrigin();

                    quadreToPick->setOrigin(quadreToPick->getOrigin() + distDos);
                    quadreToPick->setPosition(chameleon.chameleonColour.getGlobalBounds().width/2
                                             , chameleon.chameleonColour.getGlobalBounds().height/2);
                    quadreToPick->setRotation(-1*chameleon.chameleonLines.getRotation());

                    rtex.draw(*quadreToPick);
                    rtex.display();

                    quadreToPick->setRotation(0);
                    quadreToPick->setOrigin(quadreOrigin);
                    quadreToPick->setPosition(oldPosition);

                    static sf::Image imgFromRenderTexture;
                    const sf::Image& maskImage = chameleon.chameleonMask;
                    imgFromRenderTexture = rtex.getTexture().copyToImage();

                    //img.flipVertically();
                    chameleon.color.loadFromImage(imgFromRenderTexture);
                    cuttedColors.clear();
                    for(int i = 0; i < CHAMELEON_FRAMES; ++i){
                        sf::Image temporalImage = chameleon.color.copyToImage();
                        sf::Image maskImage;
                        maskImage.create(CHAMELEON_WIDTH/CHAMELEON_FRAMES,CHAMELEON_HEIGHT);
                        maskImage.copy(chameleon.chameleonMask,0,0,
                                       sf::IntRect(i*CHAMELEON_FRAME_WIDTH,
                                                   currentChameleonRow*CHAMELEON_ROEW_HEIGHT,
                                                   CHAMELEON_WIDTH/CHAMELEON_FRAMES,
                                                   CHAMELEON_HEIGHT));

                        temporalImage = chameleon.color.copyToImage();
                        for(int i = 0; i < temporalImage.getSize().x; ++i){
                            for(int j = 0; j < temporalImage.getSize().y; ++j){
                                if(i <= maskImage.getSize().x && j <= maskImage.getSize().y){
                                    if(maskImage.getPixel(i,j) == sf::Color::Green){
                                        temporalImage.setPixel(i,j, sf::Color::Transparent);
                                    }
                                }
                                                if(i > temporalImage.getSize().y-95)temporalImage.setPixel(i,j, sf::Color::Transparent);
                            }
                        }
                        cuttedColors.push_back(sf::Texture());
                        cuttedColors.back().loadFromImage(temporalImage);
                    }
                    chameleon.color = cuttedColors[currentChameleonFrame];
                    chameleon.chameleonColour.setTexture(chameleon.color, true);
                    chameleon.chameleonColour.setRotation(chameleon.chameleonLines.getRotation());

                    anchorWhenCoppyed.angle = chameleon.chameleonLines.getRotation();
                    anchorWhenCoppyed.posx = chameleon.chameleonLines.getPosition().x;
                    anchorWhenCoppyed.posy = chameleon.chameleonLines.getPosition().y;
                    stepsound.play();

            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::X)){
                onetimeC = 0;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::V) && chameleon.chameleonLines.getPosition().x > HALF_BG_X){

                 if(quadreIndex >= quadreDescriptors.size()) {
                         gameState = EState::ending;
                 }
                 else {
                     //float cang = chameleon.chameleonLines.getRotation();
                    sf::Vector2f cpos = chameleon.chameleonLines.getPosition();

                    sf::Vector2f qpos = quadreToDraw->getPosition() - sf::Vector2f(quadreToDraw->getGlobalBounds().width/2,
                                                                                  quadreToDraw->getGlobalBounds().height/2);

                    sf::Vector2f qinipos = quadreToPick->getPosition() - sf::Vector2f(quadreToPick->getGlobalBounds().width/2,
                                                                                      quadreToPick->getGlobalBounds().height/2);

                    QuadreDescriptor qdes = quadreDescriptors[quadreToDraw->indexInDescriptors];
                    QuadreDescriptor qini = quadreDescriptors[quadreToPick->indexInDescriptors];
    //                std::cout << "namedes-> " << qdes.name << " index ->" << quadreToPick->indexInDescriptors << std::endl;
                //    std::cout << "nameini-> " << qini.name << " index ->" << quadreToDraw->indexInDescriptors <<std::endl;
    //                std::cout << "chamrespecte q2: "<< cpos.x-qpos.x << " , " << cpos.y-qpos.y << std::endl;
       /*             std::cout << "cham: "<< cpos.x << " , " << cpos.y << std::endl; std::cout << "dest: "<< qpos.x+qdes.dest.posx << " , " << qpos.y+qdes.dest.posy << std::endl;                    std::cout << "inicham: "<< anchorWhenCoppyed.posx << " , " << anchorWhenCoppyed.posy << std::endl;                    std::cout << "iniq;  " << qinipos.x+qini.init.posx << " , " << qinipos.y+qini.init.posy << std::endl;
                    std::cout << "if: " << ( (cpos.x > (qpos.x+qdes.dest.posx - 40)) && (cpos.x < (qpos.x+qdes.dest.posx + 40))&& (cpos.y > (qpos.y+qdes.dest.posy - 40))&& (cpos.y < (qpos.y+qdes.dest.posy + 40)) ) << " , " << ( (anchorWhenCoppyed.posx > (qinipos.x+qini.init.posx - 80)) && (anchorWhenCoppyed.posx < (qinipos.x+qini.init.posx + 80))&& (anchorWhenCoppyed.posy > (qinipos.y+qini.init.posy - 80)) && (anchorWhenCoppyed.posy < (qinipos.y+qini.init.posy + 80)) )<< std::endl;
                    std::cout << "According to this, i started at " <<(anchorWhenCoppyed.posx - qinipos.x)<< " " <<(anchorWhenCoppyed.posy - qinipos.y)<< std::endl;*/

                    if(  (cpos.x > (qpos.x+qdes.dest.posx - 40))
                      && (cpos.x < (qpos.x+qdes.dest.posx + 40))
                      && (cpos.y > (qpos.y+qdes.dest.posy - 40))
                      && (cpos.y < (qpos.y+qdes.dest.posy + 40))
                    //&& cang > qdes.dest.angle-5 && cang < qdes.dest.angle+5
                      && (anchorWhenCoppyed.posx > (qinipos.x+qini.init.posx - 80))
                      && (anchorWhenCoppyed.posx < (qinipos.x+qini.init.posx + 80))
                      && (anchorWhenCoppyed.posy > (qinipos.y+qini.init.posy - 80))
                      && (anchorWhenCoppyed.posy < (qinipos.y+qini.init.posy + 80))
                    //&& anchorWhenCoppyed.angle > qini.dest.angle-5 && anchorWhenCoppyed.angle < qini.dest.angle+5
                            ) {
                        screenMovement = 0;
                        gameState = EState::movingScreen;
                        gameState = EState::movingScreen;
                        guardGone = false;
                        guardAnimated = true;
                        animationDone = false;
                        timeSinceLastAnimGuard = 0;
                    }
                }
            }


        }


        sf::Vector2i desiredSize(BG_X, BG_Y+60);
        int windowX = window.getSize().x, windowY = window.getSize().y;

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

		//Set window view, draw and display
        window.setView(view);

        sf::Color c = sf::Color(200,0,0,255);

        window.clear();


        for(int i = 0; i < bgs.size(); ++i){
            window.draw(bgs[i]);
        }


        window.draw(*quadreToPick);
        window.draw(*quadreToDraw);
        window.draw(*quadreForTransition);

        window.draw(chameleon.chameleonColour);
        window.draw(chameleon.chameleonLines);

        for(int i = 0; i < overbgs.size(); ++i){
            window.draw(overbgs[i]);
        }

/*
            window.setView(window.getDefaultView());
            window.clear(c);
            window.draw(endS);
 */

        window.draw(guardS);

  //      debug chameleon colors
/*        sf::Sprite s;
        s.setPosition(0,0);
        for(int i = 0; i < cuttedColors.size(); ++i){
            s.setTexture(cuttedColors[i]);
            window.draw(s);
            s.move(s.getGlobalBounds().width,0);
        }
*/

        text.setString(
                    std::to_string(int(chameleon.chameleonLines.getPosition().x))+" , "+std::to_string(int(chameleon.chameleonLines.getPosition().y))
                    +"\n resp.qpick->"+std::to_string(int(chameleon.chameleonLines.getPosition().x)-quadreToPick->getPosition().x+quadreToPick->getGlobalBounds().width/2)+" , "+std::to_string(int(chameleon.chameleonLines.getPosition().y-quadreToPick->getPosition().y+quadreToPick->getGlobalBounds().height/2))
                    +"\n resp.qDraw->"+std::to_string(int(chameleon.chameleonLines.getPosition().x)-quadreToDraw->getPosition().x+quadreToDraw->getGlobalBounds().width/2)+" , "+std::to_string(int(chameleon.chameleonLines.getPosition().y-quadreToDraw->getPosition().y+quadreToDraw->getGlobalBounds().height/2))
                    );
//        window.draw(text);
        window.display();

	}

}
