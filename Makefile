CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
SRCS = main.cpp analisador_lexico.cpp analisador_sintatico.cpp analisador_semantico.cpp gerador_codigo.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = mini_compilador

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run