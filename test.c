#include <stdio.h>
#include <malloc.h>
#include "chtml.h"

void cb(chtml_event_t event, const char* str, size_t size, const chtml_context_t* ctx) {
	switch (event) {
		case CHTML_EVENT_TAG_ENTER:
			if (ctx->tag_size >= 1 && ctx->tag[0] != '/') {
				fwrite(ctx->tag, 1, ctx->tag_size, stdout);
				puts("");
			}
			break;

		case CHTML_EVENT_ATTRIBUTE:
			printf("\t");
			fwrite(ctx->attribute, 1, ctx->attribute_size, stdout);
			if (ctx->value) {
				printf(" = ");
				fwrite(ctx->value, 1, ctx->value_size, stdout);
			}
			puts("");
			break;
	}
}

int main(int argc, const char** argv) {
	FILE* f = fopen(argv[1], "rb");
	size_t length = 0;
	char* str = NULL;
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		str = malloc(length + 1);
		if (str) {
			str[length] = '\0';
			fseek(f, 0, SEEK_SET);
			fread(str, 1, length, f);

			{
				parse_html(str, &cb);
			}
		}
	}
	return 0;
}

