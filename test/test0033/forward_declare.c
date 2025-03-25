/* Copyright (C) 2024 Gtker
 * This file is part of M2-Planet.
 *
 * M2-Planet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * M2-Planet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.
 */

/* We should be able to have pointers to types that aren't declared. */
struct HasPointers {
	struct StructDoesntExist* struct_doesnt_exist;
	union UnionDoesntExist* union_doesnt_exist;
};

struct forward;

typedef struct forward TypedefForward;

struct forward* global_fwd_pointer;
TypedefForward* global_typedef;

int use_forward(struct forward* arg);
int forward_double(struct forward** arg);

int forward_pointer(struct forward* arg) {
	struct forward* ptr = arg;

	if(sizeof(struct forward*) != sizeof(void*)) {
		return 1;
	}

	if(forward_double(&arg) != 0) {
		return 1;
	}

	return use_forward(ptr);
}

int use_forward_typedef(TypedefForward* arg);
int forward_double_typedef(TypedefForward** arg);

int forward_pointer_typedef(TypedefForward* arg) {
	struct forward* ptr = arg;

	if(sizeof(struct forward*) != sizeof(void*)) {
		return 1;
	}

	if(forward_double_typedef(&arg) != 0) {
		return 1;
	}

	return use_forward(ptr);
}
struct forward {
	int a;
};

int forward_double(struct forward** arg) {
	if((*arg)->a != 0xDEADBEEF) {
		return 1;
	}
	return 0;
}

typedef struct forward TypedefForward;

int forward_double_typedef(TypedefForward** arg) {
	if((*arg)->a != 0xDEADBEEF) {
		return 1;
	}
	return 0;
}

int use_value(struct forward arg) {
	if (arg.a != 0xDEADBEEF) {
		return 1;
	}
	return 0;
}

int use_forward(struct forward* arg) {
	if(arg->a != 0xDEADBEEF) {
		return 1;
	}
	if(use_value(*arg) != 0) {
		return 1;
	}
	return 0;
}

int use_value_typedef(TypedefForward arg) {
	if (arg.a != 0xDEADBEEF) {
		return 1;
	}
	return 0;
}

int use_forward_typedef(TypedefForward* arg) {
	if(arg->a != 0xDEADBEEF) {
		return 1;
	}
	if(use_value_typedef(*arg) != 0) {
		return 1;
	}
	return 0;
}

struct contains_forward {
	struct forward fwd;
};

struct contains_forward_typedef {
	TypedefForward fwd;
};

struct forward global_fwd;
TypedefForward global_fwd_typedef;

int main(void) {
	struct forward fwd;

	if (sizeof(struct forward) != sizeof(int)) {
		return 1;
	}

	if (sizeof(struct contains_forward) != sizeof(struct forward)) {
		return 1;
	}

	fwd.a = 0xDEADBEEF;
	global_fwd.a = 0xDEADBEEF;
	global_fwd_pointer = &global_fwd;

	if(forward_pointer(&global_fwd) != 0) {
		return 1;
	}

	if(forward_pointer(global_fwd_pointer) != 0) {
		return 1;
	}

	TypedefForward fwd_typedef;

	if(sizeof(TypedefForward) != sizeof(int)) {
		return 1;
	}

	if(sizeof(struct contains_forward) != sizeof(TypedefForward)) {
		return 1;
	}

	fwd_typedef.a = 0xDEADBEEF;
	global_fwd_typedef.a = 0xDEADBEEF;
	global_fwd_pointer = &global_fwd;

	if(forward_pointer_typedef(&global_fwd_typedef) != 0) {
		return 1;
	}

	if(forward_pointer_typedef(global_fwd_pointer) != 0) {
		return 1;
	}

	if(forward_pointer(&fwd) != 0) {
		return 1;
	}

	return use_forward_typedef(&fwd_typedef);
}
