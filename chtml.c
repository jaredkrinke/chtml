#include <stddef.h>
#include "chtml.h"

/* Character classifier */
#define C_SPACE		1
#define C_BROPEN	2
#define C_BRCLOSE	3
#define C_EQUAL		4
#define C_APOS		5
#define C_QUOT		6
#define C_SLASH		7
#define C_NULL		8
#define C_OTHER		9

int classify_character(char c) {
	switch (c) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return C_SPACE;
			break;

		case '<': return C_BROPEN; break;
		case '>': return C_BRCLOSE; break;
		case '=': return C_EQUAL; break;
		case '\'': return C_APOS; break;
		case '"': return C_QUOT; break;
		case '/': return C_SLASH; break;
		case '\0': return C_NULL; break;

		default: return C_OTHER; break;
	}
}

/* Tokenizer */
#define T_NONE		0
#define T_SPACE		1
#define T_BROPEN	2
#define T_BRCLOSE	3
#define T_EQUAL		4
#define T_APOS		5
#define T_QUOT		6
#define T_TEXT		7

int get_next_token(const char** c, size_t* size) {
	int type = T_NONE;
	size_t l = 1;

	switch (classify_character(**c)) {
		case C_SPACE:
			/* Coalesce spaces */
			type = T_SPACE;
			while (classify_character(*((*c) + l)) == C_SPACE) {
				l++;
			}
			break;

		case C_BROPEN: type = T_BROPEN; break;
		case C_BRCLOSE: type = T_BRCLOSE; break;
		case C_EQUAL: type = T_EQUAL; break;
		case C_APOS: type = T_APOS; break;
		case C_QUOT: type = T_QUOT; break;

		case C_SLASH:
		case C_OTHER:
			/* Coalesce */
			type = T_TEXT;
			while (classify_character(*((*c) + l)) == C_OTHER) {
				l++;
			}
			break;

		case C_NULL:
			l = 0;
			break;
	}

	*size = l;
	*c = (*c) + l;
	return type;
}

void parse_value(const char** c, int end_type, const char** value, size_t* value_size) {
	int type = T_NONE;
	size_t size = 0;

	*value = *c;
	*value_size = 0;

	while ((type = get_next_token(c, &size)) != end_type) {
		if (type == T_NONE) {
			/* TODO: Error */
			return;
		}

		*value_size += size;
	}
}

int parse_attribute(const char** c, const char** value, size_t* value_size) {
	const char* prev = *c;
	size_t size = 0;
	int type = get_next_token(c, &size);

	switch (type) {
		case T_SPACE:
		case T_BRCLOSE:
		case T_NONE:
			/* No equal sign */
			*value = NULL;
			*value_size = 0;
			return type == T_BRCLOSE;

		case T_EQUAL:
			/* Three cases: no quotation, apostrophes, or quotation marks */
			*value = *c;
			type = get_next_token(c, &size);
			switch (type) {
				case T_TEXT:
					*value_size = size;
					break;

				case T_QUOT:
				case T_APOS:
					parse_value(c, type, value, value_size);
					break;
			}
			break;

		/* TODO: Errors */
	}

	return 0;
}

void parse_tag(const char** c, chtml_callbacks_t callbacks) {
	const char* tag = *c;
	size_t size = 0;

	if (get_next_token(c, &size) == T_TEXT) {
		if (classify_character(*tag) == C_SLASH) {
			/* Closing tag */
			return;
		}

		callbacks.start_tag(tag, size);

		for (;;) {
			const char* attribute = *c;
			int type = get_next_token(c, &size);
			
			switch (type) {
				case T_NONE:
				case T_BRCLOSE:
					return;

				case T_SPACE:
					attribute = *c;
					break;

				case T_TEXT:
					if (classify_character(*attribute) != C_SLASH) {
						const char* value = NULL;
						size_t value_size = 0;
						int closed = parse_attribute(c, &value, &value_size);

						callbacks.attribute(attribute, size, value, value_size);

						if (closed) {
							return;
						}
					}
					break;

				/* TODO: Errors! */
			}
		}
	}
}

void parse_html(const char* html, chtml_callbacks_t callbacks) {
	int type = T_NONE;
	size_t size = 0;

	while ((type = get_next_token(&html, &size)) != T_NONE) {
		switch (type) {
			case T_BROPEN:
				parse_tag(&html, callbacks);
				break;

			default: break;
		}
	}
}

/* TODO: Remove! */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>

void start(const char* tag, size_t size) {
	printf("Enter:\t");
	fwrite(tag, 1, size, stdout);
	puts("");
}

void attr(const char* attribute, size_t attribute_size, const char* value, size_t value_size) {
	printf("\tAttr:\t");
	fwrite(attribute, 1, attribute_size, stdout);
	printf("=");
	if (value) {
		fwrite(value, 1, value_size, stdout);
	}
	else {
		printf("(null)");
	}
	puts("");
}

int main(int argc, const char** argv) {
	FILE* f = fopen(argv[1], "rb");
	size_t length = 0;
	char* str = NULL;
	if (f) {
		if (fseek(f, 0, SEEK_END) != 0) printf("Error! %d", errno);
		length = ftell(f);
		str = malloc(length + 1);
		if (str) {
			str[length] = '\0';
			fseek(f, 0, SEEK_SET);
			fread(str, 1, length, f);

			{
				chtml_callbacks_t callbacks = { &start, &attr };
				parse_html(str, callbacks);

/*				char* c = str;
				char* prev = c;
				int type = 0;
				size_t length = 0;
				while (1) {
					prev = c;
					type = get_next_token(&c, &length);
					switch (type) {
						case T_NONE: printf("NONE"); break;
						case T_SPACE: printf("SPACE:\t"); break;
						case T_TEXT: printf("TEXT:\t"); break;
						case T_BROPEN: printf("BROPEN:\t"); break;
						case T_BRCLOSE: printf("BRCLOSE:\t"); break;
						case T_EQUAL: printf("EQUAL:\t"); break;
						case T_APOS: printf("APOS:\t"); break;
						case T_QUOT: printf("QUOT:\t"); break;
						default:
							printf("*** dunno");
							break;
					}
					if (type == T_SPACE) {
						printf("%lu", (size_t)(c - prev));
					}
					else {
						fwrite(prev, 1, (size_t)(c - prev), stdout);
						printf("\n");
					}
					if (!type) break;
				}*/
			}
		}
	}
	return 0;
}

