#include <QtGui>
#include <iostream>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

class Frogger : public QWidget {
    public:
    Frogger(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    ~Frogger();

    private:
    QTimer* timer;
    QImage frog[4];
    QImage jump[4];
    QImage car[5];
    QImage truck[2];
    QImage terrain[7];
    Mix_Chunk* croak;
    int player[4];
};

Frogger::Frogger(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Frogger");
    setFixedSize(640, 480);
    QRect frect = frameGeometry();
    frect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(frect.topLeft());

    frog[0].load("images/50.png");
    frog[1].load("images/52.png");
    frog[2].load("images/54.png");
    frog[3].load("images/56.png");

    jump[0].load("images/51.png");
    jump[1].load("images/53.png");
    jump[2].load("images/55.png");
    jump[3].load("images/57.png");

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

    croak = Mix_LoadWAV("sounds/4.wav");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000/33);
    for(int i = 0; i != 4; ++i) {
        player[i] = 0;
    }
}

Frogger::~Frogger() {
    Mix_FreeChunk(croak);
}

void Frogger::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    for(int i = 0; i != 10; ++i) {
        painter.drawImage(0, 48*i, terrain[i % 7]);
    }
    for(int i = 0; i != 4; ++i) {
        if(player[i]%2) {
            painter.drawImage(48*(3+2*i), 48*(9-player[i]/2-0.5), jump[i]);
        }
        else {
            painter.drawImage(48*(3+2*i), 48*(9-player[i]/2), frog[i]);
        }
    }
}

void Frogger::keyPressEvent(QKeyEvent* event) {
    Mix_PlayChannel(-1, croak, 0);
    switch(event->key()) {
        case Qt::Key_Up:
            player[0]++;
            break;
        case Qt::Key_Z:
            player[1]++;
            break;
        case Qt::Key_P:
            player[2]++;
            break;
        case Qt::Key_Q:
            player[3]++;
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
