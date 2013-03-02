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

#ifndef STRUCTS_H
#define	STRUCTS_H

#define DAYS 10 /* = 5 * 2 (odd and even) */
#define LESSONS 15
#define LESSONSP 16
#define LESSONSPP 17
#define L_TYPES 3 /* lesson types */

enum days {
	mon_e, mon_o, tue_e, tue_o, wed_e, wed_o, thu_e, thu_o, fri_e, fri_o
};

enum types {
	pr, cv, lab
};

struct lesson {
	int start, end;
	int day;
};

struct parallel {
	char *subj_name;
	char id[16];
	char teacher[256];
	int usable, type;
	int n_lessons;
	int index;
	struct lesson lesson[DAYS];
};

struct subject {
	char *name;
	struct parallel *parallels;
	struct parallel *used_parallels[L_TYPES];
	int start_type, end_type, curr_type;
	int n_parallels, pos;
};

struct rating {
	int forbidden[DAYS][LESSONSP];
	int launch_hours, launch_begins, launch_ends;
	int early, late;
	int in_row_hours;
	int allow_nonfree;
	struct {
		int forbidden_viol, in_row;
		int gap, early_hour, late_hour;
	} minus;
	struct {
		int launch_hour;
		int day_off;
	} plus;
};

struct timetable {
	struct subject *subjects;
	int n_subjects;
	struct parallel ***timetable, **subscribe;
	struct rating rat;
	char *in_path;
	char *out_path;
	int best_rat, n_solutions;
	unsigned long n_combinations;
};

void   subject_init(struct subject *s);
void   subject_free(struct subject *s);
void timetable_init(struct timetable *t);
void timetable_free(struct timetable *t);
void  parallel_init(struct parallel *l);
void    rating_init(struct rating *r);

#endif	/* STRUCTS_H */

