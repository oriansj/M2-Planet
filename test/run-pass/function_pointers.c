
int function_called = 0xff;

void set0() {
	function_called = 0;
}

void set1() {
	function_called = 1;
}

void(*global)(void) = &set0;

struct T {
	void (*f)(void);
};

struct T global_t = { &set0 };

int main() {
	void(*f)(void) = set0;

	if(function_called != 0xff) return 1;
	f();
	if(function_called != 0) return 2;
	f = set1;
	f();
	if(function_called != 1) return 3;

	f = set0;
	(f)();
	if(function_called != 0) return 4;
	f = set1;
	(f)();
	if(function_called != 1) return 5;

	f = set0;
	(*f)();
	if(function_called != 0) return 6;
	f = set1;
	(*f)();
	if(function_called != 1) return 7;

	f = set0;
	(*****f)();
	if(function_called != 0) return 8;
	f = set1;
	(*****f)();
	if(function_called != 1) return 9;

	global();
	if(function_called != 0) return 10;
	global = set1;
	global();
	if(function_called != 1) return 11;

	struct T t;
	t.f = set0;
	t.f();
	if(function_called != 0) return 12;
	t.f = set1;
	t.f();
	if(function_called != 1) return 13;

	global_t.f();
	if(function_called != 0) return 14;
	global_t.f = set1;
	global_t.f();
	if(function_called != 1) return 15;
}
