loom:


# 1. Compile Video
	gcc -I. -I/usr/include/efi -std=gnu17 -D_GNU_SOURCE -fpermissive -c efi/i_video.c -o linux/i_video.o

# 2. Compile Stubs (Ensure stubs.c has no I_ functions used above)
	gcc -c stubs.c -o linux/stubs.o

# 3. Link
	gcc -no-pie linux/*.o -o linux/loom -lm -lX11 -lXext -Wl,-z,muldefs

# 4. Play
#./linux/linuxxdoom -iwad doom.wad
