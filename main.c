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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "structs.h"
#include "timetable.h"

void set_indexes(struct timetable *t);

int settings_read(struct timetable *t, char *path);
int  subject_read(struct subject *s, int allow_all, char *fn);

int   set_forbidden(int l[2], int i, int *p);
int parse_forbidden(char *line, int *p);

int   header_read(struct parallel *p, FILE *stream);
int     item_read_char(char *w, int n, FILE *stream);
int   item_read_int(int *w, FILE *stream);
ssize_t line_read(char **line, int *len, FILE *stream);
void    line_trim(char *line);
int     line_prepare(char *line, char **val);

void    name_copy(char *src, char **dst);
void   print_err(char *item, char *val);
FILE*  fopen_read(const char *path);
int compare(const void *p1, const void *p2);
int handle_args(int argc, char **argv, char **line, int *line_len);
void request_keypress(void);

#define DEF_PATH "settings.ini"

int main(int argc, char **argv) {
	char *line = NULL;
	int line_len = 0;
	
	FILE *in = NULL, *out = NULL;
	int size, retval = 0;

	struct timetable t;
	
	if (handle_args(argc, argv, &line, &line_len)) {
		request_keypress();
		return retval;
	}

	timetable_init(&t);
	if (settings_read(&t, line)) {
		in = fopen_read(t.in_path);

		if (in) { /* load timetables */
			size = 2;
			t.subjects = (struct subject *) malloc(size * sizeof(struct subject));
			while (line_read(&line, &line_len, in) > 0) {
				if (*line != '#') {
					if (subject_read(t.subjects + t.n_subjects, t.rat.allow_nonfree, line)) {
						if (++(t.n_subjects) == size) {
							size *= 2;
							t.subjects = (struct subject *) realloc(t.subjects, size * sizeof(struct subject));
						}
					} else {
						fprintf(stderr, "File '%s' does not contain any parallels!\n", line);
						subject_free(t.subjects + t.n_subjects);
					}
				}
			}
			
			fclose(in);
			if (line) { free(line); }	
			
			if (t.n_subjects) {
				out = fopen(t.out_path, "wt");
				if (out) {
					t.subscribe = (struct parallel **) malloc(t.n_subjects * sizeof(struct parallel *));
					set_indexes(&t);
					printf("Creating your timetable, please wait...\n");
					if (timetable_create(&t, &out)) {
						
						
						printf("There are %lu possible combinations and ", t.n_combinations);
						if (t.n_solutions > 0) {
							if (t.n_solutions > 1) {
								printf("%d best of them were ", t.n_solutions);
							} else {
								printf("the best of them was ");
							}
							printf("successfully written into file '%s'!\n", t.out_path);
						} else {
							printf("There was not found any non-colliding timetable!\n");
						}
					} else if (t.n_solutions) {
						printf("Some timetables (probably %d) were written into file '%s', \
								but an error occurred so they are (probably) not the best possible!\n", t.n_solutions, t.out_path);
					}
					
					if (out) {
						fclose(out);
					}
				} else {
					fprintf(stderr, "An error occurred while opening file '%s' for writing: %s!\n", t.out_path, strerror(errno));
					retval = 1;
				}
			} else {
				fprintf(stderr, "There are no valid subjects to create timetable from!\n");
			}
		} else {
			fprintf(stderr, "An error occurred while opening file '%s' for reading: %s!\n", t.in_path, strerror(errno));
			retval = 1;
		}
	} else {
		retval = 2;
	}
	
	timetable_free(&t);
	request_keypress();
	
	return retval;
}

void set_indexes(struct timetable *t) {
	int i, j;
	struct subject **subj;

	subj = (struct subject **) malloc(t->n_subjects * sizeof(struct subject *));
	
	for (i = 0; i < t->n_subjects; i++) {
		subj[i] = t->subjects + i;
	}
	
	qsort(subj, t->n_subjects, sizeof(struct subject *), compare);
	
	for (i = 0; i < t->n_subjects; i++) {
		for (j = 0; j < subj[i]->n_parallels; j++) {
			subj[i]->parallels[j].index = i;
		}
	}
	free(subj);
}

