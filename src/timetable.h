/* 
 * timetable_creator is timetable-creating tool
 * 
 * Copyright (C) 2013 Matěj Nikl
 * 
 *
 * This file is part of timetable_creator
 * 
 * timetable_creator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * timetable_creator distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef TIMETABLE_H
#define	TIMETABLE_H

#include "structs.h"

void    timetable_print(struct timetable *t, FILE *stream);
inline void chars_print(char chr, int n, FILE *stream);
inline void  item_print(char *s, int space, FILE *stream);

int  timetable_create(struct timetable *t, FILE **stream);
int  timetable_eval(struct timetable *t);

int  utf8_strlen(char *s);

inline void parallel_place(struct parallel *l, struct timetable *t);
inline void parallel_remove(struct parallel *l, struct timetable *t);
inline int  parallel_check(struct parallel *l, struct timetable *t);

#endif	/* TIMETABLE_H */

