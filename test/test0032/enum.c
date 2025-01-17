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

	int return_value = STC_ZERO;

	return return_value;
}
