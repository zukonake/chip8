SOURCE_DIR = src
OBJ_DIR = bin
DEPEND_DIR = depend

TARGET = chip8-x64.exe

CPP_FILES = $(shell find $(SOURCE_DIR) -type f -name "*.cpp" -printf '%p ')
OBJ_FILES = $(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(notdir $(CPP_FILES))))

LIBS = -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system -static -lgcc -lstdc++ -lpthread
DEBUG_FLAGS = -g -O0 -DDEBUG
WARNING_FLAGS = -Wall -Wextra
COMPILER = x86_64-w64-mingw32-g++
COMPILER_FLAGS = -L /usr/x86_64-w64-mingw32/lib -I $(SOURCE_DIR) --std=c++14 $(WARNING_FLAGS) $(DEBUG_FLAGS)

COMPILE = $(COMPILER) $(COMPILER_FLAGS)
LINK = $(COMPILER) $(COMPILER_FLAGS) $(OBJ_FILES)

.PHONY : clean

$(TARGET) : $(OBJ_FILES)
	$(LINK) -o $@ $(LIBS)

.SECONDEXPANSION:
$(OBJ_DIR)/%.o : $$(shell find $(SOURCE_DIR) -type f -name %.cpp)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEPEND_DIR)
	$(COMPILE) -c $< -o $@
	$(COMPILE) -MM $< > $(DEPEND_DIR)/$*.d
	@sed -i '1s/^/$(OBJ_DIR)\//' $(DEPEND_DIR)/$*.d

clean :
	$(RM) -r $(OBJ_DIR) $(DEPEND_DIR) $(TARGET)

-include $(subst $(OBJ_DIR)/,$(DEPEND_DIR)/,$(patsubst %.o,%.d,$(OBJ_FILES)))
