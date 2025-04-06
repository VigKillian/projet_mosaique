CXX = g++
CXXFLAGS = -std=c++17 -I./imgui -I./imgui/backends -I. -Wall
TARGET = app

SRC = interface.cpp \
      imgui/imgui.cpp \
      imgui/imgui_draw.cpp \
      imgui/imgui_tables.cpp \
      imgui/imgui_widgets.cpp \
      imgui/imgui_demo.cpp \
      imgui/backends/imgui_impl_glfw.cpp \
      imgui/backends/imgui_impl_opengl3.cpp\
      imgui/ImGuiFileDialog.cpp

LIBS = -lGL -lglfw -ldl -lpthread -lX11

SRC_DIR = src
MY_PROGRAMS = $(wildcard $(SRC_DIR)/*.cpp)
MY_BINARIES = $(patsubst $(SRC_DIR)/%.cpp, exe/%, $(MY_PROGRAMS))

all: $(TARGET) $(MY_BINARIES)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

exe/%: $(SRC_DIR)/%.cpp
	@mkdir -p exe
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f exe/*

.PHONY: all clean
