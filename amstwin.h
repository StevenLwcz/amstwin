void cls(int s);
void paper(int stream, int p);
void pen(int stream, int p);
void new_window(int stream, int left, int right, int top, int bottom);
void init_window();
void locate_stream(int s, int x, int y);
void locate(int x, int y); // not used?
void print_0(char *buf);
void end_window();
void print_stream_cr(int stream, char *buf, bool cr);


