#include <QtGui>
#include <iostream>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include <string>

class Frog  {

    public:
    void draw(QPainter& painter);
    void keyPressed();
    Frog(std::string imageStay, std::string ImageJump, int column, Mix_Chunk* croak);

    private:
    QTimer* timer_image;
    QTimer* timer_push;
    QImage stay;
    QImage jump;
    int row;
    int column;
    Mix_Chunk* croak;
};

Frog::Frog(std::string imageStay, std::string imageJump, int column, Mix_Chunk* croak) {
    row = 0;
    stay.load(imageStay.c_str());
    jump.load(imageJump.c_str());
    this->column = column;
    this->croak = croak;
    // FIXME: timers
}

void Frog::draw(QPainter& painter) {
    // FIXME: animation
    if(row%2) {
	painter.drawImage(column, 48*(9-row/2-0.5), jump);
    }
    else {
	painter.drawImage(column, 48*(9-row/2), stay);
    }
}

void Frog::keyPressed() {
    Mix_PlayChannel(-1, croak, 0);
    row++;
}

class Frogger : public QWidget {
    public:
    Frogger(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    ~Frogger();

    private:
    QTimer* timer;
    QImage car[5];
    QImage truck[2];
    QImage terrain[7];
    Mix_Chunk* croak;
};

Frog* frogs[4];

Frogger::Frogger(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Frogger");
    setFixedSize(640, 480);
    QRect frect = frameGeometry();
    frect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(frect.topLeft());

    croak = Mix_LoadWAV("sounds/4.wav");

    frogs[0] = new Frog("images/50.png","images/51.png",48*(3+2*0),croak);
    frogs[1] = new Frog("images/52.png","images/53.png",48*(3+2*1),croak);
    frogs[2] = new Frog("images/54.png","images/55.png",48*(3+2*2),croak);
    frogs[3] = new Frog("images/56.png","images/57.png",48*(3+2*3),croak);

    car[0].load("images/100.png");
    car[1].load("images/101.png");
    car[2].load("images/102.png");
    car[3].load("images/103.png");
    car[4].load("images/104.png");

    truck[0].load("images/105.png");
    truck[1].load("images/106,ong");

    terrain[0].load("images/200.png");
    terrain[1].load("images/201.png");
    terrain[2].load("images/202.png");
    terrain[3].load("images/203.png");
    terrain[4].load("images/204.png");
    terrain[5].load("images/205.png");
    terrain[6].load("images/206.png");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000/33);
}

Frogger::~Frogger() {
    Mix_FreeChunk(croak);
}

void Frogger::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    // draw terrain
    for(int i = 0; i != 10; ++i) {
        painter.drawImage(0, 48*i, terrain[i % 7]);
    }
    // draw frogs
   for(int i = 0; i != 4; ++i) {
	frogs[i]->draw(painter);
   }
}

void Frogger::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_Up:
            frogs[0]->keyPressed();
            break;
        case Qt::Key_Z:
            frogs[1]->keyPressed();
            break;
        case Qt::Key_P: 
            frogs[2]->keyPressed();
           break;
        case Qt::Key_Q:
            frogs[3]->keyPressed();
            break;
        case Qt::Key_Escape:
            exit(0);
        default:
            break;
    }
}

int main(int argc, char* argv[]) {

    // use SDL for audio
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16SYS;
    int audio_channels = 2;
    int audio_buffers = 4096;
    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
        fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
        exit(1);
    }

    QApplication app(argc, argv);

    Frogger frogger;
    frogger.show();

    int ret = app.exec();

    // halt audio
    Mix_CloseAudio();
    SDL_Quit();

    return ret;
}
