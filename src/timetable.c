/* 
 * timetable_creator is timetable-creating tool
 * 
 * Copyright (C) 2013 Matěj Nikl
 * Copyright (C) 2013 Přemysl Janouch
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

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "structs.h"
#include "timetable.h"

#include "gettext.h"
#define _(string) gettext (string)
#define N_(string) string

#define MIN_LEN 11
void timetable_print(struct timetable *t, FILE *stream) {
	int i, j, k, width, height, total_width, space, prev_same, head_width;
	struct parallel ***par = t->timetable;
	/*char *times[] = { "7:30 - 8:15", "8:15 - 9:00", "9:15 -10:00", "10:00-10:45", "11:00-11:45",
	"11:45-12:30", "12:45-13:30", "13:30-14:15", "14:30-15:15", "15:15-16:00", "16:15-17:00",
	"17:00-17:45", "18:00-18:45", "18:45-19:30", "19:45-20:30" }; 
	/*/
	char *times[] = { "7:30   -   9:00", " 9:15   -   10:45", "11:00   -   12:30",
	"12:45   -   14:15", "14:30   -   16:00", "16:15   -   17:45",
	"18:00   -   19:30", "19:45-20:30" };
	/**/
	char *days[] = { N_("Po - S"), N_("Po - L"), N_("Ut - S"), N_("Ut - L"),
	                 N_("St - S"), N_("St - L"), N_("Ct - S"), N_("Ct - L"), N_("Pa - S"), N_("Pa - L") };
	char *numbers[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11",
	"12", "13", "14", "15" };
	
	width = MIN_LEN;
	head_width = 8;
	
	for (i = 0; i < t->n_subjects; i++) {
		t->subscribe[i] = NULL;
	}

	for (i = 0; i < DAYS; i++) {
		for (j = 0; j < LESSONSP; j++) {
			if (par[i][j]) {
				int len  = utf8_strlen(par[i][j]->subj_name);
				int len2 = utf8_strlen(par[i][j]->teacher);
				len = len > len2 ? len : len2;
				if (len > width) {
					int p = 1;
					while (par[i][j] == par[i][j + p]) {
						p++;
					}
					
					len /= p;
					if (len > width) {
						width = len;
					}
					j += p - 1;
				}
				
				if (t->subscribe[par[i][j]->index]) {
					if (par[i][j]->type > t->subscribe[par[i][j]->index]->type) {
						t->subscribe[par[i][j]->index] = par[i][j];
					}
				} else {
					t->subscribe[par[i][j]->index] = par[i][j];
				}
			}
		}
	}
	
	height = 3;
	total_width = width * LESSONS + head_width + LESSONSPP + 1;
	
	chars_print('-', total_width, stream);
	fputc('\n', stream);
	
	for (i = 0; i < height; i++) {
		fputc('|', stream);
		chars_print(' ', head_width, stream);
		chars_print('|', 2, stream);
		for (j = 0; j < LESSONS; j++) {
			if (i == 1) {
				item_print(numbers[j], width, stream);
			} else if (i == 2) {
				if (j < 7) {
					item_print(times[j], width * 2 + 1, stream);
				} else {
					item_print(times[j], width, stream);
					fputc('|', stream);
					break;
				}
			} else {
				chars_print(' ', width, stream);
			}
			fputc('|', stream);
		}
		fputc('\n', stream);
	}
	chars_print('=', total_width, stream);
	fputc('\n', stream);
	
	for (i = 0; i < DAYS; i++) {					/* for each day */
		for (j = 0; j < height; j++) {
			fputc('|', stream);
			if (j == 1) {
				item_print(gettext (days[i]), head_width, stream);
			} else {
				chars_print(' ', head_width, stream);
			}
			chars_print('|', 2, stream);
			prev_same = 1;
			for (k = 1; k < LESSONSP; k++) {			/* for each lesson */
				if (par[i][k]) {
					if (par[i][k] == par[i][k + 1]) {
						prev_same++;
						continue;
					} else {
						space = width * prev_same + prev_same - 1;
						prev_same = 1;
					}
					if (j == 0) {				/* print subj name */
						item_print(par[i][k]->subj_name, space, stream);
					} else if (j == 1) {		/* print lesson id */
						item_print(par[i][k]->id, space, stream);
					} else if (j == 2) {		/* print teacher name */
						item_print(par[i][k]->teacher, space, stream);
					} else {
						chars_print(' ', space, stream);
					}
				} else {
					chars_print(' ', width, stream);
				}
				fputc('|', stream);
			}
			fputc('\n', stream);
		}
		
		chars_print('-', total_width, stream);
		fputc('\n', stream);
	}
	
	fprintf(stream, _("Rating: %d\n\n"), t->best_rat);
	fprintf(stream, _("Subscribe to:\n"));

	for (i = 0; i < t->n_subjects; i++) {
		fprintf(stream, "%s: %s\n", t->subscribe[i]->subj_name, t->subscribe[i]->id);
	}
	
	fprintf(stream, "\n\n");
}

inline void chars_print(char chr, int n, FILE *stream) {
	for ( ; n > 0; n--) {
		fputc(chr, stream);
	}
}