#define SET_ITEMS 26
int settings_read(struct timetable *t, char *path) {
	int n;
	char *line, *val;
	int line_len;
	FILE *file;
	
	const char *in =           "in_path";
	const char *out =          "out_path";
	const char *mon_E =        "Mon_e";
	const char *tue_E =        "Tue_e";
	const char *wed_E =        "Wed_e";
	const char *thu_E =        "Thu_e";
	const char *fri_E =        "Fri_e";
	const char *mon_O =        "Mon_o";
	const char *tue_O =        "Tue_o";
	const char *wed_O =        "Wed_o";
	const char *thu_O =        "Thu_o";
	const char *fri_O =        "Fri_o";
	const char *forb =         "forbidden_hour";
	const char *early_h =      "early_hour";
	const char *late_h =       "late_hour";
	const char *gap =          "gap";
	const char *in_row =       "in_row";
	const char *launch =       "launch_hour";
	const char *day =          "day_off";
	const char *in_row_hours = "in_row_hours";
	const char *launch_h =     "launch_hours";
	const char *launch_b =     "launch_begins";
	const char *launch_e =     "launch_ends";
	const char *early =        "early";
	const char *late =         "late";
	const char *nonfree =      "allow_nonfree";
	
	line = NULL;
	file = fopen_read(path);
	
	if (file) {
		n = line_len = 0;
		while (line_read(&line, &line_len, file) != -1) {
			if (line_prepare(line, &val)) {
				
				if (! strcmp(line, in)) {
					if (t->in_path) { free(t->in_path); }
					t->in_path = (char *) malloc(strlen(val) + 1);
					strcpy(t->in_path, val);
					n++;
				} else if (! strcmp(line, out)) {
					if (t->out_path) { free(t->out_path); }
					t->out_path = (char *) malloc(strlen(val) + 1);
					strcpy(t->out_path, val);
					n++;
				} else if (! strcmp(line, mon_E)) {
					if (parse_forbidden(val, t->rat.forbidden[mon_e])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, tue_E)) {
					if (parse_forbidden(val, t->rat.forbidden[tue_e])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, wed_E)) {
					if (parse_forbidden(val, t->rat.forbidden[wed_e])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, thu_E)) {
					if (parse_forbidden(val, t->rat.forbidden[thu_e])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, fri_E)) {
					if (parse_forbidden(val, t->rat.forbidden[fri_e])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, mon_O)) {
					if (parse_forbidden(val, t->rat.forbidden[mon_o])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, tue_O)) {
					if (parse_forbidden(val, t->rat.forbidden[tue_o])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, wed_O)) {
					if (parse_forbidden(val, t->rat.forbidden[wed_o])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, thu_O)) {
					if (parse_forbidden(val, t->rat.forbidden[thu_o])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, fri_O)) {
					if (parse_forbidden(val, t->rat.forbidden[fri_o])) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, forb)) {
					if (sscanf(val, "%d", &(t->rat.minus.forbidden_viol)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, early_h)) {
					if (sscanf(val, "%d", &(t->rat.minus.early_hour)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, late_h)) {
					if (sscanf(val, "%d", &(t->rat.minus.late_hour)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, gap)) {
					if (sscanf(val, "%d", &(t->rat.minus.gap)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, in_row)) {
					if (sscanf(val, "%d", &(t->rat.minus.in_row)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, launch)) {
					if (sscanf(val, "%d", &(t->rat.plus.launch_hour)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, day)) {
					if (sscanf(val, "%d", &(t->rat.plus.day_off)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, in_row_hours)) {
					if (sscanf(val, "%d", &(t->rat.in_row_hours)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, launch_h)) {
					if (sscanf(val, "%d", &(t->rat.launch_hours)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, launch_b)) {
					if (sscanf(val, "%d", &(t->rat.launch_begins)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, launch_e)) {
					if (sscanf(val, "%d", &(t->rat.launch_ends)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, early)) {
					if (sscanf(val, "%d", &(t->rat.early)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, late)) {
					if (sscanf(val, "%d", &(t->rat.late)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				} else if (! strcmp(line, nonfree)) {
					if (sscanf(val, "%d", &(t->rat.allow_nonfree)) == 1) {
						n++;
					} else {
						print_err(line, val);
					}
				}
			}
		}
		fclose(file);
	} else {
		fprintf(stderr, "An error occurred while opening settings file '%s': %s!\n", path, strerror(errno));
		return 0;
	}
	
	if (line) {
		free(line);
	}
	
	if (n == 0) {
		fprintf(stderr, "File '%s' is not valid settings file!\n", path);
		return 0;
	} else if (n < SET_ITEMS) {
		fprintf(stderr, "Not all settings were read from settings file '%s'!\n", path);
		return 0;
	} else if (n > SET_ITEMS) {
		fprintf(stderr, "Some settings were set more then once - from settings file '%s'!\n", path);
	}

	return 1;
}

int item_read_char(char *w, int n, FILE *stream) {
	int chr;
	char *start = w;
	
	if (w) { /* write mode */
		n--; /* for terminating '\0' */
		while ((chr = fgetc(stream)) != EOF && chr != '\t' && chr != '\n' && n) {
			*w++ = (char) chr;
			n--;
		}
		*w = '\0';
	} else { /* discard mode */
		while ((chr = fgetc(stream)) != EOF && chr != '\t' && chr != '\n')
			;
	}
	
	if (chr == '\n') {
		ungetc(chr, stream);
	}
	
	if (chr != EOF && (w != start || ! w)) {
		return 1;
	}
	
	return 0;
}

int item_read_int(int *w, FILE *stream) {
	int chr, used = 0;
	
	*w = 0;
	while ((chr = fgetc(stream)) != EOF && chr != '\t' && chr != '+' && chr != '\n') {
		if (isdigit(chr)) {
			used = 1;
			*w = (*w * 10) + (chr - '0');
		}
	}
	
	if (chr == '\n') {
		ungetc(chr, stream);
	}
	
	if (chr != EOF && used) {
		return chr;
	}
	
	return 0;
}

int header_read(struct parallel *p, FILE *stream) {
	int dummy;
	char *ptr;
	
	if (! item_read_char(p->id, sizeof(p->id), stream)) {
		return 0;
	}
	
	ptr = p->id;
	
	while (*ptr) {
		if (*ptr++ == '-') {
			p->type++;
		}
	}

	if (! item_read_char(NULL, 0, stream)) {
		return 0;
	}
	
	if (! item_read_char(p->teacher, sizeof(p->teacher), stream)) {
		return 0;
	}
	
	if (! item_read_int(&(p->usable), stream)) {
		return 0;
	}
	
	if (! item_read_int(&dummy, stream)) {
		return 0;
	}
	
	p->usable -= dummy;
	
	line_trim(p->id);
	line_trim(p->teacher);
	
	return 1;
}

#define islesson(X) ((X) > 0 && (X) < 16)
int subject_read(struct subject *s, int allow_nonfree, char *fn) {
	int i, n_parallels, chr;
	int size;
	struct parallel *p;
	FILE *file = NULL;
	
	subject_init(s);
	name_copy(fn, &(s->name));
	n_parallels = 0;
	
	file = fopen_read(fn);
	if (file) {
		size = 5;
		p = s->parallels = (struct parallel *) malloc(size * sizeof(struct parallel));	
		
		while (! feof(file)) {
			if ((chr = fgetc(file)) == '#') {
				while ((chr = fgetc(file)) != EOF && chr != '\n')	/* first char is # = discard line */
					;
				continue;
			} else {
				ungetc(chr, file);	/* first char is something else = put it bask */
			}
			parallel_init(p);
			
			/* TODO: inform about fails */
			if (header_read(p, file) && (p->usable || allow_nonfree)) {	/* succeeded to read header + parallel can be used */
				n_parallels++;
				
				/*
				 * in KOS are days listed in opposite order = mon_o, mon_e, tue_o, tue_e, ... = 1, 0, 3, 2, 5, ...
				 * so this i += i % 2 ? -1 : 3 does the job
				 */
				for (i = mon_o; i < DAYS; i += i % 2 ? -1 : 3) {
					if (item_read_int(&(p->lesson[p->n_lessons].start), file) == '+' && 
						item_read_int(&(p->lesson[p->n_lessons].end), file)   == '\t') {
						if (islesson(p->lesson[p->n_lessons].start) && islesson(p->lesson[p->n_lessons].end)
							&& p->lesson[p->n_lessons].start <= p->lesson[p->n_lessons].end) {
							p->lesson[p->n_lessons].day = i;
							p->n_lessons++;
						} else {
							fprintf(stderr, "%s, line %d: lesson starting at %d. and ending at %d. hour is not valid lesson!\n", s->name, n_parallels, p->lesson[p->n_lessons].start, p->lesson[p->n_lessons].end);
						}
					}
				}
				
				if (p->n_lessons) {
					p->subj_name = s->name;
					s->n_parallels++;

					if (s->end_type < p->type) {
						s->end_type = p->type;
					} else if (s->start_type > p->type) {
						s->start_type = p->type;
					}

					if (s->n_parallels == size) {
						size *= 2;
						s->parallels = (struct parallel *) realloc(s->parallels, size * sizeof(struct parallel));
					}
					p = s->parallels + s->n_parallels;
				}
			}
			while ((chr = fgetc(file)) != EOF && chr != '\n') /* discard leftovers, TODO: check if there is not another parallel */
				;
		}

		s->curr_type = --(s->start_type);
		fclose(file);
	} else {
		fprintf(stderr, "An error occurred while opening file '%s' for reading: %s!\n", fn, strerror(errno));
	}

	return s->n_parallels;
}

int set_forbidden(int l[2], int i, int *p) {
	if (i == 0) {
		if (islesson(l[0])) {
			p[l[0]] = 1;
		} else {
			return 0;
		}
	} else {
		if (islesson(l[0]) && islesson(l[1])) {
			int j;
			for (j = l[0]; j <= l[1]; j++) {
				p[j] = 1;
			}
		} else {
			return 0;
		}
	}
	return 1;
}

int parse_forbidden(char *line, int *p) {
	int l[2], i;
	char *ptr = line;
	
	i = l[0] = l[1] = 0;
	
	while (*ptr) {
		if (isdigit(*ptr)) {
			l[i] = l[i] * 10 + (*ptr - '0');
		} else if (*ptr == '-') {
			if (i == 0) {
				i = 1;
			} else {
				return 0;
			}
		} else if (*ptr == ',') {
			if (! set_forbidden(l, i, p)) {
				return 0;
			}
			i = l[0] = l[1] = 0;
		} else if (*ptr == '#') {
			break;
		} else {
			return 0;
		}
		ptr++;
	}
	
	if (ptr != line) {
		if (! set_forbidden(l, i, p)) {
			return 0;
		}
	}
	
	return 1;
}

ssize_t line_read(char **line, int *len, FILE *stream)
{
	int pos, chr;
	chr = pos = 0;
	
	if (*len == 0) {
		*len = 50;
		*line = (char *) malloc(*len);
	}
	
	while ((chr = fgetc(stream)) != '\n' && chr != '\r' && chr != EOF) {
		(*line)[pos++] = (char) chr;
		if (pos + 1 == *len) {
			*len *= 2;
			*line = (char *) realloc(*line, *len);
		}
	}
	
	(*line)[pos] = '\0';
	
	if (chr == '\r') {
		fgetc(stream);
	}

	if (pos == 0 && chr == EOF) {
		return EOF;
	} else {
		return pos;
	}
}

void line_trim(char *line) {
	char *r, *w, *e;
	int n = 0;
	r = w = line;
	e = line + strlen(line) - 1;
	
	while (e >= line && isblank(*e)) {
		e--;
	}
	
	*(++e) = 0;
	
	while (isblank(*r)) {
		r++;
	}
	
	n = r - line;
	
	if (n) {
		while (*r) {
			*w++ = *r++;
		}
		
		*(e - n) = 0;
	}
}

int line_prepare(char *line, char **val) {
	char *ptr = line;
	while (*ptr && *ptr != '=' && *ptr != '#') {
		ptr++;
	}
	
	if (*ptr == '=') {
		*ptr = 0;
		*val = ++ptr;
		
		while (*ptr && *ptr != '#') {
			ptr++;
		}
		*ptr = 0;
		
		line_trim(line);
		line_trim(*val);
		return 1;
	} else {
		return 0;
	}
}

void name_copy(char *src, char **dst) {
	char *r, *ptr, *e;

	r = ptr = src;
	e = src + strlen(src);
	

	while (*ptr) {
		if (*ptr == '/' || *ptr == '\\') {
			r = ++ptr;
		} else if (*ptr == '.') {
			e = ptr++;
		} else {
			ptr++;
		}
	}
	
	ptr = *dst = (char *) malloc(e - r + 1);
	
	while (r != e) {
		*ptr++ = *r++;
	}
	
	*ptr = 0;
	
}

void print_err(char *item, char *val) {
	fprintf(stderr, "Invalid value '%s' for item '%s'!\n", val, item);
}

FILE* fopen_read(const char *path) {
	FILE *file = fopen(path, "rt");
	int chr;

	if (file) {
		while (! isascii(chr = fgetc(file)) && chr != EOF) /* UTF-8 header */
			;
		ungetc(chr, file);
	}
	
	return file;
}

int compare(const void *p1, const void *p2) {
	return strcmp((**((struct subject **) p1)).name, (**((struct subject **) p2)).name);
}

int handle_args(int argc, char **argv, char **line, int *line_len) {
	FILE *file;
	int i, retval = 0;

	if (argc == 1) {
#ifndef DEBUG
		*line = DEF_PATH;
#else
		*line = "../testinput/settings.ini";
#endif
		file = fopen_read(*line);
		if (file) {	
			fclose(file);
		} else {
			fprintf(stderr, "Expected settings file '%s' could not be opened: %s!\n", *line, strerror(errno));
			fprintf(stderr, "Rename your settings file to '%s' and place it in same directory, as I am, or just pass me it's path as an argument!\n", DEF_PATH);
			retval = 1;
		}
	} else if (argc == 2) {
		*line = argv[1];
	} else if (argc > 2) {
		retval = 2;
		do {
			do {
				printf("Where should I create the file? (path or just name): ");
			} while (line_read(line, line_len, stdin) <= 0);
			
			file = fopen(*line, "wt");
			if (file) {
				break;
			} else {
				fprintf(stderr, "An error occurred while opening file '%s' for writing: %s!\n", *line, strerror(errno));
			}
		} while (1);
		
		printf("Writing list of subjects into file '%s'...", *line);
		
		for (i = 1; i < argc; i++) {
			if (fprintf(file, "%s\n", argv[i]) < 0) {
				retval = 3;
			}
		}
		
		fclose(file);
		
		if (retval == 2) {
			printf("success!\n");
		} else {
			fprintf(stderr, "unknown error..sorry!\n");
		}		
	}
	
	return retval;
}

void request_keypress(void) {
	printf("\nFeel free to report bug or just say thx: niklmate@fit.cvut.cz\n");
	printf("timetable_creator  Copyright (C) 2013  Matěj Nikl\n");
#ifdef WINDOWS
	printf("Press enter to quit...");
	getchar();
#endif
}

/*void subject_print(struct subject *s) {
	unsigned int i, j, k;
	struct parallel *l;
	printf("%s:\n", s->name);
	
	for (i = 0; i < s->n_lines; i++) {
		l = s->lines + i;
		printf("%s\t%s\t%d\t", l->number, l->teacher, l->usable);
		k = 0;
		for (j = 0; j < l->n_lessons; j++) {
			for ( ; k < l->lesson[j].day; k++) {
				putchar('\t');
			}
			printf("%d+%d", l->lesson[j].start, l->lesson[j].end);
		}
		putchar('\n');
	}
	putchar('\n');
}*/
