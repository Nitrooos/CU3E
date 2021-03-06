#include "LoadingScreen.hpp"
#include <SFML/Window/Event.hpp>

LoadingScreen::LoadingScreen(RenderWindow &window, GraphicsManager *gm)
    : grMananger(gm), window(window) {
    this->onInit();

    grMananger->addObserver(this);
    grMananger->onLoad();

    this->waitForReaction();
}

void LoadingScreen::update(float percent, bool wasOpenGLused) {
    static const string texts[] = {
        "Wznoszenie scian...", "Pucowanie czajnikow...", "Malowanie balustrad...",
        "Nalewanie wod gruntowych...", "Uaktywnianie teleportu...", "Sprzatanie po melanzu...",
        "Gromadzenie fotonow", "Zamykanie drzwi" };
    static int count = 0;

    text.setString(texts[count]);
    ++count;

    progressBar.setSize(Vector2f{percent*window.getSize().x/100.0, 20.0});

    if (wasOpenGLused)
        window.popGLStates();

    window.clear(Color::Black);
    window.draw(background);
    window.draw(progressBar);
    window.draw(text);
    window.display();

    window.pushGLStates();
}

void LoadingScreen::onInit() {
    font.loadFromFile("data/fonts/castlen.ttf");
    text.setFont(this->font);
    text.setCharacterSize(40);
    text.setPosition(window.getSize().x/2, window.getSize().y - text.getCharacterSize() - 5);

    progressBar.setPosition(Vector2f{0.0, 0.0});
    progressBar.setFillColor(Color{255, 255, 255, 127});

    kastle.loadFromFile("data/loadingScreen/CU3E_logo.png");

    Vector2u kastleSize = kastle.getSize(),
             windowSize = window.getSize();

    background.setTexture(kastle);
    background.scale(Vector2f{(float)windowSize.x/kastleSize.x, (float)windowSize.y/kastleSize.y});

    window.clear(Color::Black);
    window.draw(background);
    window.display();
}

void LoadingScreen::waitForReaction() {
    text.setPosition(Vector2f{window.getSize().x/2 - text.getLocalBounds().width,
                              window.getSize().y - text.getCharacterSize() - 5});
    text.setString("Wcisnij dowolny klawisz aby rozpoczac");

    window.popGLStates();

    window.clear(Color::Black);
    window.draw(background);
    window.draw(progressBar);
    window.draw(text);
    window.display();

    window.pushGLStates();

    Event ev;
    while (1) {
        window.waitEvent(ev);
        if (ev.type == Event::KeyPressed)
            break;
    }
}
