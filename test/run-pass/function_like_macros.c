#define INT_INIT(a, b, c) int a, b, c
#define PASSTHROUGH(a) a

int main() {
	INT_INIT(* ptr = 0, regular, initialized = 0);
	ptr = &regular;
	regular = 1;
	initialized = regular;

	PASSTHROUGH(const int* a = ptr);
	PASSTHROUGH(const char* c = 0;)
	PASSTHROUGH(const char* const str = "this is my string");

	return *a - 1;
}

