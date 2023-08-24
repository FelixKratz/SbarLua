NAME=sketchybar
CFLAGS=-std=c99 -O3 -g -shared -fPIC
INSTALL_DIR=$(HOME)/.local/share/sketchybar_lua

LUA_DIR=lua-5.4.4
LIBS=-I$(LUA_DIR)/src -Lbin -llua

bin/$(NAME).so: src/$(NAME).c src/*.c bin/liblua.a
	clang $(CFLAGS) $^ $(LIBS) -o bin/$(NAME).so

install: bin/$(NAME).so | $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)
	mv bin/$(NAME).so $(INSTALL_DIR)

uninstall:
	rm -rf $(INSTALL_DIR)/$(NAME).so

clean:
	rm -rf bin
	cd $(LUA_DIR) && make clean

bin/liblua.a: | bin
	cd $(LUA_DIR) && make
	mv $(LUA_DIR)/src/liblua.a bin

bin:
	mkdir bin

$(INSTALL_DIR):
	mkdir -p $(INSTALL_DIR)
