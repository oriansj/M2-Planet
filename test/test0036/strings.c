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

int main() {
    if('\a' != 7)  return 1;
    if('\b' != 8)  return 2;
    if('\e' != 27) return 3;
    if('\f' != 12) return 4;
    if('\n' != 10) return 5;
    if('\t' != 9)  return 6;
    if('\v' != 11) return 7;
    if('\\' != 92) return 8;
    if('\'' != 39) return 9;
    if('\"' != 34) return 10;
    //if('\?' != 63) return 11;

    char* e = "\a\b\e\f\n\t\v\\\'\"?A";
    if(e[0] != 7)   return 12;
    if(e[1] != 8)   return 13;
    if(e[2] != 27)  return 14;
    if(e[3] != 12)  return 15;
    if(e[4] != 10)  return 16;
    if(e[5] != 9)   return 17;
    if(e[6] != 11)  return 18;
    if(e[7] != 92)  return 19;
    if(e[8] != 39)  return 20;
    if(e[9] != 34)  return 21;
    //if(e[10] != 63) return 22;
    if(e[11] != 65) return 23;

    if('\177' != 127)  return 25;
    if('\200' != -128) return 25;
    if('\077' != 63)   return 26;
    if('\007' != 7)    return 27;
    if('\0'   != 0)    return 28;
    if('\1'   != 1)    return 29;
    if('\01'  != 1)    return 30;
    if('\00'  != 0)    return 31;
    if('\001' != 1)    return 32;
    if('\000' != 0)    return 33;
    if('\236' != -98)  return 34;
    if('\333' != -37)  return 35;
    if('\363' != -13)  return 36;
    if('\377' != -1)   return 37;
    if('\400' != 0)    return 38;
    if('\401' != 1)    return 39;
    if('\600' != -128) return 40;
    if('\776' != -2)   return 41;
    if('\777' != -1)   return 42;

    char* o = "\177\200\077\007\0\1\01\00\001\00\236\333\363\377\400\401\600\776\777";
    if(o[0]  != 127)  return 43;
    if(o[1]  != -128) return 44;
    if(o[2]  != 63)   return 45;
    if(o[3]  != 7)    return 46;
    if(o[4]  != 0)    return 47;
    if(o[5]  != 1)    return 48;
    if(o[6]  != 1)    return 49;
    if(o[7]  != 0)    return 50;
    if(o[8]  != 1)    return 51;
    if(o[9]  != 0)    return 52;
    if(o[10] != -98)  return 53;
    if(o[11] != -37)  return 54;
    if(o[12] != -13)  return 55;
    if(o[13] != -1)   return 56;
    if(o[14] != 0)    return 57;
    if(o[15] != 1)    return 58;
    if(o[16] != -128) return 59;
    if(o[17] != -2)   return 60;
    if(o[18] != -1)   return 61;

    char* a = "A\101\501";
    if(a[0] != 65)  return 62;
    if(a[1] != 65)  return 63;
    if(a[2] != 65)  return 64;

    char* b = "\0 \0001\008\08\778\78";
    if(b[0]  != 0)   return 65;
    if(b[1]  != 32)  return 66;
    if(b[2]  != 0)   return 67;
    if(b[3]  != 49)  return 68;
    if(b[4]  != 0)   return 69;
    if(b[5]  != 56)  return 70;
    if(b[6]  != 0)   return 71;
    if(b[7]  != 56)  return 72;
    if(b[8]  != 63)  return 73;
    if(b[9]  != 56)  return 74;
    if(b[10] != 7)   return 75;
    if(b[11] != 56)  return 76;

    return 0;
}
