
pi.out: pi.cpp
	# g++ -march=native -Ofast -funroll-loops -fomit-frame-pointer -fno-exceptions -fno-rtti -o $@ $<
	g++ -pthread -static -W -O9 -funroll-all-loops -finline -ffast-math -o $@ $<
	chmod +x $@