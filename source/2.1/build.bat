gcc p2pmsgr.c chat.o icons.res -o p2pmsgr.exe -m32 -lws2_32 -lgdi32 -lwinmm -mwindows
gcc stopsrv.c -o stopsrv.exe -m32 -mwindows -lwinmm