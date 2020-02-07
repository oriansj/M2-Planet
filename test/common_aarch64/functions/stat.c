/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2020 deesix <deesix@tuta.io>
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

/*
 * chmod() changes the mode of the file specified whose pathname is given in
 *  pathname, which is dereferenced if it is a symbolic link.
 * fchmod() changes the mode of the file referred to by the open file
 *  descriptor fd.
 * The new file mode is specified in mode, which is a bit mask created by
 *  ORing together zero or more of the following:
 * S_ISUID  (04000)  set-user-ID (set process effective user ID on execve(2))
 * S_ISGID  (02000)  set-group-ID (set process effective group ID on execve(2)
 *  mandatory locking, as described in fcntl(2); take a new file's group from
 *  parent directory, as described in chown(2) and mkdir(2))
 * S_ISVTX  (01000)  sticky bit (restricted deletion flag, as described in
 *  unlink(2))
 * S_IRUSR  (00400)  read by owner
 * S_IWUSR  (00200)  write by owner
 * S_IXUSR  (00100)  execute/search by owner ("search" applies for directories
 *  , and means that entries within the directory can be accessed)
 * S_IRGRP  (00040)  read by group
 * S_IWGRP  (00020)  write by group
 * S_IXGRP  (00010)  execute/search by group
 * S_IROTH  (00004)  read by others
 * S_IWOTH  (00002)  write by others
 * S_IXOTH  (00001)  execute/search by others
 */

int chmod(char *pathname, int mode)
{
	asm("SET_X0_FROM_BP" "SUB_X0_16" "DEREF_X0"
	    "SET_X2_FROM_X0"
	    "SET_X0_FROM_BP" "SUB_X0_8" "DEREF_X0"
	    "SET_X1_FROM_X0"
	    "SET_X0_TO_0"
	    "SET_X3_FROM_X0"
	    "SET_X0_TO_FCNTL_H_AT_FDCWD"
	    "SET_X8_TO_SYS_FCHMODAT"
	    "SYSCALL");
}
