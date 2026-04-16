#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

/*

Para fins de realismo, usamos as medidas oficiais da FIFA, em que 1f = 100m.
    A espessura das linhas é de 12cm. O ponto central e a marca do pênalti têm
raio de 15cm e 12cm respectivamente IRL, mas usamos um raio fictício de 30cm para 
que estes sejam visíveis de longe sobre a linha central.

*/

#define PI 3.1415
#define GOL_Y_MAX   0.0366f
#define GOL_X_BORDA 0.525f
#define BOLA_RAIO   0.0035f

float bolaX = 0.0f;
float bolaY = 0.0f;
float velocidade = 0.01f;

int placarEsquerdo = 0;
int placarDireito  = 0;

Mix_Chunk *sfxWhistle = NULL;
Mix_Chunk *sfxGoal    = NULL;

void initAudio() {
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    sfxWhistle = Mix_LoadWAV("apito.wav");
    sfxGoal    = Mix_LoadWAV("gol.wav");

    Mix_PlayChannel(-1, sfxWhistle, 0);
}

typedef struct Jogador {
    float x, y; // Posição no campo
} Jogador;

Jogador timeAzul[11] = {
    { -0.50f,  0.00f },
    { -0.35f,  0.22f },
    { -0.35f,  0.08f },
    { -0.35f, -0.08f },
    { -0.35f, -0.22f },
    { -0.18f,  0.15f },
    { -0.18f,  0.00f },
    { -0.18f, -0.15f },
    { -0.05f,  0.20f },
    { -0.05f,  0.00f },
    { -0.05f, -0.20f },
};

Jogador timeVermelho[11] = {
    {  0.50f,  0.00f },
    {  0.35f,  0.22f },
    {  0.35f,  0.08f },
    {  0.35f, -0.08f },
    {  0.35f, -0.22f },
    {  0.18f,  0.15f },
    {  0.18f,  0.00f },
    {  0.18f, -0.15f },
    {  0.05f,  0.20f },
    {  0.05f,  0.00f },
    {  0.05f, -0.20f },
};

void desenharCorpoJogador(float cx, float cy, float r, float g, float b) {
    float largura = 0.005f;
    float altura = 0.012f;
    float cabeca = 0.008f;

    glColor3f(r, g, b);
    glBegin(GL_QUADS); // Ombros
        glVertex2f(cx - largura, cy - altura);
        glVertex2f(cx + largura, cy - altura);
        glVertex2f(cx + largura, cy + altura);
        glVertex2f(cx - largura, cy + altura);
    glEnd();
    
    glColor3f(0.0f, 0.0f, 0.0f); // Preto (pro cabelo)
    glBegin(GL_TRIANGLE_FAN); // Cabeças
        glVertex2f(cx, cy);
        for (float a = 0; a <= 2*PI; a += 0.1f)
            glVertex2f(cx + cabeca * 0.4f * cos(a),
                       cy + cabeca * 0.4f * sin(a));
    glEnd();
}

void desenharJogadores() {
    for (int i = 0; i < 11; i++)
        desenharCorpoJogador(timeAzul[i].x, timeAzul[i].y,
                       0.15f, 0.45f, 0.9f); // Azul

    for (int i = 0; i < 11; i++)
        desenharCorpoJogador(timeVermelho[i].x, timeVermelho[i].y,
                       0.9f, 0.2f, 0.15f); // Vermelho
}

void verificarGol() {
    if (bolaX - BOLA_RAIO > GOL_X_BORDA &&
        bolaY > -GOL_Y_MAX && bolaY < GOL_Y_MAX) {
        placarEsquerdo++;
        bolaX = 0.0f;
        bolaY = 0.0f;
        Mix_PlayChannel(-1, sfxGoal, 0);
    }
    else if (bolaX + BOLA_RAIO < -GOL_X_BORDA &&
             bolaY > -GOL_Y_MAX && bolaY < GOL_Y_MAX) {
        placarDireito++;
        bolaX = 0.0f;
        bolaY = 0.0f;
        Mix_PlayChannel(-1, sfxGoal, 0);
    }
}

void desenharTexto(float x, float y, const char *str) {
    glRasterPos2f(x, y); // Posição inicial do texto
    for (const char *c = str; *c != '\0'; c++)
        glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, *c);
}

void desenharPlacar() {
    char buf[64];

    glColor3f(0.0f, 0.0f, 0.0f); // Preto
    glRectf(-0.22f, 0.355f, 0.22f, 0.41f); // Fundo do placar

    glColor3f(0.9f, 0.75f, 0.1f); // Amarelo
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP); // Contorno do placar
        glVertex2f(-0.22f, 0.355f);
        glVertex2f( 0.22f, 0.355f);
        glVertex2f( 0.22f, 0.41f);
        glVertex2f(-0.22f, 0.41f);
    glEnd();

    sprintf(buf, "AZL  %d : %d  VML", placarEsquerdo, placarDireito);

    glColor3f(1.0f, 1.0f, 1.0f);
    desenharTexto(-0.135f, 0.369f, buf);
}

