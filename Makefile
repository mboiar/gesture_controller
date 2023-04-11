CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs ~/build/lib`

% : %.cpp
        g++ $(CFLAGS) -o $@ $< $(LIBS)