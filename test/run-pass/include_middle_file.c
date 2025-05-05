
int h() { return 1; }

#include "include_file_f.h"

int main() {
	return f() - h();
}

