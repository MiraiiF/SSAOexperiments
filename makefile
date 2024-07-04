main:
	g++ -c main.cpp -o Debug/main.o

source:
	cd Sources && g++ -c Camera.cpp -o ../Debug/camera.o && g++ -c Shader.cpp -o ../Debug/shader.o
	cd Sources && g++ -c stb_image.cpp -o ../Debug/stb.o && g++ -c texture.cpp -o ../Debug/texture.o
	g++ -c glad/src/glad.c -o Debug/glad.o

all: main source
	cd Debug && g++ main.o camera.o shader.o stb.o texture.o glad.o -lglfw -lassimp -o SSAO.mirai

compile: all
	cd Debug && rm main.o camera.o shader.o stb.o texture.o glad.o

run:
	cd Debug && ./SSAO.mirai

