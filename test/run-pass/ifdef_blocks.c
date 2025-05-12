#ifdef NOT_DEFINED
#error Should never happen 1
#elif 1 - 1
#error Should never happen 2
#elif 0
#error Should never happen 3
#endif

#if 1
#define RESULT 0
#elif 1
#error Should never happen 4
#else
#error Should never happen 5
#endif

int main() {
	return RESULT;
}

