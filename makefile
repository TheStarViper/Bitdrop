# mingw32-make
# maybe also zip all the files together afterward to put on itchio easier maybe idk
SRC = $(wildcard src/*.cpp) $(wildcard src/graphics/*.cpp) $(wildcard src/engine/*.cpp) $(wildcard src/variants/*.cpp)
INC = -Isrc -Iinclude -Iinclude/graphics -Iinclude/engine -Iinclude/variants -I"C:/raylib/raylib/src" 

BUILD_DIR = build

EMFLAGS = -o $(BUILD_DIR)/index.html -L"C:/raylib/raylib/src" -lraylib -flto -DPLATFORM_WEB -sUSE_GLFW=3 -sASYNCIFY -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS="['HEAPF32']" --shell-file "C:/raylib/raylib/src/shell.html" #--preload-file assets
echo_color = powershell -Command "Write-Host $(1) -ForegroundColor $(2)"

.PHONY: all setup build inject run clean

all: setup build inject run

setup:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

build:
	@echo ===============================================================================================
	@$(call echo_color, "[1/3] Emscripten is compiling da stuff", "Cyan")
	emcc $(SRC) $(INC) $(EMFLAGS)
	@echo ===============================================================================================

inject:
	@echo ===============================================================================================
	@$(call echo_color, "[2/3] Adding the css because the emscripten gui is UGLY", "Cyan")
	powershell -Command "Add-Content $(BUILD_DIR)/index.html '<style>#header { display: none !important; } body, html { margin: 0; padding: 0; background: #111; display: flex; justify-content: center; align-items: center; height: 100vh; overflow: hidden; } canvas { width: 1280px !important; height: 720px !important; box-shadow: 0 10px 30px rgba(0,0,0,0.5); display: block; } #output { width: 300px; height: 720px; resize: none; }</style><textarea id=\"output\"></textarea><script>function fixSize(){var c=document.getElementById(\"canvas\");if(c){c.width=1280;c.height=720;if(typeof GL!==\"undefined\"&&GL.currentContext){GL.currentContext.gl.viewport(0,0,1280,720);}}}window.addEventListener(\"resize\",fixSize);setInterval(fixSize,200);</script>'"
	@echo ===============================================================================================

run:
	@echo ===============================================================================================
	@$(call echo_color, "[3/3] Launching build/index.html", "Green")
	emrun $(BUILD_DIR)/index.html
	@echo ===============================================================================================

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)