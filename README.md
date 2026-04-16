Feito individualmente. Para compilar:

```
gcc -o futebol futebol.c $(sdl2-config --cflags --libs) -lSDL2_mixer -lGL -lGLU -lglut -lm
```

O jogo não inclui movimentação dos jogadores ou arquibancada com torcida. Sinto muito por isso!
O código está poluído com comentários, pois por não ser muito familiarizado com a documentação do GLUT e NDC, precisei anotar minhas procedures para não esquecer o que fazem. Acredito que o código esteja legível mesmo assim.
