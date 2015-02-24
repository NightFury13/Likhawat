#!/bin/bash
g++ -std=c++11 ./applications/Likhawat/segmenter/nogl.cpp -o ./applications/Likhawat/segmenter/no-holes -lopencv_core -lopencv_highgui -lopencv_imgproc
./applications/Likhawat/segmenter/no-holes ./applications/Likhawat/segmenter/input_images/input.jpeg
touch ./applications/Likhawat/segmenter/ho-gaya.txt