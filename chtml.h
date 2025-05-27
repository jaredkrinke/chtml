/* Note: There is no callback for text, comment, etc. nodes because I haven't needed them yet */

typedef void (*start_tag_callback_t)(const char* tag, size_t size);
typedef void (*attribute_callback_t)(const char* attribute, size_t attribute_size, const char* value, size_t value_size);
/*typedef void (*exit_tag_callback_t)(const char* tag, size_t size);*/

typedef struct {
	start_tag_callback_t start_tag;
	attribute_callback_t attribute;
	/*exit_tag_callback_t exit_tag;*/
} chtml_callbacks_t;

extern void parse_html(const char* html, chtml_callbacks_t callbacks);

