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

/* In order to work around https://github.com/oriansj/M2-Planet/issues/63
 * this files uses HIGHLY ILLEGAL undefined behavior by not initializing the statics
 * using the knowledge that M2-Planet automatically zero initializes them.
 * These should be zero initialized explicitly when the issue is fixed.
 */

int main(void) {
	/* Put something in the register to make sure 0 is deliberately loaded in. */
	if((1 + 1 + 2) != 4) {
		return 1;
	}
}
