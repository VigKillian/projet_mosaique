CXX = g++
CXXFLAGS = -std=c++17 -I./Imgui -I./Imgui/backends -I. -Wall
TARGET = app

SRC = interface.cpp \
      Imgui/imgui.cpp \
      Imgui/imgui_draw.cpp \
      Imgui/imgui_tables.cpp \
      Imgui/imgui_widgets.cpp \
      Imgui/imgui_demo.cpp \
      Imgui/backends/imgui_impl_glfw.cpp \
      Imgui/backends/imgui_impl_opengl3.cpp\
      Imgui/ImGuiFileDialog.cpp

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
