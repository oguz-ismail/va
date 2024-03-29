#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABSTOP 8

#define UTF8_BYTE_ONE(c) ((c & 0xC0) != 0x80)

struct line {
	char *ptr;
	size_t size;
	size_t len;
	size_t width;
	size_t lvl;
};

static size_t
next_tabstop(size_t pos) {
	size_t off;

	off = pos % TABSTOP;
	if (off == 0)
		return pos;
	else
		return pos + (TABSTOP - off);
}

static int getline_err;

static ssize_t
get_line(struct line *l) {
	ssize_t ret;

	errno = 0;
	ret = getline(&l->ptr, &l->size, stdin);
	getline_err = errno;
	return ret;
}

static void
put_line(struct line *l, size_t width, size_t lvl, int pad) {
	size_t i;
	size_t off, stop;

	for (i = 0; i < l->lvl; i++)
		putchar(l->ptr[i]);

	off = 0;
	for (; i < l->len; i++)
		if (l->ptr[i] == '\t') {
			stop = next_tabstop(off + 1);
			for (; off < stop; off++)
				putchar(' ');
		}
		else {
			putchar(l->ptr[i]);
			if (UTF8_BYTE_ONE(l->ptr[i]))
				off++;
		}

	for (i = l->width; i < width; i++)
		putchar(' ');

	for (i = l->lvl; i < lvl; i++)
		putchar('\t');

	if (pad)
		putchar(' ');

	fputs("\\\n", stdout);
}

static void
extend_buffer(struct line **buf_ptr, size_t *size) {
	size_t old_size, new_size;
	struct line *buf;
	size_t i;

	old_size = *size;
	if (old_size == 0)
		new_size = 1;
	else
		new_size = old_size * 2;

	buf = realloc(*buf_ptr, new_size * sizeof buf[0]);
	if (buf == NULL) {
		perror(NULL);
		exit(1);
	}

	for (i = old_size; i < new_size; i++) {
		buf[i].ptr = NULL;
		buf[i].size = 0;
	}

	*buf_ptr = buf;
	*size = new_size;
}

int
main(int argc, char **argv) {
	struct line *buf;
	size_t buf_len, buf_size;
	size_t max_width, max_lvl, min_lvl;
	int pad;
	const char *line;
	ssize_t raw_len;
	size_t len, width, lvl;
	size_t i;

	if (argc > 1) {
		fprintf(stderr, "%s: extra operand: %s\n", argv[0], argv[1]);
		return 1;
	}

	buf = NULL;
	buf_size = 0;
	extend_buffer(&buf, &buf_size);

	do {
		buf_len = 0;
		max_width = 0;
		max_lvl = 0;
		min_lvl = SIZE_MAX;
		pad = 0;

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
				if (memchr(" \t", line[len - i], 2) == NULL)
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
				else if (UTF8_BYTE_ONE(line[i]))
					width++;


			if (width > max_width) {
				max_width = width;

				if (lvl >= max_lvl)
					pad = (width % TABSTOP == 0);
			}

			if (lvl > max_lvl)
				max_lvl = lvl;

			if (lvl < min_lvl)
				min_lvl = lvl;

			buf[buf_len].len = len;
			buf[buf_len].width = width;
			buf[buf_len].lvl = lvl;

			buf_len++;
			if (buf_len >= buf_size)
				extend_buffer(&buf, &buf_size);
		}

		if (buf_len == 1) {
			fputs(buf[0].ptr, stdout);
		}
		else if (buf_len > 1) {
			if (max_lvl > min_lvl)
				max_width = next_tabstop(max_width);
			else
				pad = 1;

			for (i = 0; i < buf_len; i++)
				put_line(&buf[i], max_width, max_lvl, pad);
		}

		if (raw_len > 0)
			fputs(line, stdout);
	}
	while (raw_len > 0);

	if (ferror(stdin)) {
		perror("stdin");
		return 1;
	}

	if (getline_err) {
		errno = getline_err;
		perror(NULL);
		return 1;
	}
}
