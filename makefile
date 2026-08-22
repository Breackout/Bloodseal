CXX  := c++
MODE ?= release

# --- Librerie condivise da entrambi i programmi ---
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 sdl3-image sdl3-ttf)
SDL_LIBS   := $(shell pkg-config --libs sdl3 sdl3-image sdl3-ttf)

CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -MMD -MP \
            $(SDL_CFLAGS)
LDFLAGS  := $(SDL_LIBS)

ifeq ($(MODE),debug)
    CXXFLAGS += -g -O0 -DDEBUG -fsanitize=address,undefined
    LDFLAGS  += -fsanitize=address,undefined
else ifeq ($(MODE),release)
    CXXFLAGS += -O3 -DNDEBUG -flto
    LDFLAGS  += -flto
else
    $(error MODE sconosciuto: '$(MODE)'. Usa 'debug' o 'release')
endif

# --- Directory programmi ---
PROGRAM_DIR := zProgram

# --- Programma 1: il gioco ---
GAME_TARGET  := $(PROGRAM_DIR)/game
GAME_SRC_DIR := src
GAME_OBJ_DIR := build/$(MODE)/game
GAME_SRCS := $(wildcard $(GAME_SRC_DIR)/*.cpp)
GAME_OBJS := $(patsubst $(GAME_SRC_DIR)/%.cpp,$(GAME_OBJ_DIR)/%.o,$(GAME_SRCS))
GAME_DEPS := $(GAME_OBJS:.o=.d)

# --- Programma 2: Level Editor ---
EDITOR_TARGET  := $(PROGRAM_DIR)/lvledit
EDITOR_SRC_DIR := tools/leveleditor
EDITOR_OBJ_DIR := build/$(MODE)/leveleditor
EDITOR_SRCS := $(wildcard $(EDITOR_SRC_DIR)/*.cpp)
EDITOR_OBJS := $(patsubst $(EDITOR_SRC_DIR)/%.cpp,$(EDITOR_OBJ_DIR)/%.o,$(EDITOR_SRCS))
EDITOR_DEPS := $(EDITOR_OBJS:.o=.d)

.PHONY: all game lvledit run run-lvledit debug release clean

# --- Build completo ---
all: game lvledit

# --- Build gioco ---
game: $(GAME_TARGET)

# --- Build editor ---
lvledit: $(EDITOR_TARGET)

# --- Link gioco ---
$(GAME_TARGET): $(GAME_OBJS)
	@mkdir -p $(PROGRAM_DIR)
	$(CXX) $(GAME_OBJS) -o $@ $(LDFLAGS)

# --- Link editor ---
$(EDITOR_TARGET): $(EDITOR_OBJS)
	@mkdir -p $(PROGRAM_DIR)
	$(CXX) $(EDITOR_OBJS) -o $@ $(LDFLAGS)

# --- Compilazione gioco ---
$(GAME_OBJ_DIR)/%.o: $(GAME_SRC_DIR)/%.cpp
	@mkdir -p $(GAME_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Compilazione editor ---
$(EDITOR_OBJ_DIR)/%.o: $(EDITOR_SRC_DIR)/%.cpp
	@mkdir -p $(EDITOR_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Dipendenze automatiche ---
-include $(GAME_DEPS) $(EDITOR_DEPS)

# --- Esecuzione gioco ---
run: game
	./$(GAME_TARGET)

# --- Esecuzione Level Editor ---
run-lvledit: lvledit
	./$(EDITOR_TARGET)

# --- Build debug ---
debug:
	$(MAKE) MODE=debug all

# --- Build release ---
release:
	$(MAKE) MODE=release all

# --- Pulizia ---
clean:
	rm -rf build $(PROGRAM_DIR)
