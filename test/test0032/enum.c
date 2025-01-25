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

enum specific_no_trailing_comma {
	SNTC_ZERO = 0,
	SNTC_ONE = 1
};
enum specific_trailing_comma {
	STC_ZERO = 0,
	STC_ONE = 1,
};

enum automatic_no_trailing_comma {
	ANTC_ZERO,
	ANTC_ONE
};
enum automatic_trailing_comma {
	ATC_ZERO,
	ATC_ONE,
};

enum specific_and_automatic_no_trailing_comma {
	SAANTC_TEN = 10,
	SAANTC_ELEVEN,
	SAANTC_ONE = 1,
	SAANTC_TWO
};
enum specific_and_automatic_trailing_comma {
	SAATC_TEN = 10,
	SAATC_ELEVEN,
	SAATC_ONE = 1,
	SAATC_TWO,
};

enum {
	ANONYMOUS_ZERO,
	ANONYMOUS_ONE,
};

typedef enum specific_trailing_comma SpecificTrailingComma;

struct contains_enum {
	enum specific_trailing_comma specificTrailingComma;
	SpecificTrailingComma specificTrailingComma1;
};

struct contains_enum_pointer {
	enum specific_trailing_comma* specificTrailingComma;
	SpecificTrailingComma* specificTrailingComma1;
};

struct contains_enum_pointer_pointer {
	enum specific_trailing_comma** specificTrailingComma;
	SpecificTrailingComma** specificTrailingComma1;
};

int tests_enum(enum specific_trailing_comma specificTrailingComma, SpecificTrailingComma specificTrailingComma1) {
	if (specificTrailingComma != 0) {
		return 1;
	}

	if (specificTrailingComma1 != 0) {
		return 1;
	}

	return 0;
}

int tests_enum_pointer_pointers(enum specific_trailing_comma** specificTrailingComma, SpecificTrailingComma** specificTrailingComma1) {
	if (**specificTrailingComma != 0) {
		return 1;
	}

	if (**specificTrailingComma1 != 0) {
		return 1;
	}

	return 0;
}

int tests_enum_pointers(enum specific_trailing_comma* specificTrailingComma, SpecificTrailingComma* specificTrailingComma1) {
	if (*specificTrailingComma != 0) {
		return 1;
	}

	if (*specificTrailingComma1 != 0) {
		return 1;
	}

	if(tests_enum_pointer_pointers(&specificTrailingComma, &specificTrailingComma1)) {
		return 1;
	}

	return 0;
}

/* Works for some architectures
 * https://github.com/oriansj/M2-Planet/issues/63
 * int ten = SAATC_TEN;
 * enum specific_and_automatic_no_trailing_comma ten_enum = SAANTC_TEN;
 */

int main() {
	if(SNTC_ZERO != 0) {
		return 1;
	}

	if (SNTC_ONE != 1) {
		return 1;
	}

	if(STC_ZERO != 0) {
		return 1;
	}

	if (STC_ONE != 1) {
		return 1;
	}

	if(ANTC_ZERO != 0) {
		return 1;
	}

	if (ANTC_ONE != 1) {
		return 1;
	}

	if(ATC_ZERO != 0) {
		return 1;
	}

	if (ATC_ONE != 1) {
		return 1;
	}

	if(SAANTC_TEN != 10) {
		return 1;
	}

	if(SAANTC_ELEVEN != 11) {
		return 1;
	}

	if (SAANTC_ONE != 1) {
		return 1;
	}

	if (SAATC_TWO != 2) {
		return 1;
	}

	if(SAATC_TEN != 10) {
		return 1;
	}

	if(SAATC_ELEVEN != 11) {
		return 1;
	}

	if (SAATC_ONE != 1) {
		return 1;
	}

	if (SAATC_TWO != 2) {
		return 1;
	}

	if (sizeof(enum specific_trailing_comma) != sizeof(int))  {
		return 1;
	}

	if (sizeof(SpecificTrailingComma) != sizeof(enum specific_trailing_comma))  {
		return 1;
	}

	SpecificTrailingComma typedefVariable = STC_ZERO;

	int return_value = STC_ZERO;

	if(typedefVariable != return_value) {
		return 1;
	}

	enum specific_trailing_comma specificTrailingComma1 = STC_ZERO;

	if (specificTrailingComma1 != return_value) {
		return 1;
	}

	struct contains_enum containsEnum;
	containsEnum.specificTrailingComma1 = STC_ZERO;
	containsEnum.specificTrailingComma = STC_ZERO;

	if (containsEnum.specificTrailingComma != 0) {
		return 1;
	}

	if (containsEnum.specificTrailingComma1 != 0) {
		return 1;
	}

	enum specific_trailing_comma* specificTrailingCommaPointer = &specificTrailingComma1;
	if(*specificTrailingCommaPointer != 0) {
		return 1;
	}
	if(sizeof(enum specific_trailing_comma*) != sizeof(int*)) {
		return 1;
	}

	struct contains_enum_pointer containsEnumPointer;
	containsEnumPointer.specificTrailingComma = &containsEnum.specificTrailingComma;
	containsEnumPointer.specificTrailingComma1 = &containsEnum.specificTrailingComma1;

	if(*containsEnumPointer.specificTrailingComma != 0) {
		return 1;
	}
	if(*containsEnumPointer.specificTrailingComma1 != 0) {
		return 1;
	}

	enum specific_trailing_comma** specificTrailingCommaPointerPointer = &specificTrailingCommaPointer;
	if(**specificTrailingCommaPointerPointer != 0) {
		return 1;
	}
	if(sizeof(enum specific_trailing_comma**) != sizeof(int**)) {
		return 1;
	}

	struct contains_enum_pointer_pointer containsEnumPointerPointer;
	containsEnumPointerPointer.specificTrailingComma = &containsEnumPointer.specificTrailingComma;
	containsEnumPointerPointer.specificTrailingComma1 = &containsEnumPointer.specificTrailingComma1;

	if(**containsEnumPointerPointer.specificTrailingComma != 0) {
		return 1;
	}
	if(**containsEnumPointerPointer.specificTrailingComma1 != 0) {
		return 1;
	}

	if(tests_enum(specificTrailingComma1, typedefVariable)) {
		return 1;
	}

	if(tests_enum_pointers(&specificTrailingComma1, &typedefVariable)) {
		return 1;
	}

	if(ANONYMOUS_ZERO != 0){
		return 1;
	}

	if(ANONYMOUS_ONE != 1){
		return 1;
	}

	/* See comment near ten and ten_enum
	if(ten != 10) {
		return 1;
	}

	if(ten_enum != 10) {
		return 1;
	}
	 */

	return return_value;
}
