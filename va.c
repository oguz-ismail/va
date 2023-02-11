#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TABSTOP 8

struct line {
	char *ptr;
	size_t size;
	size_t len;
	size_t width;
	size_t lvl;
};

#define IS_BYTE1(c) ((c & 0xC0) != 0x80)

static size_t
next_tabstop(size_t i) {
	size_t n;

	n = i % TABSTOP;
	if (n == 0)
		return i;
	else
		return i + (TABSTOP - n);
}

static ssize_t
get_line(struct line *l) {
	return getline(&l->ptr, &l->size, stdin);
}

static void
put_line(struct line *l) {
	size_t i, j, k;

	for (i = 0; i < l->lvl; i++)
		putchar(l->ptr[i]);
	
	j = 0;
	for (; i < l->len; i++)
		if (l->ptr[i] == '\t') {
			k = next_tabstop(j + 1);
			for (; j < k; j++)
				putchar(' ');
		}
		else {
			putchar(l->ptr[i]);
			if (IS_BYTE1(l->ptr[i]))
				j++;
		}
}

int
main(int argc, char **argv) {
	struct line *buf;
	size_t buf_len, buf_size;
	size_t max_width, max_lvl, min_lvl;
	const char *line;
	ssize_t raw_len;
	size_t len, width, lvl;
	size_t i, j;

	if (argc > 1) {
		fprintf(stderr, "%s: extra operand: %s\n", argv[0], argv[1]);
		return 1;
	}

	buf = malloc(sizeof buf[0]);
	if (buf == NULL) {
		perror(NULL);
		return 1;
	}

	buf_size = 1;
	buf[0].ptr = NULL;
	buf[0].size = 0;

	do {
		buf_len = 0;
		max_width = 0;
		max_lvl = 0;
		min_lvl = SIZE_MAX;

		while ((raw_len = get_line(&buf[buf_len])) > 0) {
			line = buf[buf_len].ptr;
			len = raw_len;

			if (line[len - 1] == '\n')
				len--;

			for (i = 1; i <= len; i++)
				if (line[len - i] != '\\')
					break;

			if ((i - 1) % 2 == 0)
				break;

			len--;

			for (i = 1; i <= len; i++)
				if (line[len - i] != ' ' && line[len - i] != '\t')
					break;

			len = (len - i) + 1;

			for (i = 0; i < len; i++)
				if (line[i] != '\t')
					break;

			lvl = i;
			width = 0;

			for (; i < len; i++)
				if (line[i] == '\t')
					width = next_tabstop(width + 1);
				else if (IS_BYTE1(line[i]))
					width++;

			if (width > max_width)
				max_width = width;

			if (lvl > max_lvl)
				max_lvl = lvl;

			if (lvl < min_lvl)
				min_lvl = lvl;

			buf[buf_len].len = len;
			buf[buf_len].width = width;
			buf[buf_len].lvl = lvl;
			buf_len++;

			if (buf_len >= buf_size) {
				buf_size *= 2;
				buf = realloc(buf, buf_size * sizeof buf[0]);
				if (buf == NULL) {
					perror(NULL);
					return 1;
				}

				for (i = buf_len; i < buf_size; i++) {
					buf[i].ptr = NULL;
					buf[i].size = 0;
				}
			}
		}

		if (buf_len == 1) {
			fputs(buf[0].ptr, stdout);
		}
		else {
			if (max_lvl > min_lvl)
				max_width = next_tabstop(max_width);

			for (i = 0; i < buf_len; i++) {
				put_line(&buf[i]);

				for (j = buf[i].width; j < max_width; j++)
					putchar(' ');

				for (j = buf[i].lvl; j < max_lvl; j++)
					putchar('\t');

				fputs(" \\\n", stdout);
			}
		}

		if (raw_len > 0)
			fputs(line, stdout);
	}
	while (raw_len > 0);

	if (ferror(stdin)) {
		perror("stdin");
		return 1;
	}
}