void desenharBola(float centroX, float centroY, float raio) {
    glColor3f(1.0f, 1.0f, 1.0f); // Branco

    glBegin(GL_TRIANGLE_FAN); // Preenchimento da bola
    glVertex2f(centroX, centroY);
    for (float angle = 0; angle <= 2*PI; angle += 0.05f)
        glVertex2f(centroX + raio * cos(angle), centroY + raio * sin(angle));
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f); // Preto
    glLineWidth(1.0f);

    glBegin(GL_LINE_LOOP); // Contorno da bola
    for (float angle = 0; angle <= 2*PI; angle += 0.05f)
        glVertex2f(centroX + raio * cos(angle), centroY + raio * sin(angle));
    glEnd();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': case 'W':
            bolaY += velocidade;
            if (bolaY + 0.011f > 0.32f) bolaY = 0.32f - 0.011f;
            break;
        case 's': case 'S':
            bolaY -= velocidade;
            if (bolaY - 0.011f < -0.32f) bolaY = -0.32f + 0.011f;
            break;
        case 'a': case 'A':
            bolaX -= velocidade;
            if (bolaX - 0.011f < -GOL_X_BORDA &&
                (bolaY <= -GOL_Y_MAX || bolaY >= GOL_Y_MAX))
                bolaX = -GOL_X_BORDA + 0.011f;
            break;
        case 'd': case 'D':
            bolaX += velocidade;
            if (bolaX + 0.011f > GOL_X_BORDA &&
                (bolaY <= -GOL_Y_MAX || bolaY >= GOL_Y_MAX))
                bolaX = GOL_X_BORDA - 0.011f;
            break;
        case 'r': case 'R': // Reseta a posição da bola e o placar
            placarEsquerdo = 0;
            placarDireito  = 0;
            bolaX = 0.0f;
            bolaY = 0.0f;
            Mix_PlayChannel(-1, sfxWhistle, 0);
            break;
        case 27: // ESC
            exit(0);
    }

    verificarGol();
    glutPostRedisplay();
}

void campoMetade() {
    glBegin(GL_LINE_LOOP); // Grande área
        glVertex2f(0.36f, -0.2015f);
        glVertex2f(0.525f, -0.2015f);
        glVertex2f(0.525f,  0.2015f);
        glVertex2f(0.36f,  0.2015f);
    glEnd();

    glBegin(GL_POLYGON); // Marca do pênalti
    for (float a = 0; a <= 2*PI; a += 0.05f)
        glVertex2f(0.415f + 0.003f * cos(a), 0.003f * sin(a));
    glEnd();

    glBegin(GL_LINE_STRIP); // Meia-lua
    for (float a = PI - 0.93f; a <= PI + 0.93f; a += 0.05f)
        glVertex2f(0.415f + 0.0915f * cos(a), 0.0915f * sin(a));
    glEnd();

    glBegin(GL_LINE_LOOP); // Pequena área
        glVertex2f(0.445f, -0.0915f);
        glVertex2f(0.525f, -0.0915f);
        glVertex2f(0.525f,  0.0915f);
        glVertex2f(0.445f,  0.0915f);
    glEnd();

    glBegin(GL_LINE_LOOP); // Gol
        glVertex2f(0.525f, -0.0366f);
        glVertex2f(0.555f, -0.0366f);
        glVertex2f(0.555f,  0.0366f);
        glVertex2f(0.525f,  0.0366f);
    glEnd();

    glBegin(GL_LINE_STRIP); // Escanteio inferior
    for (float a = 0; a <= PI/2; a += 0.05f)
        glVertex2f(0.525f - 0.01f * cos(a), -0.32f + 0.01f * sin(a));
    glEnd();

    glBegin(GL_LINE_STRIP); // Escanteio superior
    for (float a = -(PI/2); a <= 0; a += 0.05f)
        glVertex2f(0.525f - 0.01f * cos(a), 0.32f + 0.01f * sin(a));
    glEnd();
}

void desenharCampo() {
    glColor3f(0.35f, 0.5f, 0.2f); // Verde
    glRectf(-0.6f, -0.4f, 0.6f, 0.4f); // Gramado do campo (maior que as delimitacoes do campo)


    glColor3f(1.0f, 1.0f, 1.0f); // Branco para desenhar linhas
    glLineWidth(1.2f); // Espessura das linhas

    glBegin(GL_LINE_LOOP); // Linhas laterais e de fundo
        glVertex2f(-0.525f, -0.32f);
        glVertex2f( 0.525f, -0.32f);
        glVertex2f( 0.525f,  0.32f);
        glVertex2f(-0.525f,  0.32f);
    glEnd();

    glBegin(GL_LINES); // Linha do meio-campo
        glVertex2f(0.0f, -0.32f);
        glVertex2f(0.0f,  0.32f);
    glEnd();

    glBegin(GL_LINE_LOOP); // Círculo central
    for (float a = 0; a <= 2*PI; a += 0.05f)
        glVertex2f(0.0915f * cos(a), 0.0915f * sin(a));
    glEnd();

    glBegin(GL_LINE_LOOP); // Ponto central
    for (float a = 0; a <= 2*PI; a += 0.05f)
        glVertex2f(0.003f * cos(a), 0.003f * sin(a));
    glEnd();

    campoMetade(); // Desenha metade direita

    glPushMatrix(); 
    glScalef(-1.0f, 1.0f, 1.0f); // Espelha horizontalmente
    campoMetade(); // Desenha metade esquerda
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    desenharCampo();
    desenharBola(bolaX, bolaY, BOLA_RAIO);
    desenharJogadores();
    desenharPlacar();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("OpenGL");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    initAudio();

    glutMainLoop();

    Mix_FreeChunk(sfxWhistle);
    Mix_FreeChunk(sfxGoal);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}