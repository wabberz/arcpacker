arc:
	mkdir -p build
	g++ -Wall -Wextra -m32 -s -Os arc.cpp arc_packer.cpp fs_helper.cpp zlib_helper.cpp globals.cpp -o build/arc -lz
	#g++ -Wall -Wextra -m64 -s -Os arc.cpp arc_packer.cpp fs_helper.cpp zlib_helper.cpp globals.cpp -o build/arc64 -lz
	w32-g++ -Wall -Wextra -m32 -s -Os arc.cpp arc_packer.cpp fs_helper.cpp zlib_helper.cpp globals.cpp -o build/arc.exe -lz -static
	#w64-g++ -Wall -Wextra -m64 -s -Os arc.cpp arc_packer.cpp fs_helper.cpp zlib_helper.cpp globals.cpp -o build/arc64.exe -lz -static

	#upx --best --ultra-brute build/arc
	#upx --best --ultra-brute build/arc64
	#upx --best --ultra-brute build/arc.exe
	#upx --best --ultra-brute build/arc64.exe

install: arc
	install -o root -g root -m 755 build/arc /usr/bin/

uninstall:
	rm -rf /usr/bin/arc

clean:
	rm -f build/arc
	rm -f build/arc64
	rm -f build/arcU
	rm -f build/arc64U
	rm -f build/arc.exe
	rm -f build/arcU.exe
	rm -f build/arc64.exe
	rm -f build/arc64U.exe
	#rm -f -r build
