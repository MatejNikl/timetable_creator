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

#include <stdlib.h>
#include "structs.h"

void timetable_init(struct timetable *t) {
	int i, j;
	t->timetable = (struct parallel ***) malloc(DAYS * sizeof(struct parallel **));
	
	for (i = 0; i < DAYS; i++) {
		t->timetable[i] = (struct parallel **) malloc(LESSONSPP * sizeof(struct parallel *));
		
		for (j = 0; j < LESSONSPP; j++) {
			t->timetable[i][j] = NULL;
		}
	}
	
	rating_init(&(t->rat));
	
	t->n_combinations = 0;
	t->subjects       = NULL;
	t->out_path       = NULL;
	t->in_path        = NULL;
	t->subscribe      = NULL;
	t->n_subjects     = 0;
	t->best_rat       = 1 << (sizeof(t->best_rat) * 8 - 1);
	t->n_solutions    = 0;
}

void timetable_free(struct timetable *t) {
	int i;
	
	if (t->timetable) {
		for (i = 0; i < DAYS; i++) {
			if (t->timetable[i]) {
				free(t->timetable[i]);
			}
		}
		free(t->timetable);
	}
	
	if (t->subjects) {
		for (i = 0; i < t->n_subjects; i++) {
			subject_free(t->subjects + i);
		}

		free(t->subjects);
	}
	
	if (t->subscribe) {
		free(t->subscribe);
	}
	
	if (t->in_path) {
		free(t->in_path);
	}
	
	if (t->out_path) {
		free(t->out_path);
	}
}

void subject_init(struct subject *s) {
	int i;
	for (i = 0; i < L_TYPES; i++) {
		s->used_parallels[i] = NULL;
	}
	
	s->name        = NULL;
	s->parallels   = NULL;
	s->n_parallels = 0;
	s->pos         = 0;
	s->curr_type    = 0;
	s->end_type    = pr;
	s->start_type  = lab;
}

void subject_free(struct subject *s) {
	if (s->name) {
		free(s->name);
	}
	if (s->parallels) {
		free(s->parallels);
	}
}

void parallel_init(struct parallel *p) {
	int i;
	for (i = 0; i < DAYS; i++) {
		p->lesson[i].day   = 0;
		p->lesson[i].start = 0;
		p->lesson[i].end   = 0;
	}
	
	p->subj_name    = NULL;
	*(p->id)  = 0;
	*(p->teacher) = 0;
	p->type       = 0;
	p->usable     = 0;
	p->n_lessons  = 0;
	p->index      = 0;
}

void rating_init(struct rating *r) {
	int i, j;
	for (i = 0; i < DAYS; i++) {
		for (j = 0; j < LESSONSP; j++) {
			r->forbidden[i][j] = 0;
		}
	}
	
	r->launch_hours         = 1;
	r->launch_begins        = 5;
	r->launch_ends          = 9;
	r->early                = 2;
	r->late                 = 14;
	r->in_row_hours         = 4;
	r->allow_nonfree        = 0;
	
	r->minus.forbidden_viol = -2000;
	r->minus.in_row         = -350;
	r->minus.early_hour     = -500;
	r->minus.late_hour      = -250;
	r->minus.gap            = -250;
	
	r->plus.launch_hour     = 600;
	r->plus.day_off         = 1000;
}
