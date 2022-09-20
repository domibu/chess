cmake_minimum_required(VERSION 3.6)

PROJECT(domibu)
ADD_DEFINITIONS(-Winline -std=gnu99  -g -lm -pthread -lrt -fcommon)

ADD_EXECUTABLE(domibu main.c BitBoardMagic.c Interface.c MoveGeneration.c Evaluation.c Search.c TranspositionTable.c MoveGeneration-1.c Search-1.c TranspositionTable-1.c)
TARGET_LINK_LIBRARIES(domibu m pthread rt)