inline void item_print(char *s, int space, FILE *stream) {
	int len, white;
	len = utf8_strlen(s);
	white = (space - len) / 2;
	chars_print(' ', white, stream);
	fprintf(stream, "%s", s);
	chars_print(' ', space - (white + len), stream);
}

int timetable_create(struct timetable *t, FILE **stream) {
	struct subject *s;
	struct parallel *p;
	int i, pos;
	
	t->n_combinations = 0;
	s = t->subjects;
	pos = 0;	
	
	while (1) {
		
		while (s->curr_type < s->end_type && s->pos < s->n_parallels) {
			p = s->parallels + s->pos++;
			if (p->type <= s->curr_type + 1) {
				for (i = s->curr_type; i >= p->type ; i--) {
					parallel_remove(s->used_parallels[i], t);
					s->curr_type--;
				}
				if (parallel_check(p, t)) {
					parallel_place(p, t);
					s->used_parallels[p->type] = p;
					s->curr_type++;
				}
			}
		}
		
		if (s->curr_type == s->end_type) {	/* subject placed, move forward */
			if (pos + 1 == t->n_subjects) {		/* currently on last subject = found solution */
				int rat = timetable_eval(t);
				if (rat > t->best_rat) {
					t->best_rat = rat;
					*stream = freopen(t->out_path, "wt", *stream);
					if (! *stream) {
						fprintf(stderr, _("An error occurred while re-opening file '%s' for writing: %s!\n"), t->out_path, strerror(errno));
						return 0;
					}
					timetable_print(t, *stream);
					t->n_solutions = 1;
				} else if (rat == t->best_rat) {
					timetable_print(t, *stream);
					t->n_solutions++;
				}
				parallel_remove(s->used_parallels[s->curr_type--], t);
				t->n_combinations++;
				
			} else {
				s = t->subjects + ++pos;
				s->curr_type = s->start_type;
				s->pos = 0;
			}
		} else {				/* lesson could not be placed */
			if (pos > 0) {	/* move backward */
				for (i = s->curr_type; i > s->start_type; i--) {
					parallel_remove(s->used_parallels[i], t);
				}
				s = t->subjects + --pos;
				parallel_remove(s->used_parallels[s->curr_type--], t);
			} else {			/* every combination tested -> clean up */
				for (i = s->curr_type; i > s->start_type; i--) {
					parallel_remove(s->used_parallels[i], t);
				}
				s->curr_type = s->start_type;
				s->pos = 0;
				break;
			}
		}	
	}
	
	return 1;
}

int timetable_eval(struct timetable *t) {
	int i, j, total = 0, day_started, gaps, launch, in_row, free_day;
	struct parallel ***l = t->timetable;
	struct rating *rat = &(t->rat);
	
	for (i = 0; i < DAYS; i++) {
		day_started = gaps = launch = in_row = 0;
		free_day = 1;
		for (j = 1; j < LESSONSP; j++) {
			if (l[i][j]) {
				free_day = 0;
				day_started = 1;
				
				if (++in_row > t->rat.in_row_hours) {
					total += t->rat.minus.in_row;
				}
				
				if (gaps) {
					total += gaps * rat->minus.gap;
					gaps = 0;
				}
				if (rat->forbidden[i][j]) {
					total += rat->minus.forbidden_viol;
				}
				if (j <= rat->early) {
					total += rat->minus.early_hour;
				} else if (j >= rat->late) {
					total += rat->minus.late_hour;
				}
				
			} else {
				if (j >= rat->launch_begins && j <= rat->launch_ends && launch < rat->launch_hours) { /* launch time && no launch yet*/
					launch++;
					total += rat->plus.launch_hour;
				}
				if (day_started) {
					gaps++;
				}
				
				in_row = 0;
			}
		}
		if (free_day) {
			total += rat->plus.day_off;
		}
	}
	return total;
}

int utf8_strlen(char *s) {
	int count = 0;
	char *sBefore = s;
	
	while (*s > 0) {
ascii:
		s++;
	}

	count += s - sBefore;

	while (*s) {
		if (*s > 0) {
			sBefore = s;
			goto ascii;
		} else {
			switch (*s & 0xF0) {
				case 0xE0:
					s += 3;
					break;
				case 0xF0:
					s += 4;
					break;
				default:
					s += 2;
					break;
			}
		}

		count++;
	}
	return count;
}

inline void parallel_place(struct parallel *p, struct timetable *t) {
	int i, j;
	
	for (i = 0; i < p->n_lessons; i++) {
		for (j = p->lesson[i].start; j <= p->lesson[i].end; j++) {
			t->timetable[p->lesson[i].day][j] = p;
		}
	}
}

inline void parallel_remove(struct parallel *p, struct timetable *t) {
	int i, j;

	for (i = 0; i < p->n_lessons; i++) {
		for (j = p->lesson[i].start; j <= p->lesson[i].end; j++) {
			t->timetable[p->lesson[i].day][j] = NULL;
		}
	}
}

inline int parallel_check(struct parallel *p, struct timetable *t) {
	int i, j;
	
	for (i = 0; i < p->n_lessons; i++) {
		for (j = p->lesson[i].start; j <= p->lesson[i].end; j++) {
			if (t->timetable[p->lesson[i].day][j]) {
				return 0;
			}
		}
	}
	return 1;
}
